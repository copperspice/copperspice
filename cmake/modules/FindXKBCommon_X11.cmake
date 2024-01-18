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

find_package(PkgConfig)
pkg_check_modules(PC_XKB11 QUIET xkbcommon-x11)

set(XKBCOMMON_X11_DEFINITIONS ${PC_XKB11_CFLAGS_OTHER})

find_path(XKBCOMMON_X11_INCLUDE_DIR
    NAMES xkbcommon/xkbcommon-x11.h
    HINTS ${PC_XKB11_INCLUDE_DIR} ${PC_XKB11_INCLUDE_DIRS}
)

find_library(XKBCOMMON_X11_LIB
    NAMES xkbcommon-x11
    HINTS ${PC_XKB11_LIBRARY} ${PC_XKB11_LIBRARY_DIRS}
)

set(XKBCOMMON_X11_LIBRARIES    ${XKBCOMMON_X11_LIB})
set(XKBCOMMON_X11_LIBRARY_DIR  ${XKBCOMMON_X11_LIBRARY_DIR})
set(XKBCOMMON_X11_INCLUDE_DIR  ${XKBCOMMON_X11_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XKBCommon_X11 DEFAULT_MSG
    XKBCOMMON_X11_LIB
    XKBCOMMON_X11_INCLUDE_DIR
)

mark_as_advanced(XKBCOMMON_X11_LIB  XKBCOMMON_X11_INCLUDE_DIR)