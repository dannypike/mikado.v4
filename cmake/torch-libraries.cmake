# Do torch manually because find_package is so slow

# This should inherit the variables from an earlier include(torch-headers.cmake) call
set(TORCH_LIBRARIES
   "${TORCH_BASE_DIR}/lib/torch.lib"
   "${TORCH_BASE_DIR}/lib/torch_cuda.lib"
   "${TORCH_BASE_DIR}/lib/kineto.lib"
   "${NVTOOLS_BASE_DIR}/lib/x64/nvToolsExt64_1.lib"
   "${CUDA_BASE_DIR}/lib/x64/cudart_static.lib"
   "${TORCH_BASE_DIR}/lib/caffe2_nvrtc.lib"
   "${TORCH_BASE_DIR}/lib/c10_cuda.lib"
   "${TORCH_BASE_DIR}/lib/c10.lib"
)

target_link_libraries(${TARGET} PRIVATE ${TORCH_LIBRARIES})
