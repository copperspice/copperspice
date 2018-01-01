#
# Copyright (C) 2012-2018 Barbara Geller
# Copyright (C) 2012-2018 Ansel Sermersheim
# All rights reserved.    
#
# Copyright (c) 2008 Laurent Montel, <montel@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.

#  Find the GOBJECT2 libraries, will define
#
#  GOBJECT2_FOUND - system has gobject2
#  GOBJECT2_INCLUDE_DIR - the gobject2 include directory
#  GOBJECT2_LIBRARIES - gobject2 library


if(GOBJECT2_INCLUDE_DIR AND GOBJECT2_LIBRARIES)
    # Already in cache, be silent
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

# not sure if this include dir is optional or required
# for now it is optional
if(GOBJECT2_INTERNAL_INCLUDE_DIR)
  set(GOBJECT2_INCLUDE_DIR ${GOBJECT2_INCLUDE_DIR} "${GOBJECT2_INTERNAL_INCLUDE_DIR}")
endif(GOBJECT2_INTERNAL_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GOBJECT2  DEFAULT_MSG  GOBJECT2_LIBRARIES GOBJECT2_MAIN_INCLUDE_DIR)

mark_as_advanced(GOBJECT2_INCLUDE_DIR GOBJECT2_LIBRARIES)
