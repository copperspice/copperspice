# ***********************************************************************
#
# Copyright (c) 2012-2024 Barbara Geller
# Copyright (c) 2012-2024 Ansel Sermersheim
#
# This file is part of CopperSpice.
#
# CopperSpice is free software. You can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2.1 as published by the Free Software Foundation.
#
# CopperSpice is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# https://www.gnu.org/licenses/
#
# ***********************************************************************

#  Configuration file for CopperSpice installation, defines the following variables:
#
#  COPPERSPICE_INCLUDES               - list of every include directory
#  COPPERSPICE_LIBRARIES              - list of every library
#  COPPERSPICE_<COMPONENT>_INCLUDES   - <CsCore> include directories for this component
#  COPPERSPICE_<COMPONENT>_LIBRARIES  - <CsCore> libraries required to link this component

if(COPPERSPICE_FOUND)
   return()
endif()

set(COPPERSPICE_FOUND TRUE)

# figure out install path
get_filename_component(COPPERSPICE_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
get_filename_component(COPPERSPICE_PREFIX ${COPPERSPICE_CMAKE_DIR}/ ABSOLUTE)

# library and binary dependencies (contains definitions for IMPORTED targets)
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceLibraryTargets.cmake")
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceBinaryTargets.cmake")

# macros required to build software which links with CopperSpice
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceMacros.cmake")
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceDeploy.cmake")

# IMPORTED targets
set(COPPERSPICE_INCLUDES @CS_INST_INCLUDE@)
set(COPPERSPICE_LIBRARIES)
set(COPPERSPICE_COMPONENTS @BUILD_COMPONENTS@)

foreach(component ${COPPERSPICE_COMPONENTS})
   string(TOUPPER ${component} uppercomp)
   string(TOLOWER ${component} lowercomp)

   set(COPPERSPICE_INCLUDES
      ${COPPERSPICE_INCLUDES}
      @CS_INST_INCLUDE@/Qt${component}
   )

   set(COPPERSPICE_${uppercomp}_INCLUDES
      @CS_INST_INCLUDE@/Qt${component}
   )

   set(COPPERSPICE_LIBRARIES
      ${COPPERSPICE_LIBRARIES}
      CopperSpice::Cs${component}
   )

   set(COPPERSPICE_${uppercomp}_LIBRARIES
      CopperSpice::Cs${component}
   )
endforeach()

# export variable
set(CS_INSTALL_MODE     "@CS_INSTALL_MODE@")
set(CsLibGuarded_Deploy "@CsLibGuarded_FOUND@")
set(CsSignal_Deploy     "@CsSignal_FOUND@")
set(CsString_Deploy     "@CsString_FOUND@")

# test system dependencies in downstream projects
if ("${CS_INSTALL_MODE}" STREQUAL "Package")

   if (NOT TARGET CsLibGuarded::CsLibGuarded)
      message("CMake Issue: CopperSpice was built in Package Mode\n"
         "  Target library CsLibGuarded::CsLibGuarded was not found. Perhaps a find_package() call is missing?\n\n")

      message(FATAL_ERROR "Aborting CMake...\n")
   endif()

   if (NOT TARGET CsSignal::CsSignal)
      message("CMake Issue: CopperSpice was built in Package Mode\n"
         "  Target library CsSignal::CsSignal was not found. Perhaps a find_package() call is missing?\n\n")

      message(FATAL_ERROR "Aborting CMake...\n")
   endif()

   if (NOT TARGET CsString::CsString)
      message("CMake Issue: CopperSpice was built in Package Mode\n"
         "  Target library CsString::CsString was not found. Perhaps a find_package() call is missing?\n\n")

      message(FATAL_ERROR "Aborting CMake...\n")
   endif()
endif()

if ("${CS_INSTALL_MODE}" STREQUAL "Deploy")

   if (TARGET CsLibGuarded::CsLibGuarded)
      # CS was built with the system library, downstream project must use the version in CS

      message("CMake Issue: CopperSpice was built in Deploy Mode\n"
         "  Target library CsLibGuarded::CsLibGuarded was found, system library in CS must be used.\n\n")

      message(FATAL_ERROR "Aborting CMake...\n")
   endif()

   if (TARGET CsSignal::CsSignal)
      # CS was built with the system library, downstream project must use the version in CS

      message("CMake Issue: CopperSpice was built in Deploy Mode\n"
         "  Target library CsSignal::CsSignal was found, system library in CS must be used.\n\n")

      message(FATAL_ERROR "Aborting CMake...\n")
   endif()

   if (TARGET CsString::CsString)
      # CS was built with the system library, downstream project must use the version in CS

      message("CMake Issue: CopperSpice was built in Deploy Mode\n"
         "  Target library CsString::CsString was found, system library in CS must be used.\n\n")

      message(FATAL_ERROR "Aborting CMake...\n")
   endif()
endif()
