add_definitions(
    -DPROEVALUATOR_CUMULATIVE
    -DPROEVALUATOR_INIT_PROPS
)
set(EXTRA_LCONVERT_LIBS
    CsCore${BUILD_MAJOR}
    CsXml${BUILD_MAJOR}
    ${GLOBAL_LIBRARIES}
)
set(EXTRA_LCONVERT_CXXFLAGS ${GLOBAL_CXXFLAGS})
set(EXTRA_LCONVERT_LDFLAGS ${GLOBAL_LDFLAGS})

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

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(EXTRA_LCONVERT_LIBS
        ${EXTRA_LCONVERT_LIBS}
        uuid
    )
endif()

add_executable(lconvert${TOOLS_SUFFIX} ${LCONVERT_SOURCES} $<TARGET_OBJECTS:sharedtools>)
target_link_libraries(lconvert${TOOLS_SUFFIX} ${EXTRA_LCONVERT_LIBS})
set_target_properties(lconvert${TOOLS_SUFFIX} PROPERTIES
    COMPILE_FLAGS ${EXTRA_LCONVERT_CXXFLAGS}
    LINK_FLAGS ${EXTRA_LCONVERT_LDFLAGS}
    EXPORT_NAME lconvert
)

install(
    TARGETS lconvert${TOOLS_SUFFIX}
    EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
