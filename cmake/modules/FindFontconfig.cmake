#
# Copyright (C) 2012-2018 Barbara Geller
# Copyright (C) 2012-2018 Ansel Sermersheim
# All rights reserved.    
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

# Find Fontconfig, this will define
#
#  FONTCONFIG_FOUND - system has Fontconfig
#  FONTCONFIG_INCLUDES - the Fontconfig include directory
#  FONTCONFIG_LIBRARIES - The libraries needed to use Fontconfig
#


if(FONTCONFIG_INCLUDES AND FONTCONFIG_LIBRARIES)
    set(FONTCONFIG_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
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
