# add_definitions()
set(EXTRA_LUPDATE_LIBS CsCore${BUILD_MAJOR} CsXml${BUILD_MAJOR})
set(EXTRA_LUPDATE_CXXFLAGS " ") # whitespace is required!
set(EXTRA_LUPDATE_LDFLAGS "-Wl,--no-undefined")

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

set(LUPDATE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ioutils.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/simtexth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/merge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/java.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qscript.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui.cpp

#   ${CMAKE_CURRENT_SOURCE_DIR}/qdeclarative.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(EXTRA_LUPDATE_LIBS
        ${EXTRA_LUPDATE_LIBS}
        uuid
    )
    set(LUPDATE_SOURCES
        ${LUPDATE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/lupdate.exe.manifest
    )
endif()

add_executable(lupdate ${LUPDATE_SOURCES} $<TARGET_OBJECTS:sharedtools>)
target_link_libraries(lupdate ${EXTRA_LUPDATE_LIBS})
set_target_properties(lupdate PROPERTIES
    COMPILE_FLAGS ${EXTRA_LUPDATE_CXXFLAGS}
    LINK_FLAGS ${EXTRA_LUPDATE_LDFLAGS}
)

macro_generate_resources("${LUPDATE_SOURCES}")

install(TARGETS lupdate DESTINATION ${CMAKE_INSTALL_BINDIR})
