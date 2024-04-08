# 3.16 is required for target_precompile_headers(...PRECOMPILE_HEADERS_REUSE_FROM...)
# 3.25 is required for CMAKE_C_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT
cmake_minimum_required (VERSION 3.25)

cmake_policy(SET CMP0141 NEW)  # Enable the CMAKE_MSVC_DEBUG_INFORMATION_FORMAT variable
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "EditAndContinue")

project(MikadoV4)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_VS_NINJA_FORCE_COMMAND_LINE_BUILD 1)  # verbose mode from ninja
set(TORCH_USE_VCPKG 1)
set(TORCH_USE_FIND_PACKAGE 0)
set(TORCH_USE_EXPLICIT 0)
math(EXPR TORCH_USE_ANY "${TORCH_USE_FIND_PACKAGE} | ${TORCH_USE_VCPKG} | ${TORCH_USE_EXPLICIT}")
message("TORCH_USE_ANY=${TORCH_USE_ANY}")
set(USE_MIALLOC ${TORCH_USE_VCPKG})
set(USE_GITHUB_IXWEBSOCKET TRUE) # vcpkg version or the local github image?

if(USE_MIMALLOC)
   # mimalloc *must* be linked first, so be careful with specifying the link order
   find_package(mimalloc REQUIRED)
endif()

# Need this here so that we can generate the precompiled headers
if(TORCH_USE_VCPKG)
   find_package(Torch CONFIG REQUIRED)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
elseif(TORCH_USE_EXPLICIT)
   include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/torch-headers.cmake)
endif()

add_subdirectory(cfg)
add_subdirectory(common)
add_subdirectory(windowsApi)
add_subdirectory(imgui)
add_subdirectory(broker)
add_subdirectory(globber)
add_subdirectory(torchBox)
add_subdirectory(storage)
