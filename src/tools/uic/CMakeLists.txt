add_executable(uic${TOOLS_SUFFIX} "")
add_executable(CopperSpice::uic ALIAS uic${TOOLS_SUFFIX})

add_definitions(
   -DQT_UIC
   -DQT_UIC_CPP_GENERATOR
)

set(EXTRA_UIC_LIBS
   CsCore
)

set(EXTRA_UIC_CXXFLAGS)
set(EXTRA_UIC_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp
)

target_sources(uic${TOOLS_SUFFIX}
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/extract_images.h
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_declaration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_icondata.h
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_icondeclaration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_iconinitialization.h
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_includes.h
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_initialization.h

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

   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/extract_images.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_declaration.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_icondata.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_icondeclaration.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_iconinitialization.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_includes.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/cpp/write_initialization.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/customwidgetsinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/databaseinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/driver.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/treewalker.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/ui4.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/uic.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/validator.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "DARWIN")
   list(APPEND EXTRA_UIC_LDFLAGS '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
   list(APPEND EXTRA_UIC_LDFLAGS  '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   list(APPEND EXTRA_UIC_LDFLAGS  '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   list(APPEND EXTRA_UIC_LIBS
      uuid
   )
endif()

function_variable_fixup("${EXTRA_UIC_CXXFLAGS}" EXTRA_UIC_CXXFLAGS)
function_variable_fixup("${EXTRA_UIC_LDFLAGS}"  EXTRA_UIC_LDFLAGS)

target_link_libraries(uic${TOOLS_SUFFIX}
   PRIVATE
   ${EXTRA_UIC_LIBS}
)

set_target_properties(uic${TOOLS_SUFFIX}
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_UIC_CXXFLAGS}
   LINK_FLAGS ${EXTRA_UIC_LDFLAGS}
   EXPORT_NAME uic
)

#
#   If the files in bin didn't name collide with Qt binaries we could just leave them.
#
if(BUILDING_RPM)
    install(
       TARGETS uic${TOOLS_SUFFIX}
       EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
       DESTINATION ${CMAKE_INSTALL_LIBDIR}/copperspice/bin
    )
else()
    install(
       TARGETS uic${TOOLS_SUFFIX}
       EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
       DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()