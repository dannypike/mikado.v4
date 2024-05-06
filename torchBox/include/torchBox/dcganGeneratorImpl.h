#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_DCGAN_GENERATOR_IMPL_H)
#define TBOX_DCGAN_GENERATOR_IMPL_H

namespace mikado::torchBox {

   const int kNoiseSize = 100;
   const int kBatchSize = 64;
   const int kNumEpochs = 4;

   class DCGANGeneratorImpl : public torch::nn::Module {
   public:
      DCGANGeneratorImpl(int kNoiseSize);
      torch::Tensor forward(torch::Tensor x);

      torch::nn::ConvTranspose2d conv1, conv2, conv3, conv4;
      torch::nn::BatchNorm2d batch_norm1, batch_norm2, batch_norm3;
   };
   TORCH_MODULE(DCGANGenerator);

} // namespace mikado::torchBox

#endif // TBOX_DCGAN_GENERATOR_IMPL_H