add_executable(cs_wayland_scanner${TOOLS_SUFFIX} "")
add_executable(CopperSpice::cs_wayland_scanner ALIAS cs_wayland_scanner${TOOLS_SUFFIX})

set(EXTRA_CS_WAYLAND_SCANNER_LIBS
   CsCore
   CsXml
)

set(EXTRA_CS_WAYLAND_SCANNER_CXXFLAGS)
set(EXTRA_CS_WAYLAND_SCANNER_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtXml/private

   ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(cs_wayland_scanner${TOOLS_SUFFIX}
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/cs_wayland_scanner.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   list(APPEND EXTRA_CS_WAYLAND_SCANNER_LDFLAGS '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
   list(APPEND EXTRA_CS_WAYLAND_SCANNER_LDFLAGS '-Wl,-rpath,$ORIGIN/../lib')
endif()

function_variable_fixup("${EXTRA_CS_WAYLAND_SCANNER_CXXFLAGS}" EXTRA_CS_WAYLAND_SCANNER_CXXFLAGS)
function_variable_fixup("${EXTRA_CS_WAYLAND_SCANNER_LDFLAGS}"  EXTRA_CS_WAYLAND_SCANNER_LDFLAGS)

target_link_libraries(cs_wayland_scanner${TOOLS_SUFFIX}
   PRIVATE
   ${EXTRA_CS_WAYLAND_SCANNER_LIBS}
)

target_compile_options(cs_wayland_scanner${TOOLS_SUFFIX}
   PRIVATE
   $<$<COMPILE_LANGUAGE:CXX>:SHELL:${EXTRA_CS_WAYLAND_SCANNER_CXXFLAGS}>
)

set_target_properties(cs_wayland_scanner${TOOLS_SUFFIX}
   PROPERTIES
   LINK_FLAGS ${EXTRA_CS_WAYLAND_SCANNER_LDFLAGS}
   EXPORT_NAME cs_wayland_scanner
)

install(
   TARGETS cs_wayland_scanner${TOOLS_SUFFIX}
   EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   DESTINATION ${CMAKE_INSTALL_BINDIR}
)
