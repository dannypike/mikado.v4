#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_PYTORCH_H)
#define TBOX_PYTORCH_H

namespace mikado::torchBox {

   std::vector<size_t> getShape(torch::Tensor const &tensor);
   
   ///////////////////////////////////////////////////////////////////////////
   //
   template <class TT>
   std::string toString(torch::Tensor const &tensor, int64_t count = -1, int64_t start = 0) {
      std::stringstream ss;
      try
      {
         auto view = tensor.view({ -1 });
         auto sizes = view.sizes();
         auto end = sizes[0] - 1;
         auto from = std::min(std::max((int64_t)0, start), end);
         auto to = (count < 0) ? end : std::min(std::max(from, start + count - 1), end);

         auto data = view.accessor<TT, 1>();
         ss << "[";
         for (int64_t ii = from; ii <= to; ++ii) {
            if (from < ii) {
               ss << ", ";
            }
            ss << data[ii];
         }
         if (to < end) {
            ss << ", ...";
         }
         ss << "]";
      }
      catch (const std::exception &e)
      {
         log_exception(e);
      }
      return ss.str();
   }
} // namespace mikado::common

#endif // TBOX_PYTORCH_H
