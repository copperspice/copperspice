add_definitions(
   -DQT_UIC
   -DQT_UIC_CPP_GENERATOR
   -UQT_BUILD_CORE_LIB
)
set(EXTRA_UIC_LIBS CsCore${BUILD_ABI} ${CMAKE_THREAD_LIBS_INIT})
set(EXTRA_UIC_CXXFLAGS)
set(EXTRA_UIC_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/privateinclude
   ${CMAKE_BINARY_DIR}/include/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
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

if(${CMAKE_SYSTEM_NAME} MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   set(EXTRA_UIC_LDFLAGS  ${EXTRA_UIC_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "DARWIN")
   set(EXTRA_UIC_LDFLAGS  ${EXTRA_UIC_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(EXTRA_UIC_LDFLAGS  ${EXTRA_UIC_LDFLAGS} '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   set(EXTRA_UIC_LIBS ${EXTRA_UIC_LIBS} uuid)
endif()

function_variable_fixup("${EXTRA_UIC_CXXFLAGS}" EXTRA_UIC_CXXFLAGS)
function_variable_fixup("${EXTRA_UIC_LDFLAGS}" EXTRA_UIC_LDFLAGS)

add_executable(uic${TOOLS_SUFFIX} ${UIC_SOURCES} ${UIC_INCLUDES})
target_link_libraries(uic${TOOLS_SUFFIX} ${EXTRA_UIC_LIBS})

set_target_properties(uic${TOOLS_SUFFIX} PROPERTIES
   COMPILE_FLAGS ${EXTRA_UIC_CXXFLAGS}
   LINK_FLAGS ${EXTRA_UIC_LDFLAGS}
   EXPORT_NAME uic
)

install(
   TARGETS uic${TOOLS_SUFFIX}
   EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(CopperSpice::uic ALIAS uic${TOOLS_SUFFIX})