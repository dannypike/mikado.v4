#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(MKO_TORCH_ENUMS_H)
#define MKO_TORCH_ENUMS_H

#if !defined(ENUM_HPP_CLASS_DECL)
#error "Please include the enum.hpp library from https://github.com/BlackMATov/enum.hpp/ before including this file"
#endif

//////////////////////////////////////////////////////////////////////////////
//
namespace c10 {

   ENUM_HPP_TRAITS_DECL(DeviceType,
      (CPU)
      (CUDA)
      (MKLDNN)
      (OPENGL)
      (OPENCL)
      (IDEEP)
      (HIP)
      (FPGA)
      (ORT)
      (XLA)
      (Vulkan)
      (Metal)
      (XPU)
      (MPS)
      (Meta)
      (HPU)
      (VE)
      (Lazy)
      (IPU)
      (MTIA)
      (PrivateUse1)
      (COMPILE_TIME_MAX_DEVICE_TYPES)
   )
   ENUM_HPP_REGISTER_TRAITS(DeviceType)

} // namespace c10

#endif // MKO_TORCH_ENUMS_H
