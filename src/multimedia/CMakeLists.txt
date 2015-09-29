add_definitions(
    -DQT_BUILD_MULTIMEDIA_LIB
    -DQT_MAKEDLL
)
set(EXTRA_MULTIMEDIA_LIBS
    CsCore${BUILD_MAJOR}
    CsGui${BUILD_MAJOR}
    ${GLOBAL_LIBRARIES}
)
set(EXTRA_MULTIMEDIA_CXXFLAGS ${GLOBAL_CXXFLAGS})
set(EXTRA_MULTIMEDIA_LDFLAGS ${GLOBAL_LDFLAG})

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
    ${CMAKE_BINARY_DIR}/include/QtMultimedia
    ${CMAKE_BINARY_DIR}/privateinclude/QtMultimedia
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/video
)

include(audio/audio.cmake)
include(video/video.cmake)

macro_generate_public("${MULTIMEDIA_PUBLIC_INCLUDES}" QtMultimedia)
macro_generate_private("${MULTIMEDIA_PRIVATE_INCLUDES}" QtMultimedia)
macro_generate_misc("${MULTIMEDIA_INCLUDES}" QtMultimedia)

function_variable_fixup("${EXTRA_MULTIMEDIA_CXXFLAGS}" EXTRA_MULTIMEDIA_CXXFLAGS)
function_variable_fixup("${EXTRA_MULTIMEDIA_LDFLAGS}" EXTRA_MULTIMEDIA_LDFLAGS)

add_library(CsMultimedia${BUILD_MAJOR} SHARED ${MULTIMEDIA_SOURCES} ${MULTIMEDIA_INCLUDES})
target_link_libraries(CsMultimedia${BUILD_MAJOR} ${EXTRA_MULTIMEDIA_LIBS})
set_target_properties(CsMultimedia${BUILD_MAJOR} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_MULTIMEDIA_CXXFLAGS}
    LINK_FLAGS ${EXTRA_MULTIMEDIA_LDFLAGS}
)

macro_generate_resources("${MULTIMEDIA_SOURCES}")
macro_generate_package(
    CsMultimedia${BUILD_MAJOR}
    QtMultimedia
    "${EXTRA_MULTIMEDIA_CXXFLAGS}"
    "${EXTRA_MULTIMEDIA_LIBS}"
    "CsCore${BUILD_MAJOR} CsGui${BUILD_MAJOR}"
)


install(
    TARGETS CsMultimedia${BUILD_MAJOR}
    EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/QtMultimedia
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
