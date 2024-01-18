# ***********************************************************************
#
# Copyright (c) 2012-2024 Barbara Geller
# Copyright (c) 2012-2024 Ansel Sermersheim
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
# ***********************************************************************

set(CMAKE_SYSTEM_NAME Windows)

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "(x86_64|amd64)")
    set(mingwtriplet "x86_64-w64-mingw32")
else()
    set(mingwtriplet "i686-w64-mingw32")
endif()

# which compilers to use for C and C++
set(CMAKE_C_COMPILER   ${mingwtriplet}-gcc)
set(CMAKE_CXX_COMPILER ${mingwtriplet}-g++)
set(CMAKE_RC_COMPILER  ${mingwtriplet}-windres)

# target environment located
set(CMAKE_FIND_ROOT_PATH /usr/${mingwtriplet})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(WINDRES_EXECUTABLE /usr/bin/${mingwtriplet}-windres)
