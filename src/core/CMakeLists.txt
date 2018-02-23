add_definitions(
   -DQT_BUILD_CORE_LIB
   -DBUILDING_LIB_CS_SIGNAL
   -DBUILDING_LIB_CS_STRING
)

set(EXTRA_CORE_LIBS ${GLIB2_LIBRARIES} ${GOBJECT2_LIBRARIES})
set(EXTRA_CORE_CXXFLAGS)
set(EXTRA_CORE_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/privateinclude
   ${CMAKE_BINARY_DIR}/include/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
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
   ${GLIB2_INCLUDES}
   ${GOBJECT2_INCLUDES}
   ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src
)

include(animation/animation.cmake)
include(codecs/codecs.cmake)
include(concurrent/concurrent.cmake)
include(global/global.cmake)
include(io/io.cmake)
include(json/json.cmake)
include(kernel/kernel.cmake)
include(plugin/plugin.cmake)
include(signal/signal.cmake)
include(statemachine/statemachine.cmake)
include(string/string.cmake)
include(thread/thread.cmake)
include(tools/tools.cmake)
include(xml/xml.cmake)

macro_generate_public("${CORE_PUBLIC_INCLUDES}" QtCore)
macro_generate_private("${CORE_PRIVATE_INCLUDES}" QtCore)
macro_generate_misc("${CORE_INCLUDES}" QtCore)
macro_generate_misc("${CORE_REGEX_INCLUDES}" QtCore/regex)

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

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(EXTRA_CORE_LIBS
      ${EXTRA_CORE_LIBS}
      dl
      ${CMAKE_THREAD_LIBS_INIT}
   )
   include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty/forkfd)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   set(EXTRA_CORE_LIBS
      ${EXTRA_CORE_LIBS}
      ole32
      ws2_32
   )
   set(EXTRA_CORE_LDFLAGS
      ${EXTRA_CORE_LDFLAGS}
      -Wl,-luuid
   )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   set(EXTRA_CORE_LDFLAGS
      ${EXTRA_CORE_LDFLAGS}
      -framework CoreFoundation
      -framework Foundation
      -framework Security
      -framework ApplicationServices
      -lobjc
   )
   include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty/forkfd)
   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/global/Info.plist.app DESTINATION mac)
endif()

function_variable_fixup("${EXTRA_CORE_CXXFLAGS}" EXTRA_CORE_CXXFLAGS)
function_variable_fixup("${EXTRA_CORE_LDFLAGS}" EXTRA_CORE_LDFLAGS)

add_library(CsCore${BUILD_ABI} SHARED ${CORE_SOURCES} ${CORE_INCLUDES} ${CORE_REGEX_INCLUDES})
target_link_libraries(CsCore${BUILD_ABI} ${EXTRA_CORE_LIBS})

target_include_directories(CsCore${BUILD_ABI}
   PUBLIC
   $<INSTALL_INTERFACE:include>
   $<INSTALL_INTERFACE:include/QtCore>
)

set_target_properties(CsCore${BUILD_ABI} PROPERTIES
   VERSION "0" # for compat
   SOVERSION "0.0.0" # for compat
   COMPILE_FLAGS ${EXTRA_CORE_CXXFLAGS}
   LINK_FLAGS ${EXTRA_CORE_LDFLAGS}
)

macro_generate_resources("${CORE_SOURCES}")
macro_generate_package(
   CsCore${BUILD_ABI}
   QtCore
   "${EXTRA_CORE_CXXFLAGS}"
   ""
)

install(
   TARGETS CsCore${BUILD_ABI}
   EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
   DIRECTORY ${CMAKE_BINARY_DIR}/include/QtCore
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   COMPONENT Devel
)

# one file
install(FILES ${CMAKE_BINARY_DIR}/include/qt-acconfig.h  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )