add_definitions(
    -DQT_BUILD_XML_LIB
    -DQT_SHARED
    -DQT_MAKEDLL
)
set(EXTRA_XML_LIBS CsCore${BUILD_ABI})
set(EXTRA_XML_CXXFLAGS)
set(EXTRA_XML_LDFLAGS)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/include/QtXml
    ${CMAKE_BINARY_DIR}/privateinclude/QtXml
    ${CMAKE_BINARY_DIR}/privateinclude/QtXml/private
    ${CMAKE_CURRENT_SOURCE_DIR}/dom
    ${CMAKE_CURRENT_SOURCE_DIR}/sax
    ${CMAKE_CURRENT_SOURCE_DIR}/stream
    ${GLIB2_INCLUDE_DIR}
)

include(dom/dom.cmake)
include(sax/sax.cmake)
include(stream/stream.cmake)

macro_generate_public("${XML_PUBLIC_INCLUDES}" QtXml)
macro_generate_private("${XML_PRIVATE_INCLUDES}" QtXml)
macro_generate_misc("${XML_INCLUDES}" QtXml)

function_variable_fixup("${EXTRA_XML_CXXFLAGS}" EXTRA_XML_CXXFLAGS)
function_variable_fixup("${EXTRA_XML_LDFLAGS}" EXTRA_XML_LDFLAGS)

add_library(CsXml${BUILD_ABI} SHARED ${XML_SOURCES} ${XML_INCLUDES})
target_link_libraries(CsXml${BUILD_ABI} ${EXTRA_XML_LIBS})
set_target_properties(CsXml${BUILD_ABI} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_XML_CXXFLAGS}
    LINK_FLAGS ${EXTRA_XML_LDFLAGS}
)

macro_generate_resources("${XML_SOURCES}")
macro_generate_package(
    CsXml${BUILD_ABI}
    QtXml
    "${EXTRA_XML_CXXFLAGS}"
    "${EXTRA_XML_LIBS}"
    "CsCore${BUILD_ABI}"
)

install(
    TARGETS CsXml${BUILD_ABI}
    EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/QtXml
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
