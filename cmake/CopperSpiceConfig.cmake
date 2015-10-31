# - Config file for the CopperSpice package
#
# It defines the following variables:
#
#  COPPERSPICE_INCLUDES               - all include directories
#  COPPERSPICE_LIBRARIES              - all libraries to link against
#  COPPERSPICE_<COMPONENT>_INCLUDES   - component linclude directories for e.g. CsCore
#  COPPERSPICE_<COMPONENT>_LIBRARIES  - component libraries to link against e.g. CsCore
#

if(COPPERSPICE_FOUND)
    return()
endif()
set(COPPERSPICE_FOUND TRUE)

# Compute paths
get_filename_component(COPPERSPICE_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
get_filename_component(COPPERSPICE_PREFIX ${COPPERSPICE_CMAKE_DIR}/ ABSOLUTE)

# Our library and binary dependencies (contains definitions for IMPORTED targets)
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceLibraryTargets.cmake")
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceBinaryTargets.cmake")
# Usefull macros that most people will need to build software that uses CopperSpice
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceMacros.cmake")

# These are IMPORTED targets
set(COPPERSPICE_INCLUDES @CMAKE_INSTALL_FULL_INCLUDEDIR@)
set(COPPERSPICE_LIBRARIES)
set(COPPERSPICE_COMPONENTS @BUILD_COMPONENTS@)
foreach(component ${COPPERSPICE_COMPONENTS})
    string(TOUPPER ${component} uppercomp)
    string(TOLOWER ${component} lowercomp)
    if(${lowercomp} STREQUAL "phonon")
        set(COPPERSPICE_INCLUDES
            ${COPPERSPICE_INCLUDES}
            @CMAKE_INSTALL_FULL_INCLUDEDIR@/phonon
        )
        set(COPPERSPICE_${uppercomp}_INCLUDES
            @CMAKE_INSTALL_FULL_INCLUDEDIR@/phonon
        )
    else()
        set(COPPERSPICE_INCLUDES
            ${COPPERSPICE_INCLUDES}
            @CMAKE_INSTALL_FULL_INCLUDEDIR@/Qt${component}
        )
        set(COPPERSPICE_${uppercomp}_INCLUDES
            @CMAKE_INSTALL_FULL_INCLUDEDIR@/Qt${component}
        )
    endif()

    set(COPPERSPICE_LIBRARIES
        ${COPPERSPICE_LIBRARIES}
        CopperSpice::Cs${component}@BUILD_MAJOR@
    )
    set(COPPERSPICE_${uppercomp}_LIBRARIES
        CopperSpice::Cs${component}@BUILD_MAJOR@
    )
endforeach()

# Set compiler standard to C++ 11
if(CMAKE_MAJOR_VERSION VERSION_EQUAL 3 AND CMAKE_MINOR_VERSION VERSION_GREATER 0)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_STANDARD 11)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang|AppleClang)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()