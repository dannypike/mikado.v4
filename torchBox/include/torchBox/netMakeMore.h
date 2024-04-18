#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_MAKEMORE_H)
#define TBOX_MAKEMORE_H

#include "torchBox/pytorch.h"
#include "torchBox/subsetEnum.h"
#include "netBase.h"

namespace mikado::torchBox {

   typedef common::MikadoErrorCode MikadoErrorCode;

   class NetMakeMore : public NetBase {
   public:
      NetMakeMore();

      static void addOptions(common::ConfigurePtr cfg);
      virtual MikadoErrorCode configure(common::ConfigurePtr cfg) override;
      virtual MikadoErrorCode train() override;
      virtual MikadoErrorCode verify() override;

      int64_t vocabSize() const { return (int64_t)itos_.size(); }

   protected:
      typedef std::vector<std::string>::const_iterator WordIter;
      typedef int64_t vocab_t;   // Standard integer type for tensors
      char const kSeparator = '.';

      MikadoErrorCode readNamesFile();
      MikadoErrorCode createTrainingTensors();
      MikadoErrorCode buildLayers();
      MikadoErrorCode toDevice();
      MikadoErrorCode reportLoss(Subset subsetX, Subset subsetY);

   private:
      std::map<char, vocab_t> stoi_;
      std::vector<char> itos_;
      vocab_t contextSize_ = 3;
      std::vector<std::string> names_;
      int64_t trainingDataCount_ = 0;

      long nEmbD_ = 10;   // Number of dimensions for each letter in the neural space
      long nHidden_ = 200;
      torch::Tensor C_;       // trainable parameter
      torch::Tensor bnGain_;  // trainable parameter
      torch::Tensor bnBias_;  // trainable parameter
      torch::Tensor W1_;      // trainable parameter
      torch::Tensor W2_;      // trainable parameter
      torch::Tensor b2_;      // trainable parameter
      torch::Tensor bnMeanRunning_;
      torch::Tensor bnStdRunning_;
      double const epsilon_ = 1e-5;  // prevent DIV / 0 when dividing hpreact by the std
      size_t parameterCount_ = 0;

      // Holds a collection of references to the above parameters of the model
      // for updating them during training and also ease of moving them to CUDA
      std::map<std::string, torch::Tensor *> parameters_;

      torch::Tensor tensors_[(int)Subset::SubsetCount];
   };

} // namespace mikado::torchBox

#endif // TBOX_MAKEMORE_H
