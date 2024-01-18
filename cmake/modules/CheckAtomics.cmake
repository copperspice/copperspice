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

INCLUDE(CheckCXXSourceCompiles)

function(check_working_cxx_atomics varname)

   CHECK_CXX_SOURCE_COMPILES("
#include <atomic>
#include <cstdint>
   std::atomic<uint64_t> x;
   int main() {
      x.store(5);
      return 0;
   }
   " ${varname})

endfunction(check_working_cxx_atomics)
