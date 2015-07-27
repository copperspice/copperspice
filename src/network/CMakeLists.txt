add_definitions(-DQT_BUILD_NETWORK_LIB -DQT_MAKEDLL)
set(EXTRA_NETWORK_LIBS CsCore${BUILD_MAJOR})
set(EXTRA_NETWORK_CXXFLAGS "-Wno-undefined")
set(EXTRA_NETWORK_LDFLAGS " ") # whitespace is required!

if(NOT OPENSSL_FOUND)
    add_definitions(-DQT_NO_OPENSSL)
endif()

add_subdirectory(access)
add_subdirectory(bearer)
add_subdirectory(kernel)
add_subdirectory(socket)
add_subdirectory(ssl)

# TODO: make them macros to be shared between components
foreach(pubheader ${NETWORK_PUBLIC_INCLUDES})
    string(TOLOWER ${pubheader} pubname)
    get_filename_component(pubname ${pubname} NAME)
    set(pubout ${CMAKE_BINARY_DIR}/include/QtNetwork/${pubheader})
    message(STATUS "Writing public: ${pubout}")
    file(WRITE ${pubout} "#include <${pubname}.h>")
endforeach(pubheader)

foreach(privheader ${NETWORK_PRIVATE_INCLUDES})
    get_filename_component(privname ${privheader} NAME)
    set(privout ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork/${privname})
    message(STATUS "Writing private: ${privout}")
    file(COPY ${privheader} DESTINATION ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork/)
endforeach(privheader)

foreach(header ${NETWORK_INCLUDES})
    get_filename_component(headername ${header} NAME)
    set(headout ${CMAKE_BINARY_DIR}/include/QtNetwork/${headername})
    message(STATUS "Writing: ${headout}")
    file(COPY ${header} DESTINATION ${CMAKE_BINARY_DIR}/include/QtNetwork/)
endforeach(header)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtNetwork
    ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork
    ${CMAKE_CURRENT_SOURCE_DIR}/access
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/socket
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(EXTRA_NETWORK_LIBS
        ${EXTRA_NETWORK_LIBS} lws2_32
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(EXTRA_NETWORK_LDFLAGS
        ${EXTRA_NETWORK_LDFLAGS}
        -framework CoreServices
    )
endif()


add_library(CsNetwork${BUILD_MAJOR} SHARED ${NETWORK_SOURCES} ${NETWORK_INCLUDES})
target_link_libraries(CsNetwork${BUILD_MAJOR} ${EXTRA_NETWORK_LIBS})
set_target_properties(CsNetwork${BUILD_MAJOR} PROPERTIES
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_NETWORK_CXXFLAGS}
    LINK_FLAGS ${EXTRA_NETWORK_LDFLAGS}
)

install(TARGETS CsNetwork${BUILD_MAJOR} LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/QtNetwork DESTINATION include)
