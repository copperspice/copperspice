add_library(CsCore SHARED "")
add_library(CopperSpice::CsCore ALIAS CsCore)

set_target_properties(CsCore PROPERTIES OUTPUT_NAME CsCore${BUILD_ABI})

target_compile_definitions(CsCore
   PRIVATE
   -DQT_BUILD_CORE_LIB
   -DBUILDING_LIB_CS_SIGNAL
   -DBUILDING_LIB_CS_STRING
)

if (GTK2_FOUND)
   target_link_libraries(CsCore
      PRIVATE
      ${GLIB2_LIBRARIES}
      ${GOBJECT2_LIBRARIES}
   )

else()
   add_definitions(-DQT_NO_GLIB)

endif()

include_directories(
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/privateinclude

   ${CMAKE_BINARY_DIR}/include/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_CURRENT_SOURCE_DIR}/animation
   ${CMAKE_CURRENT_SOURCE_DIR}/arch
   ${CMAKE_CURRENT_SOURCE_DIR}/codecs
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent
   ${CMAKE_CURRENT_SOURCE_DIR}/global
   ${CMAKE_CURRENT_SOURCE_DIR}/guarded
   ${CMAKE_CURRENT_SOURCE_DIR}/io
   ${CMAKE_CURRENT_SOURCE_DIR}/json
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine
   ${CMAKE_CURRENT_SOURCE_DIR}/thread
   ${CMAKE_CURRENT_SOURCE_DIR}/tools
   ${CMAKE_CURRENT_SOURCE_DIR}/xml
   ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src
)

include(animation/animation.cmake)
include(codecs/codecs.cmake)
include(concurrent/concurrent.cmake)
include(containers/containers.cmake)
include(datetime/datetime.cmake)
include(global/global.cmake)
include(guarded/guarded.cmake)
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

if (ZLIB_FOUND)
   target_link_libraries(CsCore
      PUBLIC
      ${ZLIB_LIBRARIES}
   )

else()
   target_sources(CsCore
      PRIVATE
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
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   list(APPEND EXTRA_CORE_LDFLAGS
      -framework CoreFoundation
      -framework Foundation
      -framework Security
      -framework ApplicationServices
      -framework AppKit
      -lobjc
   )
   include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty/forkfd)
   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/global/Info.plist.app DESTINATION mac)

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
   target_link_libraries(CsCore
      PUBLIC
      dl
      ${CMAKE_THREAD_LIBS_INIT}
   )
   include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty/forkfd)

elseif(CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD|DragonFly)")
   target_link_libraries(CsCore
      PRIVATE
      ${CMAKE_THREAD_LIBS_INIT}
   )
   include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty/forkfd)

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_link_libraries(CsCore
      PRIVATE
      ole32
      ws2_32
      uuid
   )

endif()

function_variable_fixup("${EXTRA_CORE_CXXFLAGS}" EXTRA_CORE_CXXFLAGS)
function_variable_fixup("${EXTRA_CORE_LDFLAGS}"  EXTRA_CORE_LDFLAGS)

target_sources(CsCore
   PRIVATE
   ${CORE_INCLUDES}
   ${CORE_REGEX_INCLUDES}
)

if(MSVC)
   target_compile_options(CsCore
      PUBLIC
      /bigobj
      /DNOMINMAX
   )
endif()

target_include_directories(CsCore
   PUBLIC
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/QtCore>
)

set_target_properties(CsCore
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_CORE_CXXFLAGS}
   LINK_FLAGS    ${EXTRA_CORE_LDFLAGS}
)

function_generate_resources(CsCore)

macro_generate_package(
   CsCore
   QtCore
   "${EXTRA_CORE_CXXFLAGS}"
   ""
)

install(
   TARGETS CsCore
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

install(FILES ${CMAKE_BINARY_DIR}/include/cs-config.h  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )
