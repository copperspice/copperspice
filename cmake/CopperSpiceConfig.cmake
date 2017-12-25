#
# Copyright (C) 2012-2018 Barbara Geller
# Copyright (C) 2012-2018 Ansel Sermersheim
# All rights reserved.
#
#  Config file for the CopperSpice package, defines the following variables:
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

# compute paths
get_filename_component(COPPERSPICE_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
get_filename_component(COPPERSPICE_PREFIX ${COPPERSPICE_CMAKE_DIR}/ ABSOLUTE)

# library and binary dependencies (contains definitions for IMPORTED targets)
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceLibraryTargets.cmake")
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceBinaryTargets.cmake")

# macros needed to build software linking with CopperSpice
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceMacros.cmake")

# IMPORTED targets
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
        CopperSpice::Cs${component}@BUILD_ABI@
    )
    set(COPPERSPICE_${uppercomp}_LIBRARIES
        CopperSpice::Cs${component}@BUILD_ABI@
    )
endforeach()

# set compiler standard to C++14    (todo: switch to using compiler features)
if(NOT CMAKE_VERSION VERSION_LESS "3.1.0")
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   set(CMAKE_CXX_STANDARD 14)

elseif(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang|AppleClang)")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

endif()
