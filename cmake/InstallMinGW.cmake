#
# Copyright (C) 2012-2018 Barbara Geller
# Copyright (C) 2012-2018 Ansel Sermersheim
# All rights reserved.
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
