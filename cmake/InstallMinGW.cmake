#
# Copyright (c) 2012-2019 Barbara Geller
# Copyright (c) 2012-2019 Ansel Sermersheim
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

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
   set(libgcc libgcc_s_seh-1)
else()
   set(libgcc libgcc_s_sjlj-1)
endif()

set(mingw_required_libraries
   ${libgcc}
   libstdc++-6
   libwinpthread-1
)

foreach(library ${mingw_required_libraries})
   find_library(${library}_LIBRARY ${library})
   list(APPEND CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${${library}_LIBRARY})
endforeach()

include(InstallRequiredSystemLibraries)
