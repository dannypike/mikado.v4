# Settings and switches that are common to all Mikado projects

set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)	# std::jthread requires C++20

target_include_directories(${TARGET} PUBLIC
   ${COMMON_FOLDER}/include
   ${API_FOLDER}/include
   ${IMGUI_FOLDER}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${TORCH_INCLUDE_DIRS}
)

# Include ixWebSocket stuff
include(${CMAKE_FOLDER}/ixwebsocket.cmake)

# Import the standard precompiled headers
set(PRECOMPILED_HEADER_FILES
   # Standard Library
   <cassert>
   <filesystem>
   <source_location>
   <string>
	<thread>

   # Windows API
   <winsock2.h>
	"../include/windows-hack.h"	# undef's min and max
	<wincrypt.h>
	<d3d9.h>	# Direct3D #9

	# OpenSSL
	<openssl/bio.h>
	<openssl/pem.h>
	<openssl/x509.h>

	# IXWebsocket
   ${IXWEBSOCKET_INCLUDE_FILES}

   # Database
   <sqlite3.h>

   # Boost
   <boost/algorithm/string/predicate.hpp>
   <boost/date_time/posix_time/posix_time.hpp>
   <boost/dll.hpp>
   <boost/iostreams/concepts.hpp>
   <boost/iostreams/stream_buffer.hpp>
   <boost/json.hpp>
   <boost/lexical_cast.hpp>
   <boost/locale.hpp>
   <boost/lockfree/spsc_queue.hpp>
   <boost/program_options.hpp>
   <boost/tuple/tuple.hpp>
   <boost/uuid/uuid.hpp>
   <boost/uuid/uuid_generators.hpp>
   <boost/uuid/uuid_io.hpp>
)

if(${TARGET} STREQUAL "common")
   target_precompile_headers(${TARGET} PRIVATE ${PRECOMPILED_HEADER_FILES})
elseif(${TARGET} STREQUAL "makeMore")
   # We need a separate-but-identical list for makeMore, because Torch is
   # using different compiler switches (spefically the debug type)
   target_precompile_headers(${TARGET} PRIVATE ${PRECOMPILED_HEADER_FILES})
else()
   # Everything else uses the same precompiled file as common
   target_precompile_headers(${TARGET} REUSE_FROM common)
endif()

find_package(Boost REQUIRED COMPONENTS date_time filesystem json program_options system)
target_include_directories(${TARGET} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${TARGET} PRIVATE ${Boost_LIBRARIES})

find_package(OpenSSL REQUIRED)
target_link_libraries(${TARGET} PRIVATE ${OPENSSL_LIBRARIES})
target_link_libraries(${TARGET} PRIVATE Crypt32 d3d9.lib)
