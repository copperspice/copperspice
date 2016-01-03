# - Try to find Fontconfig
# Once done this will define
#
#  FONTCONFIG_FOUND - system has Fontconfig
#  FONTCONFIG_INCLUDES - the Fontconfig include directory
#  FONTCONFIG_LIBRARIES - The libraries needed to use Fontconfig
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(FONTCONFIG_INCLUDES AND FONTCONFIG_LIBRARIES)
    set(FONTCONFIG_FIND_QUIETLY TRUE)
endif()

find_path(FONTCONFIG_INCLUDES
    NAMES
    fontconfig/fontconfig.h
    HINTS
    $ENV{FONTCONFIGDIR}/include
    /usr/include
    /usr/local/include
    ${INCLUDE_INSTALL_DIR}
)

find_library(FONTCONFIG_LIBRARIES
    fontconfig
    HINTS
    $ENV{FONTCONFIGDIR}/lib
    /usr/lib
    /usr/local/lib
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Fontconfig DEFAULT_MSG FONTCONFIG_INCLUDES FONTCONFIG_LIBRARIES)

mark_as_advanced(FONTCONFIG_INCLUDES FONTCONFIG_LIBRARIES)
