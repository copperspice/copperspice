add_definitions(-DQT_UIC -DQT_UIC_CPP_GENERATOR -UQT_BUILD_CORE_LIB)
set(EXTRA_UIC_LIBS CsCore${BUILD_MAJOR} pthread)
set(EXTRA_UIC_CXXFLAGS "-Wno-undefined")
set(EXTRA_UIC_LDFLAGS " ") # whitespace is required!

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp
)

set(UIC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppextractimages.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwritedeclaration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwriteicondata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwriteicondeclaration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwriteiconinitialization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwriteincludes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwriteinitialization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/customwidgetsinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/databaseinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/driver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/treewalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui4.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/validator.cpp
)

set(UIC_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppextractimages.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwritedeclaration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwriteicondata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwriteicondeclaration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwriteiconinitialization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwriteincludes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/cppwriteinitialization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/customwidgetsinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/databaseinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/driver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/globaldefs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/option.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qclass_lib_map.h
    ${CMAKE_CURRENT_SOURCE_DIR}/treewalker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/uic.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/validator.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(EXTRA_UIC_LIBS
        ${EXTRA_UIC_LIBS}
        uuid
    )
endif()

add_executable(uic ${UIC_SOURCES} ${UIC_INCLUDES})
target_link_libraries(uic ${EXTRA_UIC_LIBS})
set_target_properties(uic PROPERTIES
    COMPILE_FLAGS ${EXTRA_UIC_CXXFLAGS}
    LINK_FLAGS ${EXTRA_UIC_LDFLAGS}
)

install(TARGETS uic RUNTIME DESTINATION bin)

