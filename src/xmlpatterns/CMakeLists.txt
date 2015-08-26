add_definitions(-DQT_BUILD_XMLPATTERNS_LIB -DQT_SHARED -DQT_MAKEDLL)
# BROOM (enable OpenSSL)
add_definitions(-DQT_NO_OPENSSL)
set(EXTRA_XMLPATTERNS_LIBS CsCore${BUILD_MAJOR} CsNetwork${BUILD_MAJOR})
set(EXTRA_XMLPATTERNS_CXXFLAGS " ") # whitespace is required!
set(EXTRA_XMLPATTERNS_LDFLAGS "-Wl,--no-undefined")

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

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtNetwork
    ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork
    ${CMAKE_BINARY_DIR}/include/QtXmlPatterns
    ${CMAKE_BINARY_DIR}/privateinclude/QtXmlPatterns
    ${CMAKE_CURRENT_SOURCE_DIR}
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

add_library(CsXmlPatterns${BUILD_MAJOR} SHARED ${XMLPATTERNS_SOURCES} ${XMLPATTERNS_INCLUDES})
target_link_libraries(CsXmlPatterns${BUILD_MAJOR} ${EXTRA_XMLPATTERNS_LIBS})
set_target_properties(CsXmlPatterns${BUILD_MAJOR} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_XMLPATTERNS_CXXFLAGS}
    LINK_FLAGS ${EXTRA_XMLPATTERNS_LDFLAGS}
)

macro_generate_resources("${XMLPATTERNS_SOURCES}")
macro_generate_package(
    CsXmlPatterns${BUILD_MAJOR}
    "${EXTRA_XMLPATTERNS_CXXFLAGS}"
    "${EXTRA_XMLPATTERNS_LIBS}"
    "CsCore${BUILD_MAJOR} CsNetwork${BUILD_MAJOR}"
)

install(TARGETS CsXmlPatterns${BUILD_MAJOR} LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/QtXmlPatterns DESTINATION include)
