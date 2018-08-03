add_definitions(-UQT_BUILD_CORE_LIB)
set(EXTRA_RCC_LIBS
   CsCore${BUILD_ABI}
   CsXml${BUILD_ABI}
   ${CMAKE_THREAD_LIBS_INIT}
)
set(EXTRA_RCC_CXXFLAGS)
set(EXTRA_RCC_LDFLAGS)

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
)

set(RCC_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/rcc.cpp
)

set(RCC_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/rcc.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   set(EXTRA_RCC_LDFLAGS  ${EXTRA_RCC_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "DARWIN")
   set(EXTRA_RCC_LDFLAGS  ${EXTRA_RCC_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(EXTRA_RCC_LDFLAGS  ${EXTRA_RCC_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   set(EXTRA_RCC_LIBS ${EXTRA_RCC_LIBS} uuid)
endif()

function_variable_fixup("${EXTRA_RCC_CXXFLAGS}" EXTRA_RCC_CXXFLAGS)
function_variable_fixup("${EXTRA_RCC_LDFLAGS}"  EXTRA_RCC_LDFLAGS)

add_executable(rcc${TOOLS_SUFFIX} ${RCC_SOURCES} ${RCC_INCLUDES})
target_link_libraries(rcc${TOOLS_SUFFIX} ${EXTRA_RCC_LIBS})

set_target_properties(rcc${TOOLS_SUFFIX} PROPERTIES
    COMPILE_FLAGS ${EXTRA_RCC_CXXFLAGS}
    LINK_FLAGS ${EXTRA_RCC_LDFLAGS}
    EXPORT_NAME rcc
)

install(
    TARGETS rcc${TOOLS_SUFFIX}
    EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(CopperSpice::rcc ALIAS rcc${TOOLS_SUFFIX})