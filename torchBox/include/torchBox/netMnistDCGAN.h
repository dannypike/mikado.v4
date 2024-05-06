#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_MNIST_DCGAN_H)
#define TBOX_MNIST_DCGAN_H

#include "netBase.h"

namespace mikado::torchBox {

   class NetMnistDCGAN : public NetBase {
   public:
      NetMnistDCGAN();

      static void addOptions(common::ConfigurePtr cfg);
      virtual MikadoErrorCode configure(common::ConfigurePtr cfg) override;
      virtual MikadoErrorCode train() override;
      virtual MikadoErrorCode verify() override;

   protected:

   private:
      using ds = torch::data::datasets::MNIST;

      torch::Device device_;
      std::filesystem::path datasetsFolder_;
      std::filesystem::path outputFolder_; 
      
      int numberOfEpochs_ = 30; // The number of epochs to train.
   };

} // namespace mikado::torchBox

#endif // TBOX_MNIST_DCGAN_H
