# Settings and switches that are common to all Mikado library projects

add_library(${TARGET} STATIC ${SRC_FILES})

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${TARGET})

target_link_libraries(${TARGET} PRIVATE
   ${LIB_FOLDER}/windowsApi.lib
   ${LIB_FOLDER}/common.lib
   ${LIB_FOLDER}/imgui.lib
   bcrypt.lib
)
