# add_definitions()
set(EXTRA_LUPDATE_LIBS CsCore${BUILD_ABI} CsXml${BUILD_ABI})
set(EXTRA_LUPDATE_CXXFLAGS)
set(EXTRA_LUPDATE_LDFLAGS)

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

set(LUPDATE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ioutils.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/simtexth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/merge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/java.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qscript.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui.cpp

#   ${CMAKE_CURRENT_SOURCE_DIR}/qdeclarative.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   set(EXTRA_LUPDATE_LDFLAGS  ${EXTRA_LUPDATE_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "DARWIN")
   set(EXTRA_LUPDATE_LDFLAGS  ${EXTRA_LUPDATE_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(EXTRA_LUPDATE_LDFLAGS  ${EXTRA_LUPDATE_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(EXTRA_LUPDATE_LIBS ${EXTRA_LUPDATE_LIBS} uuid)

    set(LUPDATE_SOURCES
        ${LUPDATE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/lupdate.exe.manifest
    )
endif()

function_variable_fixup("${EXTRA_LUPDATE_CXXFLAGS}" EXTRA_LUPDATE_CXXFLAGS)
function_variable_fixup("${EXTRA_LUPDATE_LDFLAGS}" EXTRA_LUPDATE_LDFLAGS)

add_executable(lupdate${TOOLS_SUFFIX} ${LUPDATE_SOURCES} $<TARGET_OBJECTS:sharedtools> ${lupdate_manifest})
target_link_libraries(lupdate${TOOLS_SUFFIX} ${EXTRA_LUPDATE_LIBS})

set_target_properties(lupdate${TOOLS_SUFFIX} PROPERTIES
    COMPILE_FLAGS ${EXTRA_LUPDATE_CXXFLAGS}
    LINK_FLAGS ${EXTRA_LUPDATE_LDFLAGS}
    EXPORT_NAME lupdate
)

macro_generate_resources("${LUPDATE_SOURCES}")

install(
    TARGETS lupdate${TOOLS_SUFFIX}
    EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(CopperSpice::lupdate ALIAS lupdate${TOOLS_SUFFIX})