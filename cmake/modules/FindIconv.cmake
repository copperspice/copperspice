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

#  Find Iconv, will define
#
#  ICONV_FOUND - system has Iconv
#  ICONV_INCLUDES - the Iconv include directory
#  ICONV_LIBRARIES - The libraries needed to use Iconv


if(ICONV_INCLUDES AND ICONV_LIBRARIES)
    set(ICONV_FIND_QUIETLY TRUE)
endif()

# Iconv does not provide pkg-config files
find_path(ICONV_INCLUDES
    NAMES
    iconv.h
    HINTS
    $ENV{ICONVDIR}/include
    /usr/include
    /usr/local/include
    ${INCLUDE_INSTALL_DIR}
)

find_library(ICONV_LIBRARIES
    NAMES
    iconv libiconv libiconv-2 c
    HINTS
    $ENV{ICONVDIR}/lib
    /usr/lib
    /usr/local/lib
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Iconv DEFAULT_MSG ICONV_INCLUDES ICONV_LIBRARIES)

mark_as_advanced(ICONV_INCLUDES ICONV_LIBRARIES)
