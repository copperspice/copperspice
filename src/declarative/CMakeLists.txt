add_definitions(-DQT_NO_OPENSSL -DQT_BUILD_DECLARATIVE_LIB -DQT_SHARED -DQT_MAKEDLL)
set(EXTRA_DECLARATIVE_LIBS "")
set(EXTRA_DECLARATIVE_CXXFLAGS "-Wno-undefined")
set(EXTRA_DECLARATIVE_LDFLAGS " ") # whitespace is required!

include(debugger/debugger.cmake)
include(graphicsitems/graphicsitems.cmake)
include(qml/qml.cmake)
include(qml/parser/parser.cmake)
include(qml/rewriter/rewriter.cmake)
include(util/util.cmake)

macro_generate_public("${DECLARATIVE_PUBLIC_INCLUDES}" QtDeclarative)
macro_generate_private("${DECLARATIVE_PRIVATE_INCLUDES}" QtDeclarative)
macro_generate_misc("${DECLARATIVE_INCLUDES}" QtDeclarative)

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

add_library(CsDeclarative${BUILD_MAJOR} SHARED ${DECLARATIVE_SOURCES} ${DECLARATIVE_INCLUDES})
target_link_libraries(CsDeclarative${BUILD_MAJOR} ${EXTRA_DECLARATIVE_LIBS})
set_target_properties(CsDeclarative${BUILD_MAJOR} PROPERTIES
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_DECLARATIVE_CXXFLAGS}
    LINK_FLAGS ${EXTRA_DECLARATIVE_LDFLAGS}
)

macro_generate_resources("${DECLARATIVE_SOURCES}")

install(TARGETS CsDeclarative${BUILD_MAJOR} LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/QtDeclarative DESTINATION include)
