add_definitions(-DQT_BUILD_XML_LIB -DQT_SHARED -DQT_MAKEDLL)
set(EXTRA_XML_LIBS "")
set(EXTRA_XML_CXXFLAGS "-Wno-undefined")
set(EXTRA_XML_LDFLAGS " ") # whitespace is required!

include(dom/dom.cmake)
include(sax/sax.cmake)
include(stream/stream.cmake)

macro_generate_public("${XML_PUBLIC_INCLUDES}" QtXml)
macro_generate_private("${XML_PRIVATE_INCLUDES}" QtXml)
macro_generate_misc("${XML_INCLUDES}" QtXml)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtXml
    ${CMAKE_BINARY_DIR}/privateinclude/QtXml
    ${CMAKE_CURRENT_SOURCE_DIR}/dom
    ${CMAKE_CURRENT_SOURCE_DIR}/sax
    ${CMAKE_CURRENT_SOURCE_DIR}/stream
    ${GLIB2_INCLUDE_DIR}
)

add_library(CsXml${BUILD_MAJOR} SHARED ${XML_SOURCES} ${XML_INCLUDES})
target_link_libraries(CsXml${BUILD_MAJOR} ${EXTRA_XML_LIBS})
set_target_properties(CsXml${BUILD_MAJOR} PROPERTIES
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_XML_CXXFLAGS}
    LINK_FLAGS ${EXTRA_XML_LDFLAGS}
)

macro_generate_resources("${XML_SOURCES}")

install(TARGETS CsXml${BUILD_MAJOR} LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/QtXml DESTINATION include)
