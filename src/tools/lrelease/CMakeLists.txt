add_executable(lrelease${TOOLS_SUFFIX} "")
add_executable(CopperSpice::lrelease ALIAS lrelease${TOOLS_SUFFIX})

set(EXTRA_LRELEASE_LIBS
   CsCore
   CsXml
)

set(EXTRA_LRELEASE_CXXFLAGS)
set(EXTRA_LRELEASE_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtXml/private

   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_SOURCE_DIR}/src/tools/shared
)

target_sources(lrelease${TOOLS_SUFFIX}
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
   $<TARGET_OBJECTS:sharedtools>
)

if(CMAKE_SYSTEM_NAME MATCHES "DARWIN")
   list(APPEND EXTRA_LRELEASE_LDFLAGS  '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
   list(APPEND EXTRA_LRELEASE_LDFLAGS '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   list(APPEND EXTRA_LRELEASE_LDFLAGS '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   list(APPEND EXTRA_LRELEASE_LIBS uuid)
endif()

function_variable_fixup("${EXTRA_LRELEASE_CXXFLAGS}" EXTRA_LRELEASE_CXXFLAGS)
function_variable_fixup("${EXTRA_LRELEASE_LDFLAGS}"  EXTRA_LRELEASE_LDFLAGS)

target_link_libraries(lrelease${TOOLS_SUFFIX}
   PRIVATE
   ${EXTRA_LRELEASE_LIBS}
)

set_target_properties(lrelease${TOOLS_SUFFIX}
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_LRELEASE_CXXFLAGS}
   LINK_FLAGS ${EXTRA_LRELEASE_LDFLAGS}
   EXPORT_NAME lrelease
)

#
#   If the files in bin didn't name collide with Qt binaries we could just leave them.
#
if(BUILDING_RPM)
    install(
       TARGETS lrelease${TOOLS_SUFFIX}
       EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
       DESTINATION ${CMAKE_INSTALL_LIBDIR}/copperspice/bin
    )
else()
    install(
       TARGETS lrelease${TOOLS_SUFFIX}
       EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
       DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()