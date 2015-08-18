add_definitions(-DQT_BUILD_MULTIMEDIA_LIB -DQT_MAKEDLL)
set(EXTRA_MULTIMEDIA_LIBS CsCore${BUILD_MAJOR} CsGui${BUILD_MAJOR})
set(EXTRA_MULTIMEDIA_CXXFLAGS " ") # whitespace is required!
set(EXTRA_MULTIMEDIA_LDFLAGS "-Wl,--no-undefined")

include(audio/audio.cmake)
include(video/video.cmake)

macro_generate_public("${MULTIMEDIA_PUBLIC_INCLUDES}" QtMultimedia)
macro_generate_private("${MULTIMEDIA_PRIVATE_INCLUDES}" QtMultimedia)
macro_generate_misc("${MULTIMEDIA_INCLUDES}" QtMultimedia)

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

add_library(CsMultimedia${BUILD_MAJOR} SHARED ${MULTIMEDIA_SOURCES} ${MULTIMEDIA_INCLUDES})
target_link_libraries(CsMultimedia${BUILD_MAJOR} ${EXTRA_MULTIMEDIA_LIBS})
set_target_properties(CsMultimedia${BUILD_MAJOR} PROPERTIES
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_MULTIMEDIA_CXXFLAGS}
    LINK_FLAGS ${EXTRA_MULTIMEDIA_LDFLAGS}
)

macro_generate_resources("${MULTIMEDIA_SOURCES}")

install(TARGETS CsMultimedia${BUILD_MAJOR} LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/QtMultimedia DESTINATION include)