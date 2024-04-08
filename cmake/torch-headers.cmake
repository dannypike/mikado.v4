# Set up to link to libtorch manually, because find_package is so slow
# and screws up the compiler switches

set(PYTORCH_CPP_DIR "E:/Projects/AI/Pytorch/cpp-2.21")
set(TORCH_BASE_DIR "${PYTORCH_CPP_DIR}/cuda-dbg-11.8/libtorch")
set(CUDA_BASE_DIR "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.8")
set(NVTOOLS_BASE_DIR "C:/Program Files/NVIDIA Corporation/NvToolsExt")

message("Setting Torch include directories explicitly to point at ${TORCH_BASE_DIR}")

set(TORCH_INCLUDE_DIRS
   "${TORCH_BASE_DIR}/include"
   "${TORCH_BASE_DIR}/include/torch/csrc/api/include"
   "${NVTOOLS_BASE_DIR}/include"
)
