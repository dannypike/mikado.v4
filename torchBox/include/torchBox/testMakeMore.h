#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_MAKEMORE_H)
#define TBOX_MAKEMORE_H

#include "testBase.h"

namespace mikado::torchBox {

   class TestMakeMore : public TestBase {
   public:
      TestMakeMore() = default;

      static void addOptions(common::ConfigurePtr cfg);
      virtual void run() override;

      size_t vocabSize() const { return itos_.size(); }

   protected:
      typedef std::vector<std::string>::const_iterator WordIter;
      typedef int64_t vocab_t;   // Standard integer type for tensors

      void readNamesFile();
      void makeTensors();
      void buildDataset(WordIter fromWord, WordIter toWord
         , std::vector<vocab_t> &xx, std::vector<vocab_t> &yy);

   private:
      std::map<char, vocab_t> stoi_;
      std::vector<char> itos_;
      size_t contextSize_ = 3;
      size_t totalLength_ = 0;
      std::vector<std::string> names_;

      enum class TensorType : int {
         kTrainX = 0,
         kTrainY,
         kDevelopX,
         kDevelopY,
         kTestX,
         kTestY,
         TensorTypeCount
      };

      torch::Tensor tensors_[(int)TensorType::TensorTypeCount];
   };

} // namespace mikado::torchBox

#endif // TBOX_MAKEMORE_H
