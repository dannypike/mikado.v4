# 3.16 is required for target_precompile_headers(...PRECOMPILE_HEADERS_REUSE_FROM...)
# 3.25 is required for CMAKE_C_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT
cmake_minimum_required (VERSION 3.25)

project(MikadoV4)

set(USE_GITHUB_IXWEBSOCKET TRUE)
if (USE_GITHUB_IXWEBSOCKET)
   message("Compiling using local sources for ixWebsocket")
   set(IXWEBSOCKET_INCLUDE_FILES "${CMAKE_SOURCE_DIR}/IXWebSocket/ixwebsocket/IXWebSocketServer.h>")
else()
   set(IXWEBSOCKET_INCLUDE_FILES <ixwebsocket/IXWebSocketServer.h>)
endif()

add_subdirectory(cfg)
add_subdirectory(common)
add_subdirectory(windowsApi)
add_subdirectory(broker)
add_subdirectory(storage)
add_subdirectory(globber)
