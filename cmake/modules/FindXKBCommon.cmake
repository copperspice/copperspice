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
pkg_check_modules(PC_XKBCOMMON QUIET xkbcommon)

set(XKBCOMMON_DEFINITIONS ${PC_XKBCOMMON_CFLAGS_OTHER})

find_path(XKBCOMMON_INCLUDE_DIR
    NAMES xkbcommon/xkbcommon.h
    HINTS ${PC_XKBCOMMON_INCLUDE_DIR} ${PC_XKBCOMMON_INCLUDE_DIRS}
)

find_library(XKBCOMMON_LIB
    NAMES xkbcommon
    HINTS ${PC_XKBCOMMON_LIBRARY} ${PC_XKBCOMMON_LIBRARY_DIRS}
)

set(XKBCOMMON_LIBRARIES    ${XKBCOMMON_LIB})
set(XKBCOMMON_LIBRARY_DIR  ${XKBCOMMON_LIBRARY_DIR})
set(XKBCOMMON_INCLUDE_DIR  ${XKBCOMMON_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XKBCommon DEFAULT_MSG
    XKBCOMMON_LIB
    XKBCOMMON_INCLUDE_DIR
)

mark_as_advanced(XKBCOMMON_LIB XKBCOMMON_INCLUDE_DIR)