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
pkg_check_modules(PC_X11_XCB QUIET x11-xcb )

set(X11_XCB_DEFINITIONS ${PC_X11_XCB_CFLAGS_OTHER})

find_path(X11_XCB_INCLUDE_DIR
    NAMES X11/Xlib-xcb.h
    HINTS ${PC_X11_XCB_INCLUDE_DIR} ${PC_X11_XCB_INCLUDE_DIRS}
)

find_library(X11_XCB_LIB
    NAMES X11-xcb
    HINTS ${PC_X11_XCB_LIBRARY} ${PC_X11_XCB_LIBRARY_DIRS}
)

set(X11_XCB_LIBRARIES    ${X11_XCB_LIB})
set(X11_XCB_LIBRARY_DIR  ${X11_XCB_LIBRARY_DIR})
set(X11_XCB_INCLUDE_DIR  ${X11_XCB_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(X11_XCB DEFAULT_MSG
    X11_XCB_LIB
    X11_XCB_INCLUDE_DIR
)

mark_as_advanced(X11_XCB_LIB  X11_XCB_INCLUDE_DIR)