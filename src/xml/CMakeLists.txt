add_library(CsXml SHARED "")
add_library(CopperSpice::CsXml ALIAS CsXml)

set_target_properties(CsXml PROPERTIES OUTPUT_NAME CsXml${BUILD_ABI})

add_definitions(
   -DQT_BUILD_XML_LIB
)

set(EXTRA_XML_CXXFLAGS)
set(EXTRA_XML_LDFLAGS)

target_include_directories(CsXml
   PUBLIC
   $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include/QtXml>
   $<INSTALL_INTERFACE:include/QtXml>
)

include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtXml/private

   ${CMAKE_CURRENT_SOURCE_DIR}/dom
   ${CMAKE_CURRENT_SOURCE_DIR}/sax
   ${CMAKE_CURRENT_SOURCE_DIR}/stream
   ${GLIB2_INCLUDES}
)

include(dom/dom.cmake)
include(sax/sax.cmake)
include(stream/stream.cmake)

macro_generate_public("${XML_PUBLIC_INCLUDES}"   QtXml)
macro_generate_private("${XML_PRIVATE_INCLUDES}" QtXml)
macro_generate_misc("${XML_INCLUDES}" QtXml)

function_variable_fixup("${EXTRA_XML_CXXFLAGS}" EXTRA_XML_CXXFLAGS)
function_variable_fixup("${EXTRA_XML_LDFLAGS}"  EXTRA_XML_LDFLAGS)

target_sources(CsXml
   PRIVATE
   ${XML_INCLUDES}
)

target_link_libraries(CsXml
   PUBLIC
   CsCore
)

set_target_properties(CsXml
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_XML_CXXFLAGS}
   LINK_FLAGS ${EXTRA_XML_LDFLAGS}
)

function_generate_resources(CsXml)

macro_generate_package(
   CsXml
   QtXml
   "${EXTRA_XML_CXXFLAGS}"
   "CsCore"
)

install(
   TARGETS CsXml
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
