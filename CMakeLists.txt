# 3.16 is required for target_precompile_headers(...PRECOMPILE_HEADERS_REUSE_FROM...)
# 3.25 is required for CMAKE_C_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT
cmake_minimum_required (VERSION 3.25)

project(MikadoV4)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_VS_NINJA_FORCE_COMMAND_LINE_BUILD 1)  # verbose mode from ninja
set(TORCH_USE_FIND_PACKAGE TRUE)
set(USE_GITHUB_IXWEBSOCKET TRUE) # vcpkg version or the local github image?

if (NOT TORCH_USE_FIND_PACKAGE)
   include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/torch-headers.cmake)
endif()

add_subdirectory(cfg)
add_subdirectory(common)
add_subdirectory(windowsApi)
add_subdirectory(imgui)
add_subdirectory(broker)
add_subdirectory(globber)
add_subdirectory(makeMore)
add_subdirectory(storage)
