# Settings and switches that are common to all Mikado executable projects

add_executable(${TARGET} ${SRC_FILES})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${TARGET})

target_link_libraries(${TARGET} PRIVATE
   ${LIB_FOLDER}/windowsApi.lib
   ${LIB_FOLDER}/common.lib
   ${LIB_FOLDER}/imgui.lib
   bcrypt.lib
)
