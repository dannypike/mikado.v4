#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(MKO_TORCHBOX_H)
#define MKO_TORCHBOX_H

#include "common/fyi.h"
#include "common/globals.h"

namespace mikado::torchBox {

   unsigned int const TORCHBOX_VERSION_MAJOR = 0;
   unsigned int const TORCHBOX_VERSION_MINOR = 1;

   class TorchBox : public std::enable_shared_from_this<TorchBox>, public common::BrokerFYI {
   public:
      TorchBox()
      {
      }
      static void outputBanner();

      common::MikadoErrorCode configure(common::ConfigurePtr cfg);
      common::MikadoErrorCode start();
      common::MikadoErrorCode stop();

   protected:
      void onBrokerMessage(common::WebSocketPtr broker, ix::WebSocketMessagePtr const &msg);

   private:
      common::ConfigurePtr cfg_;
      common::WebSocketPtr broker_;
      c10::DeviceType c10Device_ = c10::DeviceType::CUDA;   // Default to CUDA and fallback if not found
      torch::TensorOptions torchDevice_ = torch::kCUDA;     // Same value as C10
      std::vector<std::string> testNames_ { common::kMulMat, common::kMakeMore };
   };
    
} // namespace mikado::torchBox

#endif // MKO_TORCHBOX_H
