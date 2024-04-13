// Copyright (c) 2024 Gamaliel Ltd
#include "common.h"
#include "torchBox/pytorch.h"

using namespace std;

namespace mikado::torchBox {

   ///////////////////////////////////////////////////////////////////////////
   //
   vector<size_t> getShape(torch::Tensor const &tensor) {
      vector<size_t> shape;
      for (auto dim : tensor.sizes()) {
         shape.push_back(dim);
      }
      return move(shape);
   }

} // namespace mikado::torchBox
