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

#  Find Freetype2, this will define
#
#  FREETYPE_FOUND     - system has Freetype2
#  FREETYPE_INCLUDES  - the Freetype2 include directory
#  FREETYPE_LIBRARIES - The libraries needed to use Freetype2


if(FREETYPE_INCLUDES AND FREETYPE_LIBRARIES)
    set(FREETYPE_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_FREETYPE QUIET freetype2)
endif()

find_path(FREETYPE_INCLUDES
    NAMES
    ft2build.h
    PATH_SUFFIXES freetype2 freetype2/freetype
    HINTS
    $ENV{FREETYPEDIR}/include
    ${PC_FREETYPE_INCLUDEDIR}
    /usr/include
    /usr/local/include
    ${INCLUDE_INSTALL_DIR}
)

find_library(FREETYPE_LIBRARIES
    freetype
    HINTS
    $ENV{FREETYPEDIR}/lib
    ${PC_FREETYPE_LIBDIR}
    /usr/lib
    /usr/local/lib
    ${LIB_INSTALL_DIR}
)

# CMake still refers to its Freetype module, make it happy
if(FREETYPE_INCLUDES)
    set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDES})
endif()
# usually CMake handles this but in this case it does not
if(FREETYPE_INCLUDES AND FREETYPE_LIBRARIES)
    set(FREETYPE_FOUND TRUE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Freetype2 DEFAULT_MSG FREETYPE_INCLUDES FREETYPE_LIBRARIES)

mark_as_advanced(FREETYPE_INCLUDES FREETYPE_LIBRARIES)
