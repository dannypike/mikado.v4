include(../cmake/dumpVariables.cmake)

if (${DUMP_VARIABLES}) # Set to FALSE to disable the following code
    message("")
    message("####################################################")
    message("List of variables for ${PROJECT_NAME}:")
    message("")
    dumpVariables()
    message("")
    message("End of the list of variables for ${PROJECT_NAME}")
    message("####################################################")
 endif()
 
 message("")
 message("End of CMakeLists.txt for ${PROJECT_NAME}")
 message("####################################################")
 