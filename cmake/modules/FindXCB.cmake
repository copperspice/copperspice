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

find_package(PkgConfig)

if(NOT XCB_FIND_COMPONENTS)
    set(XCB_FIND_COMPONENTS xcb)
endif()

include(FindPackageHandleStandardArgs)
set(XCB_FOUND        true)
set(XCB_INCLUDE_DIRS "")
set(XCB_LIBRARIES    "")

foreach(comp ${XCB_FIND_COMPONENTS})
   # component name
   string(TOUPPER ${comp} compname)
   string(REPLACE "-" "_" compname ${compname})

   # header name
   string(REPLACE "xcb-" ""         headername  xcb/${comp}.h)
   string(REPLACE "-"    ""         headername  ${headername})
   string(REPLACE "xcb/" "xcb/xcb_" headername2 ${headername})

   # library name
   set(libname ${comp})

   pkg_check_modules(PC_${comp} QUIET ${comp} xcb)

   find_path(${compname}_INCLUDE_DIR NAMES ${headername} ${headername2}
      HINTS
      ${PC_${comp}_INCLUDE_DIR}
      ${PC_${comp}_INCLUDE_DIRS}
   )

   find_library(${compname}_LIB NAMES ${libname}
      HINTS
      ${PC_${comp}_LIBRARY}
      ${PC_${comp}_LIBRARY_DIRS}
   )

   if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.17.0")
      find_package_handle_standard_args(${comp}
         FOUND_VAR ${comp}_FOUND
         REQUIRED_VARS ${compname}_INCLUDE_DIR  ${compname}_LIB
         NAME_MISMATCHED
      )

   else()
      find_package_handle_standard_args(${comp}
         FOUND_VAR ${comp}_FOUND
         REQUIRED_VARS ${compname}_INCLUDE_DIR  ${compname}_LIB
      )
   endif()

   mark_as_advanced(${compname}_INCLUDE_DIR   ${compname}_LIB)

   list(APPEND XCB_INCLUDE_DIR  ${${compname}_INCLUDE_DIR})
   list(APPEND XCB_LIBRARIES    ${${compname}_LIB})

   if(NOT ${comp}_FOUND)
      set(XCB_FOUND false)
   endif()
endforeach()

list(REMOVE_DUPLICATES XCB_INCLUDE_DIRS)
