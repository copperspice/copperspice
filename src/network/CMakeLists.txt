add_definitions(
    -DQT_BUILD_NETWORK_LIB
)
set(EXTRA_NETWORK_LIBS CsCore${BUILD_ABI})
set(EXTRA_NETWORK_CXXFLAGS)
set(EXTRA_NETWORK_LDFLAGS)

# these lines must come before the other includes, os x ordering issue
if (SECURETRANSPORT_FOUND)
    add_definitions(-DQT_SSL)
    add_definitions(-DQT_SECURETRANSPORT)

elseif (OPENSSL_FOUND)
    add_definitions(-DQT_SSL)
    add_definitions(-DQT_OPENSSL)

    include_directories(${OPENSSL_INCLUDE_DIR})
    set(EXTRA_NETWORK_LIBS
        ${EXTRA_NETWORK_LIBS} ${OPENSSL_LIBRARIES}
    )
endif()

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/include/QtNetwork
    ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork
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

macro_generate_public("${NETWORK_PUBLIC_INCLUDES}" QtNetwork)
macro_generate_private("${NETWORK_PRIVATE_INCLUDES}" QtNetwork)
macro_generate_misc("${NETWORK_INCLUDES}" QtNetwork)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(EXTRA_NETWORK_LIBS
        ${EXTRA_NETWORK_LIBS} ws2_32 crypt32 dnsapi iphlpapi
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(EXTRA_NETWORK_CXXFLAGS
        ${EXTRA_NETWORK_CXXFLAGS}
        -D__APPLE_USE_RFC_3542
    )
    set(EXTRA_NETWORK_LDFLAGS
        ${EXTRA_NETWORK_LDFLAGS}
        -framework CoreServices -framework Security
    )
endif()

function_variable_fixup("${EXTRA_NETWORK_CXXFLAGS}" EXTRA_NETWORK_CXXFLAGS)
function_variable_fixup("${EXTRA_NETWORK_LDFLAGS}" EXTRA_NETWORK_LDFLAGS)

add_library(CsNetwork${BUILD_ABI} SHARED ${NETWORK_SOURCES} ${NETWORK_INCLUDES})
target_link_libraries(CsNetwork${BUILD_ABI} ${EXTRA_NETWORK_LIBS})

target_include_directories(CsNetwork${BUILD_ABI}
   PUBLIC
   $<INSTALL_INTERFACE:include/QtNetwork>
)

set_target_properties(CsNetwork${BUILD_ABI} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_NETWORK_CXXFLAGS}
    LINK_FLAGS ${EXTRA_NETWORK_LDFLAGS}
)

macro_generate_resources("${NETWORK_SOURCES}")
macro_generate_package(
    CsNetwork${BUILD_ABI}
    QtNetwork
    "${EXTRA_NETWORK_CXXFLAGS}"
    "CsCore${BUILD_ABI}"
)

install(
    TARGETS CsNetwork${BUILD_ABI}
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
