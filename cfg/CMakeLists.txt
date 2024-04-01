# Copyright (c) 2024 Gamaliel Ltd
#
# Configuration files are copied to the cfg/v4 folder
#
# 3.16 is required for target_precompile_headers(...PRECOMPILE_HEADERS_REUSE_FROM...)
# 3.25 is required for CMAKE_C_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT
cmake_minimum_required (VERSION 3.25)

set(CMAKE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(${CMAKE_FOLDER}/header.cmake)
set(DUMP_VARIABLES FALSE)

set(PROJECT_NAME Configuration)
set(TARGET cfg)

add_library(${TARGET} INTERFACE)

# Copy configuration
file(GLOB CFG_FILES "response/*.*")
foreach(II IN ITEMS ${CFG_FILES})
   get_filename_component(CFG_FILE_NAME ${II} NAME)  
   message("copying file ${II}")
   configure_file(response/${CFG_FILE_NAME} ${CFG_FOLDER}/${CFG_FILE_NAME} COPYONLY)
endforeach()

include(${CMAKE_FOLDER}/footer.cmake)
