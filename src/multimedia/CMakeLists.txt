add_library(CsMultimedia SHARED "")
add_library(CopperSpice::CsMultimedia ALIAS CsMultimedia)

set_target_properties(CsMultimedia PROPERTIES OUTPUT_NAME CsMultimedia${BUILD_ABI})

target_compile_definitions(CsMultimedia
   PRIVATE
   -DQT_BUILD_MULTIMEDIA_LIB
)

set(EXTRA_MULTIMEDIA_CXXFLAGS)
set(EXTRA_MULTIMEDIA_LDFLAGS)

target_include_directories(CsMultimedia
   PUBLIC
   $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include/QtMultimedia>
   $<INSTALL_INTERFACE:include/QtMultimedia>
)

include_directories(
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/privateinclude/QtMultimedia/private

    ${CMAKE_CURRENT_SOURCE_DIR}/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/camera
    ${CMAKE_CURRENT_SOURCE_DIR}/controls
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/playback
    ${CMAKE_CURRENT_SOURCE_DIR}/radio
    ${CMAKE_CURRENT_SOURCE_DIR}/recording
    ${CMAKE_CURRENT_SOURCE_DIR}/video
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)

include(audio/audio.cmake)
include(camera/camera.cmake)
include(controls/controls.cmake)
include(kernel/kernel.cmake)
include(playback/playback.cmake)
include(radio/radio.cmake)
include(recording/recording.cmake)
include(video/video.cmake)
include(widgets/widgets.cmake)

include(../plugins/multimedia/mediaservices/avfoundation/avfoundation.cmake)
include(../plugins/multimedia/mediaservices/directshow/directshow.cmake)
include(../plugins/multimedia/mediaservices/gstreamer/gstreamer.cmake)
include(../plugins/multimedia/playlistformats/playlistformats.cmake)

macro_generate_public("${MULTIMEDIA_PUBLIC_INCLUDES}" QtMultimedia)
macro_generate_private("${MULTIMEDIA_PRIVATE_INCLUDES}" QtMultimedia)
macro_generate_misc("${MULTIMEDIA_INCLUDES}" QtMultimedia)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   list(APPEND EXTRA_MULTIMEDIA_LDFLAGS
      -framework OpenGL
   )
endif()

function_variable_fixup("${EXTRA_MULTIMEDIA_CXXFLAGS}" EXTRA_MULTIMEDIA_CXXFLAGS)
function_variable_fixup("${EXTRA_MULTIMEDIA_LDFLAGS}"  EXTRA_MULTIMEDIA_LDFLAGS)

target_sources(CsMultimedia
   PRIVATE
   ${MULTIMEDIA_INCLUDES}
)

target_link_libraries(CsMultimedia
   PUBLIC
   CsCore
   CsGui
   CsNetwork
   CsOpenGL
)

target_include_directories(CsMultimedia
   PUBLIC
   ${GSTREAMER_INCLUDE_DIRS}
)

set_target_properties(CsMultimedia
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_MULTIMEDIA_CXXFLAGS}
   LINK_FLAGS ${EXTRA_MULTIMEDIA_LDFLAGS}
)

function_generate_resources(CsMultimedia)

macro_generate_package(
   CsMultimedia
   QtMultimedia
   "${EXTRA_MULTIMEDIA_CXXFLAGS}"
   "CsCore CsGui"
)

install(
    TARGETS CsMultimedia
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
