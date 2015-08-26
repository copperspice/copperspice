add_definitions(-DQT_BUILD_SCRIPTTOOLS_LIB -DQT_SHARED -DQT_MAKEDLL)
set(EXTRA_SCRIPTTOOLS_LIBS "")
set(EXTRA_SCRIPTTOOLS_CXXFLAGS " ") # whitespace is required!
set(EXTRA_SCRIPTTOOLS_LDFLAGS "-Wl,--no-undefined")

include(debugging/debugging.cmake)

macro_generate_public("${SCRIPTTOOLS_PUBLIC_INCLUDES}" QtScriptTools)
macro_generate_private("${SCRIPTTOOLS_PRIVATE_INCLUDES}" QtScriptTools)
macro_generate_misc("${SCRIPTTOOLS_INCLUDES}" QtScriptTools)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
    ${CMAKE_BINARY_DIR}/include/QtScriptTools
    ${CMAKE_BINARY_DIR}/privateinclude/QtScriptTools
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/debugging
)

add_library(CsScriptTools${BUILD_MAJOR} SHARED ${SCRIPTTOOLS_SOURCES} ${SCRIPTTOOLS_INCLUDES})
target_link_libraries(CsScriptTools${BUILD_MAJOR} ${EXTRA_SCRIPTTOOLS_LIBS})
set_target_properties(CsScriptTools${BUILD_MAJOR} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_SCRIPTTOOLS_CXXFLAGS}
    LINK_FLAGS ${EXTRA_SCRIPTTOOLS_LDFLAGS}
)

macro_generate_resources("${SCRIPTTOOLS_SOURCES}")
macro_generate_package(
    CsScriptTools${BUILD_MAJOR}
    "${EXTRA_SCRIPTTOOLS_CXXFLAGS}"
    "${EXTRA_SCRIPTTOOLS_LIBS}"
    "CsCore${BUILD_MAJOR} CsGui${BUILD_MAJOR}"
)

install(TARGETS CsScriptTools${BUILD_MAJOR} LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/QtScriptTools DESTINATION include)
