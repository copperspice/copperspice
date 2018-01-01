#
# Copyright (C) 2012-2018 Barbara Geller
# Copyright (C) 2012-2018 Ansel Sermersheim
# All rights reserved.    
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

#  Find Network Audio System, will define
#
#  NAS_FOUND - system has Network Audio System
#  NAS_INCLUDES - the Network Audio System include directory
#  NAS_LIBRARIES - The libraries needed to use Network Audio System
#


if(NAS_INCLUDES AND NAS_LIBRARIES)
    set(NAS_FIND_QUIETLY TRUE)
endif()

# NAS does not provide pkg-config files
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
find_package_handle_standard_args(NAS DEFAULT_MSG NAS_INCLUDES NAS_LIBRARIES)

mark_as_advanced(NAS_INCLUDES NAS_LIBRARIES)
