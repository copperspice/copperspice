#
# Copyright (c) 2012-2019 Barbara Geller
# Copyright (c) 2012-2019 Ansel Sermersheim
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
set(COPPERSPICE_INCLUDES @CMAKE_INSTALL_FULL_INCLUDEDIR@)
set(COPPERSPICE_LIBRARIES)
set(COPPERSPICE_COMPONENTS @BUILD_COMPONENTS@)

foreach(component ${COPPERSPICE_COMPONENTS})
   string(TOUPPER ${component} uppercomp)
   string(TOLOWER ${component} lowercomp)

   set(COPPERSPICE_INCLUDES
      ${COPPERSPICE_INCLUDES}
      @CMAKE_INSTALL_FULL_INCLUDEDIR@/Qt${component}
   )

   set(COPPERSPICE_${uppercomp}_INCLUDES
      @CMAKE_INSTALL_FULL_INCLUDEDIR@/Qt${component}
   )

   set(COPPERSPICE_LIBRARIES
      ${COPPERSPICE_LIBRARIES}
      CopperSpice::Cs${component}
   )

   set(COPPERSPICE_${uppercomp}_LIBRARIES
      CopperSpice::Cs${component}
   )
endforeach()
