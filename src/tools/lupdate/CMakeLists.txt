add_executable(lupdate${TOOLS_SUFFIX} "")
add_executable(CopperSpice::lupdate ALIAS lupdate${TOOLS_SUFFIX})

list(APPEND EXTRA_LUPDATE_LIBS
   CsCore
   CsXml
)

set(EXTRA_LUPDATE_CXXFLAGS)
set(EXTRA_LUPDATE_LDFLAGS)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude

    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

    ${CMAKE_BINARY_DIR}/include/QtXml
    ${CMAKE_BINARY_DIR}/privateinclude/QtXml/private

    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/tools/shared
)

target_sources(lupdate${TOOLS_SUFFIX}
   PRIVATE
   ${CMAKE_SOURCE_DIR}/src/tools/shared/similartext.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/merge.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/java.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/ui.cpp
   $<TARGET_OBJECTS:sharedtools>
)

if(CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   set(EXTRA_LUPDATE_LDFLAGS  ${EXTRA_LUPDATE_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "DARWIN")
   set(EXTRA_LUPDATE_LDFLAGS  ${EXTRA_LUPDATE_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
   set(EXTRA_LUPDATE_LDFLAGS  ${EXTRA_LUPDATE_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   list(APPEND EXTRA_LUPDATE_LIBS
      uuid
   )

   target_sources(lupdate${TOOLS_SUFFIX}
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/lupdate.exe.manifest
   )
endif()

function_variable_fixup("${EXTRA_LUPDATE_CXXFLAGS}" EXTRA_LUPDATE_CXXFLAGS)
function_variable_fixup("${EXTRA_LUPDATE_LDFLAGS}" EXTRA_LUPDATE_LDFLAGS)

target_link_libraries(lupdate${TOOLS_SUFFIX} ${EXTRA_LUPDATE_LIBS})

set_target_properties(lupdate${TOOLS_SUFFIX}
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_LUPDATE_CXXFLAGS}
   LINK_FLAGS ${EXTRA_LUPDATE_LDFLAGS}
   EXPORT_NAME lupdate
)

function_generate_resources(lupdate${TOOLS_SUFFIX})

install(
    TARGETS lupdate${TOOLS_SUFFIX}
    EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
