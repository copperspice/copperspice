add_definitions(-DQT_BUILD_CORE_LIB)
set(EXTRA_CORE_LIBS "")
set(EXTRA_CORE_CXXFLAGS "-Wno-undefined")
set(EXTRA_CORE_LDFLAGS " ") # whitespace is required!

add_subdirectory(animation)
add_subdirectory(arch)
add_subdirectory(codecs)
add_subdirectory(concurrent)
add_subdirectory(global)
add_subdirectory(io)
add_subdirectory(json)
add_subdirectory(kernel)
add_subdirectory(plugin)
add_subdirectory(statemachine)
add_subdirectory(thread)
add_subdirectory(tools)
add_subdirectory(xml)

# TODO: make them macros to be shared between components
foreach(pubheader ${CORE_PUBLIC_INCLUDES})
    string(TOLOWER ${pubheader} pubname)
    get_filename_component(pubname ${pubname} NAME)
    set(pubout ${CMAKE_BINARY_DIR}/include/QtCore/${pubheader})
    message(STATUS "Writing public: ${pubout}")
    file(WRITE ${pubout} "#include <${pubname}.h>")
endforeach(pubheader)

foreach(privheader ${CORE_PRIVATE_INCLUDES})
    get_filename_component(privname ${privheader} NAME)
    set(privout ${CMAKE_BINARY_DIR}/privateinclude/QtCore/${privname})
    message(STATUS "Writing private: ${privout}")
    file(COPY ${privheader} DESTINATION ${CMAKE_BINARY_DIR}/privateinclude/QtCore/)
endforeach(privheader)

foreach(header ${CORE_INCLUDES})
    get_filename_component(headername ${header} NAME)
    set(headout ${CMAKE_BINARY_DIR}/include/QtCore/${headername})
    message(STATUS "Writing: ${headout}")
    file(COPY ${header} DESTINATION ${CMAKE_BINARY_DIR}/include/QtCore/)
endforeach(header)

if(ZLIB_FOUND)
    set(EXTRA_CORE_LIBS
        ${EXTRA_CORE_LIBS}
        ${ZLIB_LIBRARIES}
    )
else(ZLIB_FOUND)
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/adler32.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/compress.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/crc32.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/deflate.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/gzclose.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/gzlib.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/gzread.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/gzwrite.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/infback.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/inffast.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/inflate.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/inftrees.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/trees.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/uncompr.c
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib/zutil.c
    )
    include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty)
endif(ZLIB_FOUND)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_CURRENT_SOURCE_DIR}/animation
    ${CMAKE_CURRENT_SOURCE_DIR}/arch
    ${CMAKE_CURRENT_SOURCE_DIR}/codecs
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrent
    ${CMAKE_CURRENT_SOURCE_DIR}/global
    ${CMAKE_CURRENT_SOURCE_DIR}/io
    ${CMAKE_CURRENT_SOURCE_DIR}/json
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine
    ${CMAKE_CURRENT_SOURCE_DIR}/thread
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/xml
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(EXTRA_CORE_LIBS
        ${EXTRA_CORE_LIBS} dl pthread
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(EXTRA_CORE_LIBS
        ${EXTRA_CORE_LIBS} ole32 ws2_32
    )
    set(EXTRA_CORE_LDFLAGS
        ${EXTRA_CORE_LDFLAGS}
        -Wl,-luuid
    )
    add_definitions(-DQT_MAKEDLL)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(EXTRA_CORE_LDFLAGS
        ${EXTRA_CORE_LDFLAGS}
        -framework CoreFoundation
        -framework Security
        -framework ApplicationServices
        -lObjC
    )
    # FIXME: where does this go? some variables must be replaced too
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/global/Info.plist.app DESTINATION mac)
endif()


add_library(CsCore1 SHARED ${CORE_SOURCES} ${CORE_INCLUDES})
target_link_libraries(CsCore1 ${EXTRA_CORE_LIBS})
set_target_properties(CsCore1 PROPERTIES
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_CORE_CXXFLAGS}
    LINK_FLAGS ${EXTRA_CORE_LDFLAGS}
)

install(TARGETS CsCore1 LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/QtCore DESTINATION include)
