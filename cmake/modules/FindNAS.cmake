# ***********************************************************************
#
# Copyright (c) 2012-2021 Barbara Geller
# Copyright (c) 2012-2021 Ansel Sermersheim
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

#  Find Network Audio System, will define
#
#  NAS_FOUND - system has Network Audio System
#  NAS_INCLUDES - the Network Audio System include directory
#  NAS_LIBRARIES - The libraries needed to use Network Audio System


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
