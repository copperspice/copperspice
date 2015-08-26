add_definitions(-DPROEVALUATOR_CUMULATIVE -DPROEVALUATOR_INIT_PROPS)
set(EXTRA_LCONVERT_LIBS CsCore${BUILD_MAJOR} CsXml${BUILD_MAJOR})
set(EXTRA_LCONVERT_CXXFLAGS " ") # whitespace is required!
set(EXTRA_LCONVERT_LDFLAGS "-Wl,--no-undefined")

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtXml
    ${CMAKE_BINARY_DIR}/privateinclude/QtXml
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/tools/shared
)

set(LCONVERT_SOURCES
    $<TARGET_OBJECTS:sharedtools>
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(EXTRA_LCONVERT_LIBS
        ${EXTRA_LCONVERT_LIBS}
        uuid
    )
endif()

add_executable(lconvert ${LCONVERT_SOURCES} $<TARGET_OBJECTS:sharedtools>)
target_link_libraries(lconvert ${EXTRA_LCONVERT_LIBS})
set_target_properties(lconvert PROPERTIES
    COMPILE_FLAGS ${EXTRA_LCONVERT_CXXFLAGS}
    LINK_FLAGS ${EXTRA_LCONVERT_LDFLAGS}
)

install(TARGETS lconvert RUNTIME DESTINATION bin)
