#
# Copyright (c) 2012-2020 Barbara Geller
# Copyright (c) 2012-2020 Ansel Sermersheim
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
