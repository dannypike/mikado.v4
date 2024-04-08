#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_TESTBASE_H)
#define TBOX_TESTBASE_H

#include "torchBase.h"

namespace mikado::torchBox {

   class TestBase : public std::enable_shared_from_this<TestBase>, public TorchBase {
   public:
      TestBase() = default;

      virtual void run() = 0;
   };

} // namespace mikado::torchBox

#endif // TBOX_TESTBASE_H
