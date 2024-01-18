# ***********************************************************************
#
# Copyright (c) 2012-2024 Barbara Geller
# Copyright (c) 2012-2024 Ansel Sermersheim
# Copyright (c) 2015 Ivailo Monev, <xakepa10@gmail.com>
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

#  GOBJECT2_FOUND       - system has gobject2
#  GOBJECT2_INCLUDE_DIR - the gobject2 include directory
#  GOBJECT2_LIBRARIES   - gobject2 library

if(GOBJECT2_INCLUDE_DIR AND GOBJECT2_LIBRARIES)
   # Already in cache
   set(GOBJECT2_FIND_QUIETLY TRUE)
endif()

find_package(PkgConfig)
pkg_check_modules(PC_LibGOBJECT2 QUIET gobject-2.0)

find_path(GOBJECT2_MAIN_INCLUDE_DIR
   NAMES gobject.h
   HINTS ${PC_LibGOBJECT2_INCLUDEDIR}
   PATH_SUFFIXES glib-2.0/gobject/)

find_library(GOBJECT2_LIBRARY
   NAMES gobject-2.0
   HINTS ${PC_LibGOBJECT2_LIBDIR}
)

set(GOBJECT2_LIBRARIES ${GOBJECT2_LIBRARY})

# search the gobjectconfig.h include dir under the same root where the library is found
get_filename_component(gobject2LibDir "${GOBJECT2_LIBRARIES}" PATH)

find_path(GOBJECT2_INTERNAL_INCLUDE_DIR glibconfig.h
   PATH_SUFFIXES glib-2.0/include
   HINTS ${PC_LibGOBJECT2_INCLUDEDIR} "${gobject2LibDir}" ${CMAKE_SYSTEM_LIBRARY_PATH})

set(GOBJECT2_INCLUDE_DIR "${GOBJECT2_MAIN_INCLUDE_DIR}")

# for now, include dir is optional
if(GOBJECT2_INTERNAL_INCLUDE_DIR)
   set(GOBJECT2_INCLUDE_DIR ${GOBJECT2_INCLUDE_DIR} "${GOBJECT2_INTERNAL_INCLUDE_DIR}")
endif()

include(FindPackageHandleStandardArgs)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.17.0")
   find_package_handle_standard_args(GOBJECT2  DEFAULT_MSG  GOBJECT2_LIBRARIES GOBJECT2_MAIN_INCLUDE_DIR NAME_MISMATCHED)
else()
   find_package_handle_standard_args(GOBJECT2  DEFAULT_MSG  GOBJECT2_LIBRARIES GOBJECT2_MAIN_INCLUDE_DIR)
endif()

mark_as_advanced(GOBJECT2_INCLUDE_DIR GOBJECT2_LIBRARIES)
