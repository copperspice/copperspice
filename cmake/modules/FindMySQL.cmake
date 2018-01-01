#
# Copyright (C) 2012-2018 Barbara Geller
# Copyright (C) 2012-2018 Ansel Sermersheim
# All rights reserved.    
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

#  Find MySQL, will define
#
#  MYSQL_FOUND     - system has MySQL
#  MYSQL_INCLUDES  - the MySQL include directory
#  MYSQL_LIBRARIES - The libraries needed to use MySQL
#

if(MYSQL_INCLUDES AND MYSQL_LIBRARIES)
    set(MYSQL_FIND_QUIETLY TRUE)
endif()

# Neither MySQL nor MariaDB provide pkg-config files

find_path(MYSQL_INCLUDES
    NAMES
    mysql.h
    PATH_SUFFIXES mysql
    HINTS
    $ENV{MYSQLDIR}/include
    /usr/include
    /usr/local/include
    ${INCLUDE_INSTALL_DIR}
)

find_library(MYSQL_LIBRARIES
    mysqld
    HINTS
    $ENV{MYSQLDIR}/lib
    /usr/lib
    /usr/local/lib
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MySQL DEFAULT_MSG MYSQL_INCLUDES MYSQL_LIBRARIES)

mark_as_advanced(MYSQL_INCLUDES MYSQL_LIBRARIES)
