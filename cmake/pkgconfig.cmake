#
# Copyright (C) 2012-2018 Barbara Geller
# Copyright (C) 2012-2018 Ansel Sermersheim
# All rights reserved.    
#

prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@/@PC_REALNAME@

Name: @PC_NAME@
Description: @PC_NAME@ library
Version: @PACKAGE_VERSION@
Libs: -L${libdir} -l@PC_NAME@
Cflags: @PC_CFLAGS@ -I@CMAKE_INSTALL_FULL_INCLUDEDIR@ -I${includedir}
Requires: @PC_REQUIRES@
