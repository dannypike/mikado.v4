# 3.16 is required for target_precompile_headers(...PRECOMPILE_HEADERS_REUSE_FROM...)
# 3.25 is required for CMAKE_C_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT
cmake_minimum_required (VERSION 3.25)

project(MikadoV4)

add_subdirectory(common)
add_subdirectory(windowsApi)
add_subdirectory(globber)
