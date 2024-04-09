#include "common.h"
#include "torchBox/pytorch.h"
#include "torchBox/testMakeMore.h"

namespace bt = boost::posix_time;
namespace common = mikado::common;
namespace po = boost::program_options;
using namespace common;
using namespace std;

namespace mikado::torchBox {

   ///////////////////////////////////////////////////////////////////////////
   //
   void TestMakeMore::addOptions(ConfigurePtr cfg) {
      cfg->addOptions()
         (kMakeMoreNamesFile.c_str(), po::value<string>(), "A text file of names, one per line")
         ;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void TestMakeMore::readNamesFile() {
      // Build a map of char-to-index (stoi) and back again (itos)
      string name;
      size_t count = 0;

      totalLength_ = 0;
      auto filename = lexicalPath(getConfig()->get<string>(kMakeMoreNamesFile));
      if (ifstream ifs(filename); ifs.is_open()) {
         str_info() << "Reading names from " << filename << endl;
         while (ifs) {
            getline(ifs, name);
            if (!name.empty()) {
               ++count;
               totalLength_ += name.size() + 1; // Include the '.' separator

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
      }
      else {
         str_error() << "Failed to open names file: '" << filename << "'" << endl;
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void TestMakeMore::makeTensors() {
      try
      {
         auto contextSize = getConfig()->get<long>(kMakeMoreContextSize, contextSize_);
         int trainSize = 0.8 * names_.size();
         int developSize = 0.1 * names_.size();
         int testSize = names_.size() - (trainSize + developSize);

         torch::TensorOptions options{ dtype(torch::kInt64) };

         // We will build the tensor from two arrays of "vocab_t" integers (embedding indices)
         // and then convert them to tensors
         std::vector<vocab_t> xx, yy;
         xx.reserve(totalLength_ * contextSize_);
         yy.reserve(totalLength_);

         buildDataset(names_.begin(), names_.begin() + trainSize, xx, yy);
         tensors_[(int)TensorType::kTrainX] = torch::from_blob(xx.data(), { (vocab_t)totalLength_, contextSize }, options);
         tensors_[(int)TensorType::kTrainY] = torch::from_blob(yy.data(), { (vocab_t)totalLength_ }, options);
         //str_info() << "X = " << getShape(tensors_[(int)TensorType::kTrainX]) << endl;
         //str_info() << "Y = " << getShape(tensors_[(int)TensorType::kTrainY]) << endl;

         buildDataset(names_.begin() + trainSize, names_.begin() + trainSize + developSize
            , xx, yy);
         tensors_[(int)TensorType::kDevelopX] = torch::from_blob(xx.data(), { (vocab_t)totalLength_, contextSize }, options);
         tensors_[(int)TensorType::kDevelopY] = torch::from_blob(yy.data(), { (vocab_t)totalLength_ }, options);
         
         buildDataset(names_.begin() + trainSize + developSize, names_.end(), xx, yy);
         tensors_[(int)TensorType::kTestX] = torch::from_blob(xx.data(), { (vocab_t)totalLength_, contextSize }, options);
         tensors_[(int)TensorType::kTestY] = torch::from_blob(yy.data(), { (vocab_t)totalLength_ }, options);
         
         // Transfer them all to the CUDA device
         if (auto deviceType = getC10Device(); deviceType != c10::DeviceType::CPU) {
            for (int ii = (int)TensorType::kTrainX; ii < (int)TensorType::TensorTypeCount; ++ii)
            {
               //!!! Do we need to assign the tensor to the return value of to(), when we move it?
               tensors_[ii].to(deviceType);
            }
         }
      }
      catch (const std::exception &e)
      {
         log_exception(e);
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void TestMakeMore::buildDataset(WordIter fromWord, WordIter toWord
         , vector<vocab_t> &xx, vector<vocab_t> &yy) {

      // The dataset is all of the possible sequences of letters that are in the names file, where each sequence
      // is equal to the context length, padded with '.', which represents "the end of the sequence"

      try
      {
         while (fromWord < toWord) {
            string letterSequence = *fromWord++ + ".";    // Using a '.' to pad out the context when there are no more letters

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
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void TestMakeMore::run() {
      auto startedAt = bt::second_clock::local_time();
      str_info() << "Running test 'MakeMore'" << endl;

      readNamesFile();
      makeTensors();

      auto elapsed = bt::second_clock::local_time() - startedAt;

      str_info() << "MakeMore took " << elapsed << " seconds." << endl;
   }

} // namespace mikado::torchBox
