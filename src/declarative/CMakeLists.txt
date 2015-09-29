add_definitions(
    -DQT_NO_OPENSSL
    -DQT_BUILD_DECLARATIVE_LIB
    -DQT_SHARED
    -DQT_MAKEDLL
)
set(EXTRA_DECLARATIVE_LIBS ${GLOBAL_LIBRARIES})
set(EXTRA_DECLARATIVE_CXXFLAGS ${GLOBAL_CXXFLAGS})
set(EXTRA_DECLARATIVE_LDFLAGS ${GLOBAL_LDFLAGS})

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
    ${CMAKE_BINARY_DIR}/include/QtNetwork
    ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork
    ${CMAKE_BINARY_DIR}/include/QtScript
    ${CMAKE_BINARY_DIR}/privateinclude/QtScript
    ${CMAKE_BINARY_DIR}/include/QtDeclarative
    ${CMAKE_BINARY_DIR}/privateinclude/QtDeclarative
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

add_library(CsDeclarative${BUILD_MAJOR} SHARED ${DECLARATIVE_SOURCES} ${DECLARATIVE_INCLUDES})
target_link_libraries(CsDeclarative${BUILD_MAJOR} ${EXTRA_DECLARATIVE_LIBS})
set_target_properties(CsDeclarative${BUILD_MAJOR} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_DECLARATIVE_CXXFLAGS}
    LINK_FLAGS ${EXTRA_DECLARATIVE_LDFLAGS}
)

macro_generate_resources("${DECLARATIVE_SOURCES}")
macro_generate_package(
    CsDeclarative${BUILD_MAJOR}
    QtDeclarative
    "${EXTRA_DECLARATIVE_CXXFLAGS}"
    "${EXTRA_DECLARATIVE_LIBS}"
    "CsCore${BUILD_MAJOR} QtGui${BUILD_MAJOR} CsNetwork${BUILD_MAJOR} CsScript${BUILD_MAJOR}"
)

install(
    TARGETS CsDeclarative${BUILD_MAJOR}
    EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/QtDeclarative
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
