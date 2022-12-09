add_library(CsXmlPatterns SHARED "")
add_library(CopperSpice::CsXmlPatterns ALIAS CsXmlPatterns)

set_target_properties(CsXmlPatterns PROPERTIES OUTPUT_NAME CsXmlPatterns${BUILD_ABI})

add_definitions(
   -DQT_BUILD_XMLPATTERNS_LIB
)

set(EXTRA_XMLPATTERNS_CXXFLAGS)
set(EXTRA_XMLPATTERNS_LDFLAGS)

target_include_directories(CsXmlPatterns
   PUBLIC
   $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include/QtXmlPatterns>
   $<INSTALL_INTERFACE:include/QtXmlPatterns>
)

include_directories(
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/privateinclude/QtXmlPatterns/private

    ${CMAKE_CURRENT_SOURCE_DIR}/acceltree
    ${CMAKE_CURRENT_SOURCE_DIR}/api
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/environment
    ${CMAKE_CURRENT_SOURCE_DIR}/expr
    ${CMAKE_CURRENT_SOURCE_DIR}/functions
    ${CMAKE_CURRENT_SOURCE_DIR}/iterators
    ${CMAKE_CURRENT_SOURCE_DIR}/janitors
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/projection
    ${CMAKE_CURRENT_SOURCE_DIR}/qtokenautomaton
    ${CMAKE_CURRENT_SOURCE_DIR}/schema
    ${CMAKE_CURRENT_SOURCE_DIR}/type
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

include(acceltree/acceltree.cmake)
include(api/api.cmake)
include(data/data.cmake)
include(environment/environment.cmake)
include(expr/expr.cmake)
include(functions/functions.cmake)
include(iterators/iterators.cmake)
include(janitors/janitors.cmake)
include(parser/parser.cmake)
include(projection/projection.cmake)
include(schema/schema.cmake)
include(type/type.cmake)
include(utils/utils.cmake)

macro_generate_public("${XMLPATTERNS_PUBLIC_INCLUDES}" QtXmlPatterns)
macro_generate_private("${XMLPATTERNS_PRIVATE_INCLUDES}" QtXmlPatterns)
macro_generate_misc("${XMLPATTERNS_INCLUDES}" QtXmlPatterns)

function_variable_fixup("${EXTRA_XMLPATTERNS_CXXFLAGS}" EXTRA_XMLPATTERNS_CXXFLAGS)
function_variable_fixup("${EXTRA_XMLPATTERNS_LDFLAGS}" EXTRA_XMLPATTERNS_LDFLAGS)

target_sources(CsXmlPatterns
   PRIVATE
   ${XMLPATTERNS_INCLUDES}
)

target_link_libraries(CsXmlPatterns
   PRIVATE
   CsCore
   CsNetwork
)

set_target_properties(CsXmlPatterns
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_XMLPATTERNS_CXXFLAGS}
   LINK_FLAGS ${EXTRA_XMLPATTERNS_LDFLAGS}
)

function_generate_resources(CsXmlPatterns)

macro_generate_package(
   CsXmlPatterns
   QtXmlPatterns
   "${EXTRA_XMLPATTERNS_CXXFLAGS}"
   "CsCore CsNetwork"
)

install(
    TARGETS CsXmlPatterns
    EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/QtXmlPatterns
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
