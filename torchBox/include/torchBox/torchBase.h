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
      void setDeviceType(c10::DeviceType device) { device_ = device; }
      c10::DeviceType getDeviceType() const { return device_; }

   private:
      common::ConfigurePtr cfg_;
      c10::DeviceType device_ = c10::kCUDA;
   };

} // namespace mikado::torchBox

#endif // TBOX_TORCHBASE_H
