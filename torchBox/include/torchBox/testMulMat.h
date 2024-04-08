#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_MULMAT_H)
#define TBOX_MULMAT_H

#include "testBase.h"

namespace mikado::torchBox {

   class TestMulMat : public TestBase {
   public:
      TestMulMat() = default;

      virtual void run() override;
   };

} // namespace mikado::torchBox

#endif // TBOX_MULMAT_H
