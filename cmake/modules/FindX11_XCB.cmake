#  Copyright (C) 2012-2019 Barbara Geller
#  Copyright (C) 2012-2019 Ansel Sermersheim
#
#  Redistribution and use is allowed according to the terms of the BSD license.


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