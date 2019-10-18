add_library(CsDeclarative SHARED "")
add_library(CopperSpice::CsDeclarative ALIAS CsDeclarative)

set_target_properties(CsDeclarative PROPERTIES OUTPUT_NAME CsDeclarative${BUILD_ABI})

target_add_definitions(CsDeclarative
   -DQT_BUILD_DECLARATIVE_LIB
)

set(EXTRA_DECLARATIVE_LIBS)
set(EXTRA_DECLARATIVE_CXXFLAGS)
set(EXTRA_DECLARATIVE_LDFLAGS)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private
    ${CMAKE_BINARY_DIR}/include/QtNetwork
    ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork
    ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork/private
    ${CMAKE_BINARY_DIR}/include/QtScript
    ${CMAKE_BINARY_DIR}/privateinclude/QtScript
    ${CMAKE_BINARY_DIR}/privateinclude/QtScript/private
    ${CMAKE_BINARY_DIR}/include/QtDeclarative
    ${CMAKE_BINARY_DIR}/privateinclude/QtDeclarative
    ${CMAKE_BINARY_DIR}/privateinclude/QtDeclarative/private
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/debugger
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsitems
    ${CMAKE_CURRENT_SOURCE_DIR}/qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/rewriter
    ${CMAKE_CURRENT_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src
)

include(debugger/debugger.cmake)
include(graphicsitems/graphicsitems.cmake)
include(qml/qml.cmake)
include(qml/parser/parser.cmake)
include(qml/rewriter/rewriter.cmake)
include(util/util.cmake)

macro_generate_public("${DECLARATIVE_PUBLIC_INCLUDES}" QtDeclarative)
macro_generate_private("${DECLARATIVE_PRIVATE_INCLUDES}" QtDeclarative)
macro_generate_misc("${DECLARATIVE_INCLUDES}" QtDeclarative)

set_target_properties(CsDeclarative
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_DECLARATIVE_CXXFLAGS}
   LINK_FLAGS ${EXTRA_DECLARATIVE_LDFLAGS}
)

target_link_libraries(CsDeclarative
   PRIVATE
   ${EXTRA_DECLARATIVE_LIBS}
)

function_generate_resources("${DECLARATIVE_SOURCES}")

macro_generate_package(
   CsDeclarative
   QtDeclarative
   "${EXTRA_DECLARATIVE_CXXFLAGS}"
   "CsCore CsGui CsNetwork CsScript"
)

install(TARGETS CsDeclarative
   EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/QtDeclarative
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
