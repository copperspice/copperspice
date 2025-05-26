# ***********************************************************************
#
# Copyright (c) 2012-2025 Barbara Geller
# Copyright (c) 2012-2025 Ansel Sermersheim
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

include(CheckSymbolExists)

macro(check_64_bit_io_functions varname)

   set(OLD_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")

   list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE)

   check_symbol_exists(stat64 "sys/stat.h" HAVE_STAT64)
   check_symbol_exists(readdir64 "dirent.h" HAVE_READDIR64)
   check_symbol_exists(open64 "fcntl.h" HAVE_OPEN64)

   set(CMAKE_REQUIRED_DEFINITIONS "${OLD_DEFINITIONS}")

   if(HAVE_STAT64 AND HAVE_READDIR64 AND HAVE_OPEN64)
      set(${varname} ON)
   else()
      set(${varname} OFF)
   endif()

endmacro()
