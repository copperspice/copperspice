#
# Copyright (C) 2012-2018 Barbara Geller
# Copyright (C) 2012-2018 Ansel Sermersheim
# All rights reserved.    
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

# Find Freetype2, this will define
#
#  FREETYPE_FOUND     - system has Freetype2
#  FREETYPE_INCLUDES  - the Freetype2 include directory
#  FREETYPE_LIBRARIES - The libraries needed to use Freetype2

if(FREETYPE_INCLUDES AND FREETYPE_LIBRARIES)
    set(FREETYPE_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_FREETYPE QUIET freetype2)
endif()

find_path(FREETYPE_INCLUDES
    NAMES
    ft2build.h
    PATH_SUFFIXES freetype2 freetype2/freetype
    HINTS
    $ENV{FREETYPEDIR}/include
    ${PC_FREETYPE_INCLUDEDIR}
    /usr/include
    /usr/local/include
    ${INCLUDE_INSTALL_DIR}
)

find_library(FREETYPE_LIBRARIES
    freetype
    HINTS
    $ENV{FREETYPEDIR}/lib
    ${PC_FREETYPE_LIBDIR}
    /usr/lib
    /usr/local/lib
    ${LIB_INSTALL_DIR}
)

# CMake still refers to its Freetype module, make it happy
if(FREETYPE_INCLUDES)
    set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDES})
endif()
# usually CMake handles this but in this case it does not
if(FREETYPE_INCLUDES AND FREETYPE_LIBRARIES)
    set(FREETYPE_FOUND TRUE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Freetype2 DEFAULT_MSG FREETYPE_INCLUDES FREETYPE_LIBRARIES)

mark_as_advanced(FREETYPE_INCLUDES FREETYPE_LIBRARIES)
