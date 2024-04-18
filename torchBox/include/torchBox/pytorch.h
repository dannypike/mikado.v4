#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(TBOX_PYTORCH_H)
#define TBOX_PYTORCH_H

namespace mikado::torchBox {

   std::vector<size_t> getShape(torch::Tensor const &tensor);

   ///////////////////////////////////////////////////////////////////////////
   //
   template<class TT>
   void toStringTree(std::stringstream &ss, torch::Tensor tensor
         , std::vector<int64_t> const &extents) {
      try
      {
         // If the tensor is on the CUDA device, move it to the CPU so that
         // the formatting code can treat it like a normal array.
         if (tensor.device().is_cuda()) {
            tensor = tensor.clone().to(c10::DeviceType::CPU);
         }

         auto sizes = tensor.sizes();
         auto numel = tensor.numel();
         
         // Wrap the data accessor in its own try block, as the type mismatch is
         // an easy mistake to make.
         TT *data = nullptr;
         try
         {
            data = tensor.data_ptr<TT>();
         }
         catch (const std::exception &e)
         {
            log_exception(e);
            return;
         }

         typedef std::function<void(std::stringstream &, TT *&, int64_t)> TreeWalkerType;
         TreeWalkerType treeWalker
            = [&](std::stringstream &ss, TT *&data, int64_t level)
            {
               // limit the number of elements to print in this dimension?
               auto count = sizes[level];
               if (level < extents.size()) {
                  if (auto maxCount = extents[level]; maxCount >= 0) {
                     count = std::min(count, maxCount);
                  }
               }

               ss << "[ ";
               if (level == sizes.size() - 1) {
                  for (int64_t ii = 0; ii < count; ++ii) {
                     if (0 < ii) {
                        ss << ", ";
                     }
                     ss << data[ii];
                  }
                  if (count < sizes[level]) {
                     ss << ", ...";
                  }
                  data += sizes[level];
               }
               else {
                  for (int64_t ii = 0; ii < count; ++ii) {
                     if (0 < ii) {
                        ss << ", ";
                     }
                     treeWalker(ss, data, level + 1);
                  }
                  if (count < sizes[level]) {
                     ss << ", ...";
                  }
               }
               ss << " ]";
            };

         TT *originalData = data;
         treeWalker(ss, data, 0);
      }
      catch (const std::exception &e)
      {
         log_exception(e);
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   template <class TT>
   std::string toStringFlat(torch::Tensor tensor, int64_t count = -1, int64_t start = 0) {
      std::stringstream ss;
      try
      {
         toStringTree<TT>(ss, tensor.view({ -1 }).slice(0, start), { count });
      }
      catch (const std::exception &e)
      {
         log_exception(e);
      }
      return ss.str();
   }

} // namespace mikado::common

#endif // TBOX_PYTORCH_H
