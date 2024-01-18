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

#  Find Fontconfig, this will define
#
#  FONTCONFIG_FOUND - system has Fontconfig
#  FONTCONFIG_INCLUDES - the Fontconfig include directory
#  FONTCONFIG_LIBRARIES - The libraries needed to use Fontconfig


if(FONTCONFIG_INCLUDES AND FONTCONFIG_LIBRARIES)
    set(FONTCONFIG_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    find_package(PkgConfig)
    pkg_check_modules(PC_FONTCONFIG QUIET fontconfig)
endif()

find_path(FONTCONFIG_INCLUDES
    NAMES
    fontconfig/fontconfig.h
    HINTS
    $ENV{FONTCONFIGDIR}/include
    ${PC_FONTCONFIG_INCLUDEDIR}
    /usr/include
    /usr/local/include
    ${INCLUDE_INSTALL_DIR}
)

find_library(FONTCONFIG_LIBRARIES
    fontconfig
    HINTS
    $ENV{FONTCONFIGDIR}/lib
    ${PC_FONTCONFIG_LIBDIR}
    /usr/lib
    /usr/local/lib
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Fontconfig DEFAULT_MSG FONTCONFIG_INCLUDES FONTCONFIG_LIBRARIES)

mark_as_advanced(FONTCONFIG_INCLUDES FONTCONFIG_LIBRARIES)
