# add_definitions()
set(EXTRA_LRELEASE_LIBS CsCore${BUILD_MAJOR} CsXml${BUILD_MAJOR})
set(EXTRA_LRELEASE_CXXFLAGS "-Wno-undefined")
set(EXTRA_LRELEASE_LDFLAGS " ") # whitespace is required!

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
    ${CMAKE_SOURCE_DIR}/src/tools/shared/numerus.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translator.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translatormessage.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/qm.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/qph.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/po.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ts.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/xliff.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ioutils.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/lrelease/main.cpp
)

set(LRELEASE_INCLUDES
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translator.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translatormessage.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ioutils.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(EXTRA_LRELEASE_LIBS
        ${EXTRA_LRELEASE_LIBS}
        uuid
    )
endif()

add_executable(lrelease ${LRELEASE_SOURCES} ${LRELEASE_INCLUDES})
target_link_libraries(lrelease ${EXTRA_LRELEASE_LIBS})
set_target_properties(lrelease PROPERTIES
    COMPILE_FLAGS ${EXTRA_LRELEASE_CXXFLAGS}
    LINK_FLAGS ${EXTRA_LRELEASE_LDFLAGS}
)

install(TARGETS lrelease RUNTIME DESTINATION bin)
