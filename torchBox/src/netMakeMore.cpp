#include "common.h"
#include "windowsApi.h"
#include "torchBox/pytorch.h"
#include "torchBox/netMakeMore.h"

namespace bt = boost::posix_time;
namespace common = mikado::common;
namespace po = boost::program_options;
namespace windowsApi = mikado::windowsApi;
using namespace std;
using namespace torch;
using namespace torch::indexing;

namespace mikado::torchBox {

   ///////////////////////////////////////////////////////////////////////////
   //
   NetMakeMore::NetMakeMore()
      : NetBase(common::kMakeMore) {
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void NetMakeMore::addOptions(common::ConfigurePtr cfg) {
      cfg->addOptions()
         (common::kMakeMoreNamesFile.c_str(), po::value<string>()
            , "A text file of names, one per line")
         (common::kMakeMoreMaxNames.c_str(), po::value<size_t>()->default_value(numeric_limits<size_t>::max())
            , "Maximum number of names to read from the file")
         (common::kMakeMoreTrainingSteps.c_str(), po::value<size_t>()->default_value(1000)
            , "Maximum number of steps to use while training")
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

      auto filename = common::lexicalPath(getConfig()->get<string>(common::kMakeMoreNamesFile));
      ifstream ifs(filename); 
      if (!ifs.is_open()) {
         str_error() << "Failed to open names file: '" << filename << "'" << endl;
         return MikadoErrorCode::MKO_ERROR_PATH_NOT_FOUND;
      }

      size_t maxNames = getConfig()->get<size_t>(common::kMakeMoreMaxNames);
      str_info() << "reading up to " << maxNames << " names from " << filename << endl;
      while (ifs && !ifs.eof()) {
         getline(ifs, name);
         name = common::trim(name);
         if (name.empty()) {
            continue;
         }

         if (maxNames < ++count) {
            str_info() << "maximum number of names reached" << endl;
            break;
         }
         trainingDataCount_ += name.size() + 1; // +1 for the kSeparator

         // Create the tokens for each letter in the name
         for (auto cc : name + kSeparator) {
            if (stoi_.insert(make_pair(cc,itos_.size())).second) {
               itos_.push_back(cc);
            }
         }

         // Save the word to be a part of the training dataset
         names_.emplace_back(name);
      }
      assert(trainingDataCount_ < numeric_limits<vocab_t>::max()); // Total length of names is too long?
      str_info() << count << " names read, with " << vocabSize() << " unique characters, total length = "
         << trainingDataCount_ << endl;

      // randomize the list of names
      random_device rnd;
      std::mt19937 rng(rnd());
      shuffle(names_.begin(), names_.end(), rng);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   // Build the training data from the names file and split it into training,
   // development and test sets
   //
   MikadoErrorCode NetMakeMore::createTrainingTensors() {
      try
      {
         typedef function<void(Subset, Subset, WordIter, WordIter)> BuildDatasetType;
         BuildDatasetType buildDataset = [&](Subset x, Subset y, WordIter itBeginWord, WordIter itEndWord) {
            
            static auto options = TensorOptions().dtype(torch::kInt64).device(c10::kCPU);
            
            str_info() << "building dataset for " << x << "/Y, with "
               << distance(itBeginWord, itEndWord) << " words" << endl;

            try {
               int64_t totalColumns = 0;
               vector<vocab_t> dataX, dataY;
               while (itBeginWord < itEndWord) {
                  // Start with a fresh context for each name
                  vector<vocab_t> context(contextSize_, stoi_[kSeparator]);

                  auto w = *itBeginWord++;
                  for (auto ch : w + kSeparator) {
                     auto ix = stoi_[ch];
                     dataX.insert(dataX.end(), context.begin(), context.end());  // This context ...
                     dataY.push_back(ix);  // ... predicts this letter
                     
                     ++totalColumns;   // Total number of colums in the training dataset
                  
                     context.erase(context.begin());
                     context.push_back(ix);
                  }
               }
               tensors_[(int)x] = torch::tensor(dataX, options).view({ totalColumns, contextSize_ });
               tensors_[(int)y] = torch::tensor(dataY, options);
            }
            catch (const std::exception &e) {
               log_exception(e);
            }
         };

         // Split the names into training, development and test sets
         assert(2 < names_.size());    // Need at least one in each group
         auto splitCount = max(names_.size() / 10, 1ull);
         auto itSplit2 = names_.end() - splitCount;
         auto itSplit1 = itSplit2 - splitCount;

         buildDataset(Subset::TrainX, Subset::TrainY, names_.begin(), itSplit1);
         buildDataset(Subset::DevelopX, Subset::DevelopY, itSplit1, itSplit2);
         buildDataset(Subset::TestX, Subset::TestY, itSplit2, names_.end());

         str_info() << "XYTrain: X.shape = " << tensors_[(int)Subset::TrainX].sizes()
            << ", Y.shape = " << tensors_[(int)Subset::TrainY].sizes() << endl;
         str_info() << "XYDevelop: X.shape = " << tensors_[(int)Subset::DevelopX].sizes()
            << ", Y.shape = " << tensors_[(int)Subset::DevelopY].sizes() << endl;
         str_info() << "XYTest: X.shape = " << tensors_[(int)Subset::TestX].sizes()
            << ", Y.shape = " << tensors_[(int)Subset::TestY].sizes() << endl;
         str_info() << "total number of names = " << names_.size() << endl;
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
         auto options = TensorOptions().dtype(torch::kFloat32);

         //str_info() << "build.X.TrainX = " << toStringFlat<int64_t>(tensors_[(int)Subset::TrainX], 32) << " -> B" << endl;
         
         // Number of dimensions in the embedding vector
         nEmbD_ = getConfig()->get<long>(common::kMakeMoreEmbeddingDim, nEmbD_);

         // Number of neurons in the hidden layer
         nHidden_ = getConfig()->get<long>(common::kMakeMoreHiddenLayer, nHidden_);

         // Set the seed of the random number generator (RNG) to a fixed value (42), for reproducibility
         torch::Generator g = torch::make_generator<torch::CPUGeneratorImpl>(42);

         // Create the embedding layer, e.g. 10 dimensions for each letter
         torch::manual_seed(2147483647);  // not sure why MakeMore doesn't use the RNG above, but we're copying Karpathy's code so we do it this way
         C_ = torch::randn({ vocabSize(), nEmbD_ }, options);
         str_info() << "C_ initialized to: " << endl
            << C_ << endl;

         // Some "magic numbers" to fix problems with using only a simple random initialisation.
         auto scaleW2 = 0.01;
         auto scaleB2 = 0;

         // Create the hidden layer, which has two dimensions :
         // 1. Number of inputs = (number of characters in the embedding, i.e. contextSize_) * (nEmbD_ - the length of each embedding vector)
         // 2. The number of neurons in the hidden layer (user-configurable, as above)
         //
         // Use Kaiming He's initialization to set the weights to suitable values to prevent the tanh() activation saturating
         W1_ = torch::zeros({ nEmbD_ * contextSize_, nHidden_ }, options);
         torch::nn::init::kaiming_normal_(W1_, 0, torch::kFanIn, torch::kTanh);
         
         // Note there is no bias when using batch normalization (BN), because it has no effect. IOW, adding b1,
         // then substracting the "mean of b1" has zero net effect. In BN, the bias effect is provided by a
         // value that is trained separately (bnBias_) ...
         bnBias_ = torch::zeros({ 1, nHidden_ }, options);

         // BN gain that will be used to normalise the hidden layer weights before applying the tanh()
         // activation function
         bnGain_ = torch::ones({ 1, nHidden_ }, options);

         // BN mean and std are estimated during training, so that they are available during inference(when there is only
         // a single input, not a batch of them), e.g.when calling the split_loss() method. They're
         // not used as trainable parameters, so they're not in the parameters_ list
         bnMeanRunning_ = torch::zeros({ 1, nHidden_ }, options);
         bnStdRunning_ = torch::ones({ 1, nHidden_ }, options);

         // Final layer
         // The output has 'vocab_size' values, each of which will show a single probability
         // Need to reduce the size of the weights to make sure that they are first values are near zero,
         // and we don't have to spend iterations on "squashing". Dangerous to set it to zero (no entropy)
         
         // Set the seed to a fixed value (42), for reproducibility
         W2_ = torch::randn({ nHidden_, vocabSize() }, g, options);
         b2_ = torch::randn({ vocabSize() }, g, options);

#if false   // Don't do this yet. we haven't built the embedding matrix and it's too slow to access
         // the CUDA device other than as a batch transfer)

      // Now we can move them to the CUDA device
         if (auto device = getC10Device(); device != c10::DeviceType::CPU) {
            W2_.to(device);
            b2_.to(device);
         }
#endif

         // All of the parameters are trainable
         parameters_.insert(make_pair(common::kTensorC, &C_));
         parameters_.insert(make_pair(common::kTensorW1, &W1_));
         parameters_.insert(make_pair(common::kTensorW2, &W2_));
         parameters_.insert(make_pair(common::kTensorB2, &b2_));
         parameters_.insert(make_pair(common::kTensorBNGain, &bnGain_));
         parameters_.insert(make_pair(common::kTensorBNBias, &bnBias_));

         size_t index = 0, count = 0;
         for (auto pairParameter : parameters_) {
            auto parameter = pairParameter.second;
            parameter->set_requires_grad(true);
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
   MikadoErrorCode NetMakeMore::toDevice() {
      try
      {
         // Transfer everything to the CUDA device, if so configrued
         auto deviceType = getC10Device(); 
         str_info() << "performing calculations on device: " << deviceType << endl;
         if (c10::DeviceType::CPU == deviceType) {
            return MikadoErrorCode::MKO_STATUS_NOOP;
         }

         for (int ii = 0; ii < (int)Subset::SubsetCount; ++ii) {
            tensors_[ii] = tensors_[ii].to(deviceType);
            str_info() << "tensor[" << (Subset)ii << "] is at " << tensors_[ii].device() << endl;
         }
         C_ = C_.to(deviceType);
         bnGain_ = bnGain_.to(deviceType);
         bnBias_ = bnBias_.to(deviceType);
         W1_ = W1_.to(deviceType);
         W2_ = W2_.to(deviceType);
         b2_ = b2_.to(deviceType);
         bnMeanRunning_ = bnMeanRunning_.to(deviceType);
         bnStdRunning_ = bnStdRunning_.to(deviceType);

         for (auto pairParameter : parameters_) {
            auto parameter = pairParameter.second;
            str_info() << "parameter " << pairParameter.first << " is at " << parameter->device() << endl;
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
   MikadoErrorCode NetMakeMore::reportLoss(Subset subsetX, Subset subsetY) {

      typedef function<void(Subset, Subset, Tensor, Tensor)> SplitLossType;
      SplitLossType splitLoss
         = [&](Subset subsetX, Subset subsetY, Tensor bnMeanRunning, Tensor bnStdRunning) {

         Tensor x = tensors_[(int)subsetX];
         Tensor y = tensors_[(int)subsetY];
         auto emb = C_.index({x}); // dims: [N, contextSize_, nEmbD_]

         auto embCat = emb.view({ emb.sizes()[0], -1});
         auto hiddenPreAct = embCat.mm(W1_);

         hiddenPreAct = bnGain_ * (hiddenPreAct - bnMeanRunning_) / (bnStdRunning_ + epsilon_) + bnBias_;
         auto hiddenAct = hiddenPreAct.tanh();  // (N, nHidden)
         auto logits = hiddenAct.mm(W2_) + b2_;   // (N, vocabSize())
         auto loss = torch::nn::functional::cross_entropy(logits, y);

         torch::print(str_info(), C_, 160);
         str_info() << fixed << setprecision(4)
            << "reportLoss(" << subsetX << ") returned " << loss.item<float>() 
            << ", with vocabSize = " << vocabSize() << endl;
         str_info() << "    C.shape = " << getShape(C_) << endl;
         str_info() << "    x.shape = " << getShape(x) << endl;
         str_info() << "  emb.shape = " << getShape(emb) << endl;
         torch::print(str_info() << "  predicted output (logits):" << endl
            , logits, 240) << endl;
         torch::print(str_info() << "  predicted output (softmax):" << endl
            , torch::softmax(logits, 1), 240) << endl;
         str_info() << "  expected output (classes):" << endl
            << y << endl;
         };

      torch::NoGradGuard no_grad;
      try
      {
         splitLoss(subsetX, subsetY, bnMeanRunning_, bnStdRunning_);
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
         || MKO_IS_ERROR(rc = createTrainingTensors())
         || MKO_IS_ERROR(rc = buildLayers())
         || MKO_IS_ERROR(rc = toDevice())
         || MKO_IS_ERROR(rc = reportLoss(Subset::TrainX, Subset::TrainY))
         || MKO_IS_ERROR(rc = reportLoss(Subset::DevelopX, Subset::DevelopY))
         ) {
         return rc;
      }

      auto elapsed = bt::second_clock::local_time() - startedAt;
      trainingSteps_ = cfg->get<size_t>(common::kMakeMoreTrainingSteps);

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
