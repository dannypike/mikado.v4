message("####################################################")
message("Processing CMakeLists.txt for ${PROJECT_NAME}")
message("")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(DUMP_VARIABLES FALSE)

# Enable Hot Reload for MSVC compilers if supported.
# This MUST be set before the project() statement or it will not be possible to
# use Edit And Continue (https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_DEBUG_INFORMATION_FORMAT.html)
if (POLICY CMP0141)
   cmake_policy(SET CMP0141 NEW)
endif()
project(${PROJECT_NAME})

set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
set(Boost_NO_WARN_NEW_VERSIONS 1)	# Suppress version warning for Boost
string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
