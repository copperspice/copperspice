#
# Copyright (C) 2012-2018 Barbara Geller
# Copyright (C) 2012-2018 Ansel Sermersheim
# All rights reserved.    
#
# Copyright (c) 2016, Ivailo Monev, <xakepa10@gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

#  Find Iconv, will define
#
#  ICONV_FOUND - system has Iconv
#  ICONV_INCLUDES - the Iconv include directory
#  ICONV_LIBRARIES - The libraries needed to use Iconv
#


if(ICONV_INCLUDES AND ICONV_LIBRARIES)
    set(ICONV_FIND_QUIETLY TRUE)
endif()

# Iconv does not provide pkg-config files
find_path(ICONV_INCLUDES
    NAMES
    iconv.h
    HINTS
    $ENV{ICONVDIR}/include
    /usr/include
    /usr/local/include
    ${INCLUDE_INSTALL_DIR}
)

find_library(ICONV_LIBRARIES
    NAMES
    iconv libiconv libiconv-2 c
    HINTS
    $ENV{ICONVDIR}/lib
    /usr/lib
    /usr/local/lib
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Iconv DEFAULT_MSG ICONV_INCLUDES ICONV_LIBRARIES)

mark_as_advanced(ICONV_INCLUDES ICONV_LIBRARIES)
