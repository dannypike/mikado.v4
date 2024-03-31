if (USE_GITHUB_IXWEBSOCKET)
   # Link the local version of ixWebsocket (downloaded and build from Github)
   message("Compiling using local sources for ixWebsocket")
   set(IXWEBSOCKET_INCLUDE_FILES "${CMAKE_SOURCE_DIR}/IXWebSocket/ixwebsocket/IXWebSocketServer.h")

   find_package(ZLIB REQUIRED)
   target_link_libraries(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/IXWebSocket/build/Debug/ixwebsocket.lib ws2_32.lib ${ZLIB_LIBRARIES})
else()
   message("Compiling using the VCPKG version of ixWebsocket")
   set(IXWEBSOCKET_INCLUDE_FILES <ixwebsocket/IXWebSocketServer.h>)

   # Link the ixWebsocket installed via vcpkg
   find_package(ixwebsocket CONFIG REQUIRED)
   target_link_libraries(${TARGET} PRIVATE ixwebsocket::ixwebsocket)
endif()
