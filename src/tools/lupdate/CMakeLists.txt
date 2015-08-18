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
    ${CMAKE_SOURCE_DIR}/src/tools/shared/numerus.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translator.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translatormessage.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/qm.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/qph.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/po.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ts.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/xliff.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/ioutils.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/shared/simtexth.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/lupdate/main.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/lupdate/merge.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/lupdate/cpp.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/lupdate/java.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/lupdate/qscript.cpp
    ${CMAKE_SOURCE_DIR}/src/tools/lupdate/ui.cpp

#   ${CMAKE_SOURCE_DIR}/src/tools/lupdate/qdeclarative.cpp
)

set(LUPDATE_INCLUDES
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translator.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/translatormessage.h
    ${CMAKE_SOURCE_DIR}/src/tools/shared/simtexth.h
    ${CMAKE_SOURCE_DIR}/src/tools/lupdate/lupdate.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(EXTRA_LUPDATE_LIBS
        ${EXTRA_LUPDATE_LIBS}
        uuid
    )
    # TODO: bin_SCRIPTS = src/tools/lupdate/lupdate.exe.manifest
endif()

set(EXTRA_DIST
    ${EXTRA_DIST}
    src/tools/lupdate/lupdate.exe.manifest
)

add_executable(lupdate ${LUPDATE_SOURCES} ${LUPDATE_INCLUDES})
target_link_libraries(lupdate ${EXTRA_LUPDATE_LIBS})
set_target_properties(lupdate PROPERTIES
    COMPILE_FLAGS ${EXTRA_LUPDATE_CXXFLAGS}
    LINK_FLAGS ${EXTRA_LUPDATE_LDFLAGS}
)

install(TARGETS lupdate RUNTIME DESTINATION bin)
