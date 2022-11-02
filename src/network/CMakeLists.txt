add_library(CsNetwork SHARED "")
add_library(CopperSpice::CsNetwork ALIAS CsNetwork)

set_target_properties(CsNetwork PROPERTIES OUTPUT_NAME CsNetwork${BUILD_ABI})

target_compile_definitions(CsNetwork
   PRIVATE
   -DQT_BUILD_NETWORK_LIB
)

set(EXTRA_NETWORK_CXXFLAGS)
set(EXTRA_NETWORK_LDFLAGS)

if (SECURETRANSPORT_FOUND)
   # darwin
   target_compile_definitions(CsNetwork
      PUBLIC
      -DQT_SSL
      -DQT_SECURETRANSPORT
   )

elseif (OPENSSL_FOUND)
   target_compile_definitions(CsNetwork
      PUBLIC
      -DQT_SSL
      -DQT_OPENSSL
   )
   include_directories(${OPENSSL_INCLUDE_DIR})

   target_link_libraries(CsNetwork
      PRIVATE
      ${OPENSSL_LIBRARIES}
   )
endif()

include_directories(
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/privateinclude

   ${CMAKE_BINARY_DIR}/include/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_BINARY_DIR}/include/QtNetwork
   ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork/private

   ${CMAKE_CURRENT_SOURCE_DIR}/access
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel
   ${CMAKE_CURRENT_SOURCE_DIR}/socket
   ${CMAKE_CURRENT_SOURCE_DIR}/ssl
)

include(access/access.cmake)
include(bearer/bearer.cmake)
include(kernel/kernel.cmake)
include(socket/socket.cmake)
include(ssl/ssl.cmake)

macro_generate_public("${NETWORK_PUBLIC_INCLUDES}"   QtNetwork)
macro_generate_private("${NETWORK_PRIVATE_INCLUDES}" QtNetwork)
macro_generate_misc("${NETWORK_INCLUDES}" QtNetwork)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   list(APPEND EXTRA_NETWORK_CXXFLAGS
      -D__APPLE_USE_RFC_3542
   )

   list(APPEND EXTRA_NETWORK_LDFLAGS
      -framework CoreServices
      -framework Security
      -framework SystemConfiguration
   )
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_link_libraries(CsNetwork
      PRIVATE
      ws2_32
      crypt32
      dnsapi
      iphlpapi
    )
endif()

function_variable_fixup("${EXTRA_NETWORK_CXXFLAGS}" EXTRA_NETWORK_CXXFLAGS)
function_variable_fixup("${EXTRA_NETWORK_LDFLAGS}"  EXTRA_NETWORK_LDFLAGS)

target_sources(CsNetwork
   PRIVATE
   ${NETWORK_INCLUDES}
)

target_link_libraries(CsNetwork
   PRIVATE
   CsCore
)

target_include_directories(CsNetwork
   PUBLIC
   $<INSTALL_INTERFACE:include/QtNetwork>
)

set_target_properties(CsNetwork
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_NETWORK_CXXFLAGS}
   LINK_FLAGS ${EXTRA_NETWORK_LDFLAGS}
)

function_generate_resources(CsNetwork)

macro_generate_package(
   CsNetwork
   QtNetwork
   "${EXTRA_NETWORK_CXXFLAGS}"
   "CsCore"
)

install(
   TARGETS CsNetwork
   EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
   DIRECTORY ${CMAKE_BINARY_DIR}/include/QtNetwork
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   COMPONENT Devel
)
