#include "common.h"
#include "torchBox/pytorch.h"
#include "torchBox/netMakeMore.h"

namespace bt = boost::posix_time;
namespace common = mikado::common;
namespace po = boost::program_options;
using namespace common;
using namespace std;

namespace mikado::torchBox {

   ///////////////////////////////////////////////////////////////////////////
   //
   NetMakeMore::NetMakeMore()
      : NetBase(common::kMakeMore) {
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void NetMakeMore::addOptions(ConfigurePtr cfg) {
      cfg->addOptions()
         (kMakeMoreNamesFile.c_str(), po::value<string>(), "A text file of names, one per line")
         ;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode NetMakeMore::readNamesFile() {
      // Build a map of char-to-index (stoi) and back again (itos)
      string name;
      size_t count = 0;

      // The separator has the "special" index 0 to make it easy to spot
      stoi_.insert(make_pair(kSeparator, 0));
      itos_.push_back(kSeparator);

      totalLength_ = 0;
      auto filename = lexicalPath(getConfig()->get<string>(kMakeMoreNamesFile));
      ifstream ifs(filename); 
      if (!ifs.is_open()) {
         str_error() << "Failed to open names file: '" << filename << "'" << endl;
         return MikadoErrorCode::MKO_ERROR_PATH_NOT_FOUND;
      }

      str_info() << "reading names from " << filename << endl;
      while (ifs) {
         getline(ifs, name);
         if (!name.empty()) {
            ++count;
            totalLength_ += name.size() + 1; // Include the '.' separator in the length calculation

            // Create the tokens for each letter in the name
            for (auto cc : name) {
               if (stoi_.insert(make_pair(cc,itos_.size())).second) {
                  itos_.push_back(cc);
               }
            }

            // Save the words to build the dataset
            names_.emplace_back(name);
         }
      }
      assert(totalLength_ < numeric_limits<vocab_t>::max()); // Total length of names is too long?
      str_info() << count << " names read, with " << vocabSize() << " unique characters" << endl;

      // randomize the list of names
      random_device rnd;
      mt19937 rng(rnd());
      shuffle(names_.begin(), names_.end(), rng);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode NetMakeMore::makeTensors() {
      try
      {
         auto contextSize = getConfig()->get<long>(kMakeMoreContextSize, contextSize_);
         int trainSize = 0.8 * names_.size();
         int developSize = 0.1 * names_.size();
         //int datasetSize = names_.size() - (trainSize + developSize);

         torch::TensorOptions options{ dtype(torch::kInt64) };

         // We will build the tensor from two arrays of "vocab_t" integers (embedding indices)
         // and then convert them to tensors
         std::vector<vocab_t> xx, yy;
         xx.reserve(totalLength_ * contextSize_);
         yy.reserve(totalLength_);

         buildDataset(names_.begin(), names_.begin() + trainSize, xx, yy);
         tensors_[(int)Subset::kTrainX] = torch::from_blob(xx.data(), { (vocab_t)totalLength_, contextSize }, options);
         tensors_[(int)Subset::kTrainY] = torch::from_blob(yy.data(), { (vocab_t)totalLength_ }, options);
         //str_info() << "X = " << getShape(tensors_[(int)Subset::kTrainX]) << endl;
         //str_info() << "Y = " << getShape(tensors_[(int)Subset::kTrainY]) << endl;

         buildDataset(names_.begin() + trainSize, names_.begin() + trainSize + developSize
            , xx, yy);
         tensors_[(int)Subset::kDevelopX] = torch::from_blob(xx.data(), { (vocab_t)totalLength_, contextSize }, options);
         tensors_[(int)Subset::kDevelopY] = torch::from_blob(yy.data(), { (vocab_t)totalLength_ }, options);
         
         buildDataset(names_.begin() + trainSize + developSize, names_.end(), xx, yy);
         tensors_[(int)Subset::kTestX] = torch::from_blob(xx.data(), { (vocab_t)totalLength_, contextSize }, options);
         tensors_[(int)Subset::kTestY] = torch::from_blob(yy.data(), { (vocab_t)totalLength_ }, options);
         
         // Transfer them all to the CUDA device
         if (auto deviceType = getC10Device(); deviceType != c10::DeviceType::CPU) {
            for (int ii = (int)Subset_traits::to_underlying(Subset::kTrainX); ii < (int)Subset::SubsetCount; ++ii) {
               tensors_[ii].to(deviceType);
            }
         }
      }
      catch (const std::exception &e)
      {
         log_exception(e);
         return MikadoErrorCode::MKO_ERROR_EXCEPTION;
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode NetMakeMore::buildDataset(WordIter fromWord, WordIter toWord
         , vector<vocab_t> &xx, vector<vocab_t> &yy) {

      // The dataset is all of the possible sequences of letters that are in the names file, where each sequence
      // is equal to the context length, padded with '.', which represents "the end of the sequence"

      try
      {
         while (fromWord < toWord) {
            string letterSequence = *fromWord++ + kSeparator;    // Using a '.' to pad out the context when there are no more letters

            vector<vocab_t> context;
            vector<vocab_t> embedding = { 0 };

            for (auto ch : letterSequence) {
               // This won't throw, unless will throw unless the readNamesFile() failed to
               // construct the stoi_ map correctly
               embedding[0] = stoi_[ch];

               xx.insert(xx.end(), context.begin(), context.end());
               yy.push_back(embedding[0]);

               context.push_back(embedding[0]);
               while (contextSize_ < context.size()) {
                  context.erase(context.begin());
               }
            }
         }
      }
      catch (const std::exception &e)
      {
         log_exception(e);
         return MikadoErrorCode::MKO_ERROR_EXCEPTION;
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode NetMakeMore::buildLayers() {
      try
      {
         // Number of dimensions in the embedding vector
         embeddingDim_ = getConfig()->get<long>(kMakeMoreEmbeddingDim, embeddingDim_);

         // Number of neurons in the hidden layer
         hiddenLayer_ = getConfig()->get<long>(kMakeMoreHiddenLayer, hiddenLayer_);

         // Create the embedding layer, e.g. 10 dimensions for each letter
         auto options = torch::TensorOptions().dtype(torch::kFloat32).device(getC10Device());
         C_ = torch::randn({ vocabSize(), embeddingDim_ }, options);
         parameters_.push_back(&C_);

         // Some "magic numbers" to fix problems with using only a simple random initialisation.
         auto scaleW2 = 0.01;
         auto scaleB2 = 0;

         // Create the hidden layer, which has two dimensions :
         // 1. Number of inputs = (number of characters in the embedding(block_size)) x(dim - size of each embedding)
         // 2. The number of neurons in the hidden layer(user - configurable, as above)
         //
         // Use Kaiming He's initialization to set the weights to suitable values to prevent the tanh() activation saturating
         W1_ = torch::zeros({ embeddingDim_ * contextSize_, hiddenLayer_ }, options);
         parameters_.push_back(&W1_);
         torch::nn::init::kaiming_normal_(W1_, 0, torch::kFanIn, torch::kTanh);

         // Final layer
         // The output has 'vocab_size' values, each of which will show a single probability
         // Need to reduce the size of the weights to make sure that they are first values are near zero,
         // and we don't have to spend iterations on "squashing". Dangerous to set it to zero (no entropy)
         
         // Set the seed to a fixed value (42), for reproducibility
         auto optionsRng = torch::TensorOptions().dtype(torch::kFloat32);  // Need to use the CPU in CUDA11.8
         torch::Generator rng = torch::make_generator<torch::CPUGeneratorImpl>(42);
         W2_ = torch::randn({ hiddenLayer_, vocabSize() }, rng, optionsRng);
         b2_ = torch::randn({ vocabSize() }, rng, optionsRng);
         parameters_.push_back(&W2_);
         parameters_.push_back(&b2_);

         // Now we can move them to the CUDA device
         if (auto device = getC10Device(); device != c10::DeviceType::CPU) {
            W2_.to(device);
            b2_.to(device);
         }

         // Batch normalisation gain and bias that will be used to normalise the hidden layer weights
         // before applying the tanh() activation function
         bnGain_ = torch::ones({ 1, hiddenLayer_ }, options);
         parameters_.push_back(&bnGain_);
         bnBias_ = torch::zeros({ 1, hiddenLayer_ }, options);
         parameters_.push_back(&bnBias_);

         // BN mean and std are estimated during training, so that they are available during inference(when there is only
         // a single input, not a batch of them), e.g.when calling the split_loss() method. They're
         // not used as trainable parameters, so they're not in the parameters_ list
         bnMeanRunning_ = torch::zeros({ 1, hiddenLayer_ }, options);
         bnStdRunning_ = torch::ones({ 1, hiddenLayer_ }, options);

         // All of the parameters are trainable
         size_t index = 0, count = 0;
         for (auto parameter : parameters_) {
            parameter->set_requires_grad(true);
            str_debug() << "# " << index++ << " : " << getShape(*parameter) << " = "
               << parameter->numel() << endl;
            count += parameter->numel();
         }
         parameterCount_ = count;
         str_info() << "total number of parameters: " << parameterCount_ << endl;
      }
      catch (const std::exception &e)
      {
         log_exception(e);
         return MikadoErrorCode::MKO_ERROR_EXCEPTION;
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode NetMakeMore::reportLoss() {
      try
      {
         
      }
      catch (const std::exception &e)
      {
         log_exception(e);
         return MikadoErrorCode::MKO_ERROR_EXCEPTION;
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode NetMakeMore::configure(common::ConfigurePtr cfg) {
      auto startedAt = bt::second_clock::local_time();
      str_info() << "configuring '" << common::kMakeMore << "'" << endl;

      auto rc = MikadoErrorCode::MKO_ERROR_NONE;
      if (MKO_IS_ERROR(rc = readNamesFile())
         || MKO_IS_ERROR(rc = makeTensors())
         || MKO_IS_ERROR(rc = buildLayers())
         || MKO_IS_ERROR(rc = reportLoss())
         ) {
         return rc;
      }

      auto elapsed = bt::second_clock::local_time() - startedAt;
      str_info() << "configuring '" << common::kMakeMore << "' took "
         << elapsed << " seconds." << endl;
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode NetMakeMore::train() {
      auto startedAt = bt::second_clock::local_time();
      str_info() << "training '" << common::kMakeMore << "'" << endl;

      auto elapsed = bt::second_clock::local_time() - startedAt;
      str_info() << "training '" << common::kMakeMore << "' took "
         << elapsed << " seconds." << endl;
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode NetMakeMore::verify() {
      auto startedAt = bt::second_clock::local_time();
      str_info() << "verifying '" << common::kMakeMore << "'" << endl;

      auto elapsed = bt::second_clock::local_time() - startedAt;
      str_info() << "verifying '" << common::kMakeMore << "' took "
         << elapsed << " seconds." << endl;
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

} // namespace mikado::torchBox
