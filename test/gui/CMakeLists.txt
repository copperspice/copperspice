cmake_minimum_required(VERSION 3.20)

project(csguitest)

set(CMAKE_PREFIX_PATH "C:\\project\\copperspice-msvc\\x64\\release\\install\\cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)

find_package(CopperSpice REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
  message("No build type specified, defaulting to Debug (no optimizations, include debug symbols), use -DCMAKE_BUILD_TYPE=RelWithDebInfo or =Release to build a release)")
endif()

if(MSVC)
  # replace /Zx with /Z7 for tooling compatibility (Incredibuild and stashed.io)
  string(REGEX REPLACE "/Z[iI7]" "/Z7" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  string(REGEX REPLACE "/Z[iI7]" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

  string (REGEX REPLACE "/W3" "/W4" CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}"  )
  string (REGEX REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string (REGEX REPLACE ".INCREMENTAL[:NO]*" "/INCREMENTAL:NO" CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
else()
  # Linux, Windows (MinGW)
  set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-undefined")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
endif()

# Setting the rpath to '$ORIGIN' ensures our application looks for the
# CopperSpice libraries and plugins in its own directory first.
set(CMAKE_INSTALL_RPATH "\$ORIGIN")
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)

add_library(cstest INTERFACE)
target_include_directories(cstest INTERFACE ..)
target_link_libraries(cstest INTERFACE CopperSpice::CsCore CopperSpice::CsGui)

add_executable(fonts main.cpp fonts.cpp)
target_link_libraries(fonts PRIVATE cstest)
#target_link_options(fonts PRIVATE -static)

install(TARGETS fonts DESTINATION .)

# add_executable(qwebpage qwebpage.cpp)
# target_link_libraries(qwebpage PRIVATE cstest)

cs_copy_library(CsGui)
cs_copy_library(CsCore)
cs_copy_plugins(CsGui)

if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
   set(TARGETBITS 32)
else()
   set(TARGETBITS 64)
endif()

message("")
message("Copperspice is being consumed from:    ${Copperspice_DIR} ")
message("${PROJECT_NAME} configured to run on:  ${CMAKE_SYSTEM_NAME} ${TARGETBITS} bit, ${CMAKE_BUILD_TYPE} Mode")
message("${PROJECT_NAME} will be built in:      ${CMAKE_BINARY_DIR}")
message("${PROJECT_NAME} will be installed in:  ${CMAKE_INSTALL_PREFIX}")
message("\n")