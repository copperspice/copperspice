# add_definitions()
set(EXTRA_LRELEASE_LIBS CsCore${BUILD_MAJOR} CsXml${BUILD_MAJOR})
set(EXTRA_LRELEASE_CXXFLAGS " ") # whitespace is required!
set(EXTRA_LRELEASE_LDFLAGS "-Wl,--no-undefined")

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

set(LRELEASE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ioutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(EXTRA_LRELEASE_LIBS
        ${EXTRA_LRELEASE_LIBS}
        uuid
    )
endif()

add_executable(lrelease ${LRELEASE_SOURCES} $<TARGET_OBJECTS:sharedtools>)
target_link_libraries(lrelease ${EXTRA_LRELEASE_LIBS})
set_target_properties(lrelease PROPERTIES
    COMPILE_FLAGS ${EXTRA_LRELEASE_CXXFLAGS}
    LINK_FLAGS ${EXTRA_LRELEASE_LDFLAGS}
)

install(TARGETS lrelease DESTINATION ${CMAKE_INSTALL_BINDIR})
