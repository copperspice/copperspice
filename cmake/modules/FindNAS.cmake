# - Try to find Network Audio System
# Once done this will define
#
#  NAS_FOUND - system has Network Audio System
#  NAS_INCLUDES - the Network Audio System include directory
#  NAS_LIBRARIES - The libraries needed to use Network Audio System
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if (NAS_INCLUDES AND NAS_LIBRARIES)
  set(NAS_FIND_QUIETLY TRUE)
endif (NAS_INCLUDES AND NAS_LIBRARIES)

find_path(NAS_INCLUDES
  NAMES
  audio.h
  PATH_SUFFIXES audio
  HINTS
  $ENV{NASDIR}/include
  /usr/include
  /usr/local/include
  ${INCLUDE_INSTALL_DIR}
)

find_library(NAS_LIBRARIES
  audio
  HINTS
  $ENV{NASDIR}/lib
  /usr/lib
  /usr/local/lib
  ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NAS DEFAULT_MSG 
                                  NAS_INCLUDES NAS_LIBRARIES)

mark_as_advanced(NAS_INCLUDES NAS_LIBRARIES)
