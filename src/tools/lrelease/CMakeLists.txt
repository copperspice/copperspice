# add_definitions()
set(EXTRA_LRELEASE_LIBS CsCore${BUILD_ABI} CsXml${BUILD_ABI})
set(EXTRA_LRELEASE_CXXFLAGS)
set(EXTRA_LRELEASE_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/privateinclude
   ${CMAKE_BINARY_DIR}/include/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
   ${CMAKE_BINARY_DIR}/include/QtXml
   ${CMAKE_BINARY_DIR}/privateinclude/QtXml
   ${CMAKE_BINARY_DIR}/privateinclude/QtXml/private
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_SOURCE_DIR}/src/tools/shared
)

set(LRELEASE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ioutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   set(EXTRA_LRELEASE_LDFLAGS  ${EXTRA_LRELEASE_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "DARWIN")
   set(EXTRA_LRELEASE_LDFLAGS  ${EXTRA_LRELEASE_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(EXTRA_LRELEASE_LDFLAGS  ${EXTRA_LRELEASE_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   set(EXTRA_LRELEASE_LIBS ${EXTRA_LRELEASE_LIBS} uuid)
endif()

function_variable_fixup("${EXTRA_LRELEASE_CXXFLAGS}" EXTRA_LRELEASE_CXXFLAGS)
function_variable_fixup("${EXTRA_LRELEASE_LDFLAGS}"  EXTRA_LRELEASE_LDFLAGS)

add_executable(lrelease${TOOLS_SUFFIX} ${LRELEASE_SOURCES} $<TARGET_OBJECTS:sharedtools>)
target_link_libraries(lrelease${TOOLS_SUFFIX} ${EXTRA_LRELEASE_LIBS})

add_executable(CopperSpice::lrelease ALIAS lrelease${TOOLS_SUFFIX})

set_target_properties(lrelease${TOOLS_SUFFIX} PROPERTIES
   COMPILE_FLAGS ${EXTRA_LRELEASE_CXXFLAGS}
   LINK_FLAGS ${EXTRA_LRELEASE_LDFLAGS}
   EXPORT_NAME lrelease
)

install(
   TARGETS lrelease${TOOLS_SUFFIX}
   EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   DESTINATION ${CMAKE_INSTALL_BINDIR}
)
