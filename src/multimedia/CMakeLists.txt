add_definitions(
   -DQT_BUILD_MULTIMEDIA_LIB
)
set(EXTRA_MULTIMEDIA_LIBS CsCore${BUILD_ABI} CsGui${BUILD_ABI})
set(EXTRA_MULTIMEDIA_CXXFLAGS)
set(EXTRA_MULTIMEDIA_LDFLAGS)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private
    ${CMAKE_BINARY_DIR}/include/QtMultimedia
    ${CMAKE_BINARY_DIR}/privateinclude/QtMultimedia
    ${CMAKE_BINARY_DIR}/privateinclude/QtMultimedia/private
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

add_library(CsMultimedia${BUILD_ABI} SHARED ${MULTIMEDIA_SOURCES} ${MULTIMEDIA_INCLUDES})
target_link_libraries(CsMultimedia${BUILD_ABI} ${EXTRA_MULTIMEDIA_LIBS})

target_include_directories(CsMultimedia${BUILD_ABI}
   PUBLIC
   $<INSTALL_INTERFACE:include/QtMultimedia>
)

set_target_properties(CsMultimedia${BUILD_ABI} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_MULTIMEDIA_CXXFLAGS}
    LINK_FLAGS ${EXTRA_MULTIMEDIA_LDFLAGS}
)

macro_generate_resources("${MULTIMEDIA_SOURCES}")
macro_generate_package(
    CsMultimedia${BUILD_ABI}
    QtMultimedia
    "${EXTRA_MULTIMEDIA_CXXFLAGS}"
    "CsCore${BUILD_ABI} CsGui${BUILD_ABI}"
)

install(
    TARGETS CsMultimedia${BUILD_ABI}
    EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/QtMultimedia
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
