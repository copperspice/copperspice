add_definitions(-UQT_BUILD_CORE_LIB)
set(EXTRA_RCC_LIBS CsCore${BUILD_MAJOR} CsXml${BUILD_MAJOR} pthread)
set(EXTRA_RCC_CXXFLAGS " ") # whitespace is required!
set(EXTRA_RCC_LDFLAGS "-Wl,--no-undefined")

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtXml
    ${CMAKE_BINARY_DIR}/privateinclude/QtXml
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(RCC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rcc.cpp
)

set(RCC_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/rcc.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(EXTRA_RCC_LIBS
        ${EXTRA_RCC_LIBS}
        uuid
    )
endif()

add_executable(rcc ${RCC_SOURCES} ${RCC_INCLUDES})
target_link_libraries(rcc ${EXTRA_RCC_LIBS})
set_target_properties(rcc PROPERTIES
    COMPILE_FLAGS ${EXTRA_RCC_CXXFLAGS}
    LINK_FLAGS ${EXTRA_RCC_LDFLAGS}
)

install(TARGETS rcc DESTINATION ${CMAKE_INSTALL_BINDIR})

