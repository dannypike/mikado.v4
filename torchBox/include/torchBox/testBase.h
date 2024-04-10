#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_TESTBASE_H)
#define TBOX_TESTBASE_H

#include "torchBase.h"

namespace mikado::torchBox {

   typedef common::MikadoErrorCode MikadoErrorCode;

   class TestBase : public std::enable_shared_from_this<TestBase>, public TorchBase {
   public:
      TestBase(std::string const &testName);

      virtual std::string const &getName() const {
         return testName_;
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
      std::string testName_;
   };
   typedef std::shared_ptr<TestBase> TestBasePtr;

} // namespace mikado::torchBox

#endif // TBOX_TESTBASE_H
