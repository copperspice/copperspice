#  Copyright (C) 2012-2019 Barbara Geller
#  Copyright (C) 2012-2019 Ansel Sermersheim
#
#  Redistribution and use is allowed according to the terms of the BSD license.


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