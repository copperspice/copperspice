add_definitions(-DQT_BUILD_MULTIMEDIA_LIB
    -DQT_MAKEDLL
)

set(EXTRA_MULTIMEDIA_LIBS CsCore${BUILD_MAJOR} CsGui${BUILD_MAJOR})
set(EXTRA_MULTIMEDIA_CXXFLAGS "-Wno-undefined")
set(EXTRA_MULTIMEDIA_LDFLAGS " ") # whitespace is required!

add_subdirectory(audio)
add_subdirectory(video)

# TODO: make them macros to be shared between components
foreach(pubheader ${MULTIMEDIA_PUBLIC_INCLUDES})
    string(TOLOWER ${pubheader} pubname)
    get_filename_component(pubname ${pubname} NAME)
    set(pubout ${CMAKE_BINARY_DIR}/include/QtMultimedia/${pubheader})
    message(STATUS "Writing public: ${pubout}")
    file(WRITE ${pubout} "#include <${pubname}.h>")
endforeach(pubheader)

foreach(privheader ${MULTIMEDIA_PRIVATE_INCLUDES})
    get_filename_component(privname ${privheader} NAME)
    set(privout ${CMAKE_BINARY_DIR}/privateinclude/QtMultimedia/${privname})
    message(STATUS "Writing private: ${privout}")
    file(COPY ${privheader} DESTINATION ${CMAKE_BINARY_DIR}/privateinclude/QtMultimedia/)
endforeach(privheader)

foreach(header ${MULTIMEDIA_INCLUDES})
    get_filename_component(headername ${header} NAME)
    set(headout ${CMAKE_BINARY_DIR}/include/QtMultimedia/${headername})
    message(STATUS "Writing: ${headout}")
    file(COPY ${header} DESTINATION ${CMAKE_BINARY_DIR}/include/QtMultimedia/)
endforeach(header)

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

install(TARGETS CsMultimedia${BUILD_MAJOR} LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/QtMultimedia DESTINATION include)