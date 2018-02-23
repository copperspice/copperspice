add_definitions(
    -DQT_BUILD_XMLPATTERNS_LIB
)
set(EXTRA_XMLPATTERNS_LIBS CsCore${BUILD_ABI} CsNetwork${BUILD_ABI})
set(EXTRA_XMLPATTERNS_CXXFLAGS)
set(EXTRA_XMLPATTERNS_LDFLAGS)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/include/QtNetwork
    ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork
    ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork/private
    ${CMAKE_BINARY_DIR}/include/QtXmlPatterns
    ${CMAKE_BINARY_DIR}/privateinclude/QtXmlPatterns
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

add_library(CsXmlPatterns${BUILD_ABI} SHARED ${XMLPATTERNS_SOURCES} ${XMLPATTERNS_INCLUDES})
target_link_libraries(CsXmlPatterns${BUILD_ABI} ${EXTRA_XMLPATTERNS_LIBS})

target_include_directories(CsXmlPatterns${BUILD_ABI}
   PUBLIC
   $<INSTALL_INTERFACE:include/QtXmlPatterns>
)

set_target_properties(CsXmlPatterns${BUILD_ABI} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_XMLPATTERNS_CXXFLAGS}
    LINK_FLAGS ${EXTRA_XMLPATTERNS_LDFLAGS}
)

macro_generate_resources("${XMLPATTERNS_SOURCES}")
macro_generate_package(
    CsXmlPatterns${BUILD_ABI}
    QtXmlPatterns
    "${EXTRA_XMLPATTERNS_CXXFLAGS}"
    "CsCore${BUILD_ABI} CsNetwork${BUILD_ABI}"
)

install(
    TARGETS CsXmlPatterns${BUILD_ABI}
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
