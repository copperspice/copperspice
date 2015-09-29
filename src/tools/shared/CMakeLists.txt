set(SHAREDTOOLS_DEFINITIONS
    ${SHAREDTOOLS_DEFINITIONS}
)
set(SHAREDTOOLS_EXTRA_CXXFLAGS ${GLOBAL_CXXFLAGS})
set(SHAREDTOOLS_EXTRA_LDFLAGS ${GLOBAL_LDFLAGS})

set(SHAREDTOOLS_INCLUDES
    ${SHAREDTOOLS_INCLUDES}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtXml
    ${CMAKE_BINARY_DIR}/privateinclude/QtXml
    ${CMAKE_SOURCE_DIR}/src/tools/shared
)

set(SHAREDTOOLS_SOURCES
    ${SHAREDTOOLS_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/tools/shared/numerus.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translator.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translatormessage.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/qm.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/qph.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/po.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ts.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/xliff.cpp
)

set(SHAREDTOOLS_HEADERS
    ${SHAREDTOOLS_HEADERS}
    ${CMAKE_SOURCE_DIR}/src/tools/shared/numerus.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translator.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translatormessage.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/qm.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/qph.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/po.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ts.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/xliff.h
)

add_library(sharedtools OBJECT ${SHAREDTOOLS_SOURCES})
target_compile_definitions(sharedtools PRIVATE ${SHAREDTOOLS_DEFINITIONS})
target_include_directories(sharedtools PRIVATE ${SHAREDTOOLS_INCLUDES})
set_target_properties(sharedtools PROPERTIES
    COMPILE_FLAGS ${SHAREDTOOLS_EXTRA_CXXFLAGS}
    LINK_FLAGS ${SHAREDTOOLS_EXTRA_LDFLAGS}
)

macro_generate_resources("${SHAREDTOOLS_SOURCES}")
macro_generate_resources("${SHAREDTOOLS_HEADERS}")
