#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_MULMAT_H)
#define TBOX_MULMAT_H

#include "testBase.h"

namespace mikado::torchBox {

   class TestMulMat : public TestBase {
   public:
      TestMulMat();

      virtual MikadoErrorCode configure(common::ConfigurePtr cfg) override;
      virtual MikadoErrorCode train() override;
   };

} // namespace mikado::torchBox

#endif // TBOX_MULMAT_H
