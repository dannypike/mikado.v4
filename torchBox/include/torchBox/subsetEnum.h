#pragma once
// Copyright (c) Gamaliel Ltd
#if !defined(MKO_SUBSET_ENUM_H)
#define MKO_SUBSET_ENUM_H

namespace mikado::torchBox {
   ENUM_HPP_CLASS_DECL(Subset, int,
      (TrainX = 0)
      (TrainY)
      (DevelopX)
      (DevelopY)
      (TestX)
      (TestY)
      (SubsetCount)
   )
   inline std::ostream &operator<<(std::ostream &os, Subset subset) {
      return os << Subset_traits::to_string(subset)
         .value_or<std::string>(common::unknownEnumAsString((int)subset));
   }
} // namespace mikado::torchBox

#endif // MKO_SUBSET_ENUM_H