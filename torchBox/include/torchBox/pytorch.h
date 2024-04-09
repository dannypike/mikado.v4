#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_PYTORCH_H)
#define TBOX_PYTORCH_H

namespace mikado::torchBox {

   std::vector<size_t> getShape(torch::Tensor const &tensor);

} // namespace mikado::common

#endif // TBOX_PYTORCH_H
