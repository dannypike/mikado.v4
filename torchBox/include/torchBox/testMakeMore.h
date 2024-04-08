#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_MAKEMORE_H)
#define TBOX_MAKEMORE_H

#include "testBase.h"

namespace mikado::torchBox {

   class TestMakeMore : public TestBase {
   public:
      TestMakeMore() = default;

      virtual void run() override;
   };

} // namespace mikado::torchBox

#endif // TBOX_MAKEMORE_H
