message("####################################################")
message("Processing CMakeLists.txt for ${PROJECT_NAME}")
message("")

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(USE_GITHUB_IXWEBSOCKET TRUE)

set(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../common)
set(API_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../windowsApi)
set(IMGUI_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../imgui)
set(LIB_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
set(CFG_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../bin/cfg)

# Enable Hot Reload for MSVC compilers if supported.
# This MUST be set before the project() statement or it will not be possible to
# use Edit And Continue (https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_DEBUG_INFORMATION_FORMAT.html)
if (POLICY CMP0141)
   cmake_policy(SET CMP0141 NEW)
endif()
project(${PROJECT_NAME})

set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
set(Boost_NO_WARN_NEW_VERSIONS 1)	# Suppress version warning for Boost
include(${CMAKE_FOLDER}/debugZI.cmake)

