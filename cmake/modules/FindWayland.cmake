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

#  Wayland_FOUND         - True if Wayland was found
#  Wayland_INCLUDE_DIRS  - Include directories for Wayland
#  Wayland_LIBRARIES     - Libraries to link against

#  Wayland::Client       - Wayland client target
#  Wayland::Server       - Wayland server target
#  Wayland::Cursor       - Wayland cursor target
#  Wayland::EGL          - Wayland EGL target
#  Wayland::Scanner      - Wayland scanner executable


find_package(PkgConfig REQUIRED)

# List of components
set(WAYLAND_COMPONENTS wayland-client wayland-server wayland-cursor wayland-egl)

foreach(_component ${WAYLAND_COMPONENTS})
   string(REPLACE "-" "_" pkgName ${_component})

   pkg_check_modules(${pkgName} QUIET ${_component})

   if(${pkgName}_FOUND)
      set(Wayland_FOUND TRUE)

      set(Wayland_${pkgName}_FOUND TRUE)
      set(Wayland_INCLUDE_DIRS ${Wayland_INCLUDE_DIRS} ${${pkgName}_INCLUDE_DIRS})
      set(Wayland_LIBRARIES ${Wayland_LIBRARIES} ${${pkgName}_LIBRARIES})

      if("${pkgName}" STREQUAL "wayland_client")
         set(libraryName "Client")

      elseif("${pkgName}" STREQUAL "wayland_server")
         set(libraryName "Server")

      elseif("${pkgName}" STREQUAL "wayland_cursor")
         set(libraryName "Cursor")

      elseif("${pkgName}" STREQUAL "wayland_egl")
         set(libraryName "EGL")

      endif()

      add_library(Wayland::${libraryName} INTERFACE IMPORTED)

      set_target_properties(Wayland::${libraryName} PROPERTIES
         INTERFACE_LINK_LIBRARIES "${${pkgName}_LIBRARIES}"
         INTERFACE_INCLUDE_DIRECTORIES "${${pkgName}_INCLUDE_DIRS}"
      )
   endif()
endforeach()

# Find wayland-scanner executable
find_program(WAYLAND_SCANNER_EXECUTABLE
   NAMES wayland-scanner
)

if(WAYLAND_SCANNER_EXECUTABLE)
   add_executable(Wayland::Scanner IMPORTED)

   set_target_properties(Wayland::Scanner PROPERTIES
      IMPORTED_LOCATION "${WAYLAND_SCANNER_EXECUTABLE}"
   )
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Wayland
   REQUIRED_VARS
      Wayland_LIBRARIES
      WAYLAND_SCANNER_EXECUTABLE
   HANDLE_COMPONENTS
)

mark_as_advanced(
   Wayland_INCLUDE_DIRS
   Wayland_LIBRARIES
   WAYLAND_SCANNER_EXECUTABLE
)
