# ***********************************************************************
#
# Copyright (c) 2012-2025 Barbara Geller
# Copyright (c) 2012-2025 Ansel Sermersheim
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
pkg_check_modules(PC_XKBCOMMON xkbcommon QUIET)

set(XKBCOMMON_DEFINITIONS ${PC_XKBCOMMON_CFLAGS_OTHER})

find_path(XKBCOMMON_INCLUDE_DIR
   NAMES xkbcommon/xkbcommon.h
   HINTS ${PC_XKBCOMMON_INCLUDE_DIR} ${PC_XKBCOMMON_INCLUDE_DIRS}
)

find_library(XKBCOMMON_LIB
   NAMES xkbcommon
   HINTS ${PC_XKBCOMMON_LIBRARY} ${PC_XKBCOMMON_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(XKBCommon DEFAULT_MSG
   XKBCOMMON_LIB
   XKBCOMMON_INCLUDE_DIR
)

if(XKBCOMMON_FOUND)
   add_library(XKBCommon::XKBCommon UNKNOWN IMPORTED)

   set_target_properties(XKBCommon::XKBCommon PROPERTIES
      IMPORTED_LOCATION "${XKBCOMMON_LIB}"
      INTERFACE_INCLUDE_DIRECTORIES "${XKBCOMMON_INCLUDE_DIR}")

   set_property(GLOBAL APPEND PROPERTY INTERNAL_DEPS_PROP XKBCommon::XKBCommon)
endif()
