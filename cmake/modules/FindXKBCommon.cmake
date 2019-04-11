#  Copyright (C) 2012-2019 Barbara Geller
#  Copyright (C) 2012-2019 Ansel Sermersheim
#
#  Redistribution and use is allowed according to the terms of the BSD license.


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