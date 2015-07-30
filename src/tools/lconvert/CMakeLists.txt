add_definitions(-DPROEVALUATOR_CUMULATIVE -DPROEVALUATOR_INIT_PROPS)
set(EXTRA_LCONVERT_LIBS CsCore${BUILD_MAJOR} CsXml${BUILD_MAJOR})
set(EXTRA_LCONVERT_CXXFLAGS "-Wno-undefined")
set(EXTRA_LCONVERT_LDFLAGS " ") # whitespace is required!

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
    ${CMAKE_SOURCE_DIR}/src/tools/shared/numerus.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translator.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translatormessage.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/qm.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/qph.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/po.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ts.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/xliff.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/lconvert/main.cpp
)

set(LCONVERT_INCLUDES
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translator.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translatormessage.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(EXTRA_LCONVERT_LIBS
        ${EXTRA_LCONVERT_LIBS}
        uuid
    )
endif()

add_executable(lconvert ${LCONVERT_SOURCES} ${LCONVERT_INCLUDES})
target_link_libraries(lconvert ${EXTRA_LCONVERT_LIBS})
set_target_properties(lconvert PROPERTIES
    COMPILE_FLAGS ${EXTRA_LCONVERT_CXXFLAGS}
    LINK_FLAGS ${EXTRA_LCONVERT_LDFLAGS}
)

install(TARGETS lconvert RUNTIME DESTINATION bin)
