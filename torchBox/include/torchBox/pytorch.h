#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_PYTORCH_H)
#define TBOX_PYTORCH_H

namespace mikado::torchBox {

   std::vector<size_t> getShape(torch::Tensor const &tensor);
   
   ///////////////////////////////////////////////////////////////////////////
   //
   template <class TT>
   std::string toStringFlat(torch::Tensor const &tensor, int64_t count = -1, int64_t start = 0) {
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

   ///////////////////////////////////////////////////////////////////////////
   //
   template<class TT>
   void toStringTree(std::stringstream &ss, torch::Tensor tensor
         , std::vector<int64_t> const &extents
         , std::vector<int64_t> const *ellipsisAt = nullptr) {
      try
      {
         auto sizes = tensor.sizes();
         auto numel = tensor.numel();
         TT *data = tensor.data_ptr<TT>();

         typedef std::function<void(std::stringstream &, TT *&, int64_t)> TreeWalker;
         TreeWalker treeWalker
            = [&](std::stringstream &ss, TT *&data, int64_t level)
            {
               // limit the number of elements to print in this dimension?
               auto count = sizes[level];
               if (level < extents.size()) {
                  if (auto maxCount = extents[level]; maxCount >= 0) {
                     count = std::min(count, maxCount);
                  }
               }

               if (level == sizes.size() - 1) {
                  ss << "[";
                  for (int64_t ii = 0; ii < count; ++ii) {
                     if (0 < ii) {
                        ss << ", ";
                     }
                     ss << data[ii];
                  }
                  if (count < sizes[level]) {
                     ss << ", ...";
                  }
                  ss << "]";
                  data += sizes[level];
               }
               else {
                  ss << "[";
                  for (int64_t ii = 0; ii < count; ++ii) {
                     if (0 < ii) {
                        ss << ", ";
                     }
                     treeWalker(ss, data, level + 1);
                  }
                  if (count < sizes[level]) {
                     ss << ", ...";
                  }
                  ss << "]";
               }
            };

         TT *originalData = data;
         treeWalker(ss, data, 0);
      }
      catch (const std::exception &e)
      {
         log_exception(e);
      }
   }

} // namespace mikado::common

#endif // TBOX_PYTORCH_H
