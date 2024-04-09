#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_TORCHBASE_H)
#define TBOX_TORCHBASE_H

#include "torch-enums.h"

namespace mikado::torchBox {

   class TorchBase {
   public:
      TorchBase() = default;
      virtual ~TorchBase() = default;

      void setConfig(common::ConfigurePtr cfg) { cfg_ = cfg; }
      common::ConfigurePtr getConfig() const { return cfg_; }
      void setC10Device(c10::DeviceType c10device) { c10Device_ = c10device; }
      c10::DeviceType getC10Device() const { return c10Device_; }
      void setTorchDevice(torch::TensorOptions torchDevice) { torchDevice_ = torchDevice; }
      torch::TensorOptions getTorchDevice() const { return torchDevice_; }

   private:
      common::ConfigurePtr cfg_;
      c10::DeviceType c10Device_ = c10::kCUDA;
      torch::TensorOptions torchDevice_ = torch::kCUDA;
   };

} // namespace mikado::torchBox

#endif // TBOX_TORCHBASE_H
