add_executable(lconvert${TOOLS_SUFFIX} "")
add_executable(CopperSpice::lconvert ALIAS lconvert${TOOLS_SUFFIX})

add_definitions(
   -DPROEVALUATOR_CUMULATIVE
   -DPROEVALUATOR_INIT_PROPS
)

set(EXTRA_LCONVERT_LIBS
   CsCore
   CsXml
)

set(EXTRA_LCONVERT_CXXFLAGS)
set(EXTRA_LCONVERT_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtXml/private

   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_SOURCE_DIR}/src/tools/shared
)

target_sources(lconvert${TOOLS_SUFFIX}
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
   $<TARGET_OBJECTS:sharedtools>
)

if(CMAKE_SYSTEM_NAME MATCHES "DARWIN")
   list(APPEND EXTRA_LCONVERT_LDFLAGS  '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   list(APPEND EXTRA_LCONVERT_LDFLAGS  '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
   list(APPEND EXTRA_LCONVERT_LDFLAGS  '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   list(APPEND EXTRA_LCONVERT_LIBS
      uuid
   )
endif()

function_variable_fixup("${EXTRA_LCONVERT_CXXFLAGS}" EXTRA_LCONVERT_CXXFLAGS)
function_variable_fixup("${EXTRA_LCONVERT_LDFLAGS}"  EXTRA_LCONVERT_LDFLAGS)

target_link_libraries(lconvert${TOOLS_SUFFIX} ${EXTRA_LCONVERT_LIBS})

set_target_properties(lconvert${TOOLS_SUFFIX}
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_LCONVERT_CXXFLAGS}
   LINK_FLAGS ${EXTRA_LCONVERT_LDFLAGS}
   EXPORT_NAME lconvert
)

install(
   TARGETS lconvert${TOOLS_SUFFIX}
   EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   DESTINATION ${CMAKE_INSTALL_BINDIR}
)
