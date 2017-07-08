add_definitions(
    -DQT_BUILD_SCRIPTTOOLS_LIB
)
set(EXTRA_SCRIPTTOOLS_LIBS)
set(EXTRA_SCRIPTTOOLS_CXXFLAGS)
set(EXTRA_SCRIPTTOOLS_LDFLAGS)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private
    ${CMAKE_BINARY_DIR}/include/QtScriptTools
    ${CMAKE_BINARY_DIR}/privateinclude/QtScriptTools
    ${CMAKE_BINARY_DIR}/privateinclude/QtScriptTools/private
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/debugging
)

include(debugging/debugging.cmake)

macro_generate_public("${SCRIPTTOOLS_PUBLIC_INCLUDES}" QtScriptTools)
macro_generate_private("${SCRIPTTOOLS_PRIVATE_INCLUDES}" QtScriptTools)
macro_generate_misc("${SCRIPTTOOLS_INCLUDES}" QtScriptTools)

function_variable_fixup("${EXTRA_SCRIPTTOOLS_CXXFLAGS}" EXTRA_SCRIPTTOOLS_CXXFLAGS)
function_variable_fixup("${EXTRA_SCRIPTTOOLS_LDFLAGS}" EXTRA_SCRIPTTOOLS_LDFLAGS)

add_library(CsScriptTools${BUILD_ABI} SHARED ${SCRIPTTOOLS_SOURCES} ${SCRIPTTOOLS_INCLUDES})
target_link_libraries(CsScriptTools${BUILD_ABI} ${EXTRA_SCRIPTTOOLS_LIBS})
set_target_properties(CsScriptTools${BUILD_ABI} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_SCRIPTTOOLS_CXXFLAGS}
    LINK_FLAGS ${EXTRA_SCRIPTTOOLS_LDFLAGS}
)

macro_generate_resources("${SCRIPTTOOLS_SOURCES}")
macro_generate_package(
    CsScriptTools${BUILD_ABI}
    QtScriptTools
    "${EXTRA_SCRIPTTOOLS_CXXFLAGS}"
    "CsCore${BUILD_ABI} CsGui${BUILD_ABI}"
)

install(
    TARGETS CsScriptTools${BUILD_ABI}
    EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/QtScriptTools
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
