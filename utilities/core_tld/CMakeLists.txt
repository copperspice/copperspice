cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)

cmake_policy(VERSION 3.18.0..3.29.6)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20.0")
   # enable RTTI on MSVC
   cmake_policy(SET CMP0117 OLD)
endif()

project(Generate_TLD)

set(BUILD_MAJOR "1")
set(BUILD_MINOR "0")
set(BUILD_MICRO "0")

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)

find_package(CopperSpice REQUIRED)

# location for install or package
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   include(GNUInstallDirs)

   # rpath used for rcc and uic when compiling a user application
   file(RELATIVE_PATH CS_BIN_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
                                   "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

   set(CMAKE_INSTALL_RPATH "@executable_path/${CS_BIN_RPATH}")

elseif (CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
   include(GNUInstallDirs)

   # rpath used for rcc and uic when compiling a user application
   file(RELATIVE_PATH CS_BIN_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
                                   "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

   set(CMAKE_INSTALL_RPATH "$ORIGIN/${CS_BIN_RPATH}")

elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
   set(CMAKE_INSTALL_BINDIR bin)
   set(CMAKE_INSTALL_LIBDIR lib)
   set(CMAKE_INSTALL_INCLUDEDIR include)

endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,-undefined,error")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,error")
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-undefined,error")

elseif (CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD|DragonFly)")
   set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-undefined")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ")
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")

elseif (MSVC)
   string (REGEX REPLACE "/W3" "" CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}"  )
   string (REGEX REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

   add_compile_options("/utf-8")

else()
   # Linux, Windows (MinGW)

   set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-undefined")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")

   if (CMAKE_SYSTEM_NAME MATCHES "Windows")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
      set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wa,-mbig-obj")
   endif()

endif()

# location for building binary files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
   set(TARGETBITS 32)
else()
   set(TARGETBITS 64)
endif()

message("")
message("Generate_TLD configured to run on:  ${CMAKE_SYSTEM_NAME} ${TARGETBITS} bit, ${CMAKE_BUILD_TYPE} Mode")
message("Generate_TLD will be built in:      ${CMAKE_BINARY_DIR}")
message("Generate_TLD will be installed in:  ${CMAKE_INSTALL_PREFIX}")
message("\n")


# part two

list(APPEND GENERATE_TLD_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

add_executable(Generate_TLD ${GENERATE_TLD_SOURCES} )

target_link_libraries(Generate_TLD
   CopperSpice::CsCore
)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   set_target_properties(Generate_TLD PROPERTIES OUTPUT_NAME generate_tld)

elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
   set_target_properties(Generate_TLD PROPERTIES OUTPUT_NAME generate_tld)

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_link_libraries(Generate_TLD
      netapi32
      mpr
   )

endif()

install(TARGETS Generate_TLD DESTINATION .)

# install the CopperSpice libraries
cs_copy_library(CsCore)
