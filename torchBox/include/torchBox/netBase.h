#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_NETBASE_H)
#define TBOX_NETBASE_H

#include "torchBase.h"

namespace mikado::torchBox {

   typedef common::MikadoErrorCode MikadoErrorCode;

   class NetBase : public std::enable_shared_from_this<NetBase>, public TorchBase {
   public:
      NetBase(std::string const &name);

      virtual std::string const &getName() const {
         return name_;
      }

      virtual MikadoErrorCode configure(common::ConfigurePtr cfg) {
         return MikadoErrorCode::MKO_STATUS_NOOP;
      }
      virtual MikadoErrorCode train() {
         return MikadoErrorCode::MKO_STATUS_NOOP;
      }
      virtual MikadoErrorCode verify() {
         return MikadoErrorCode::MKO_STATUS_NOOP;
      }

   private:
      std::string name_;
   };
   typedef std::shared_ptr<NetBase> NetBasePtr;

} // namespace mikado::torchBox

#endif // TBOX_NETBASE_H
