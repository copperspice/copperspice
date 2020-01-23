# ***********************************************************************
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
# ***********************************************************************

# Provides the following variables:
#
#  `MySQL_INCLUDE_DIRS`: Include directories necessary to use MySQL.
#  `MySQL_LIBRARIES`: Libraries necessary to use MySQL.
#  A `MySQL::MySQL` imported target.


# windows does not normally include MySQL
set(_MySQL_use_pkgconfig 0)
if (NOT WIN32)
   find_package(PkgConfig)

   if (PkgConfig_FOUND)
      set(_MySQL_use_pkgconfig 1)
   endif()

endif()

if (_MySQL_use_pkgconfig)
   pkg_check_modules(_libmariadb "libmariadb" QUIET IMPORTED_TARGET)
   unset(_mysql_target)

   if (_libmariadb_FOUND)
      set(_mysql_target "_libmariadb")
   else ()
      pkg_check_modules(_mariadb "mariadb" QUIET IMPORTED_TARGET)

      if (NOT _mariadb_FOUND)
         pkg_check_modules(_mysql "mysql" QUIET IMPORTED_TARGET)

         if (_mysql_FOUND)
            set(_mysql_target "_mysql")
         endif ()

      else ()
         set(_mysql_target "_mariadb")

         if (_mariadb_VERSION VERSION_LESS 10.4)

            get_property(_include_dirs
               TARGET    "PkgConfig::_mariadb"
               PROPERTY  "INTERFACE_INCLUDE_DIRECTORIES"
            )

            # remove "${prefix}/mariadb/.." from the interface since it breaks other projects
            list(FILTER _include_dirs EXCLUDE REGEX "\\.\\.")
            set_property(TARGET "PkgConfig::_mariadb"
               PROPERTY  "INTERFACE_INCLUDE_DIRECTORIES" "${_include_dirs}"
            )

            unset(_include_dirs)
         endif()
      endif()
   endif()

   set(MySQL_FOUND 0)

   if (_mysql_target)
      set(MySQL_FOUND 1)
      set(MySQL_INCLUDE_DIRS ${${_mysql_target}_INCLUDE_DIRS})
      set(MySQL_LIBRARIES    ${${_mysql_target}_LINK_LIBRARIES})

      if (NOT TARGET MySQL::MySQL)
         add_library(MySQL::MySQL INTERFACE IMPORTED)

         target_link_libraries(MySQL::MySQL
            INTERFACE  "PkgConfig::${_mysql_target}"
         )
      endif()
   endif()

   unset(_mysql_target)

else ()
   set(_MySQL_mariadb_versions 10.2 10.3)
   set(_MySQL_versions 5.0)
   set(_MySQL_paths)

   foreach (_MySQL_version IN LISTS _MySQL_mariadb_versions)
      list(APPEND _MySQL_paths
         "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MariaDB ${_MySQL_version};INSTALLDIR]"
         "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MariaDB ${_MySQL_version} (x64);INSTALLDIR]"
      )
   endforeach()

   foreach (_MySQL_version IN LISTS _MySQL_versions)
      list(APPEND _MySQL_paths
         "C:/Program Files/MySQL/MySQL Server ${_MySQL_version}/lib/opt"
         "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MySQL AB\\MySQL Server ${_MySQL_version};Location]"
         "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\MySQL AB\\MySQL Server ${_MySQL_version};Location]"
      )
   endforeach()

   unset(_MySQL_version)
   unset(_MySQL_versions)
   unset(_MySQL_mariadb_versions)


   find_path(MySQL_INCLUDE_DIR
      NAMES mysql.h
      PATHS  "C:/Program Files/MySQL/include"  "C:/MySQL/include"  ${_MySQL_paths}
      PATH_SUFFIXES include include/mysql
      DOC "Location of mysql.h"
   )

   mark_as_advanced(MySQL_INCLUDE_DIR)

   find_library(MySQL_LIBRARY
      NAMES libmariadb mysql libmysql mysqlclient
      PATHS  "C:/Program Files/MySQL/lib"  "C:/MySQL/lib/debug"  ${_MySQL_paths}
      PATH_SUFFIXES lib lib/opt
      DOC "Location of the mysql library"
   )

   mark_as_advanced(MySQL_LIBRARY)

   include(FindPackageHandleStandardArgs)
   find_package_handle_standard_args(MySQL
      REQUIRED_VARS
      MySQL_INCLUDE_DIR
      MySQL_LIBRARY
   )

   if (MySQL_FOUND)
      set(MySQL_INCLUDE_DIRS "${MySQL_INCLUDE_DIR}")
      set(MySQL_LIBRARIES "${MySQL_LIBRARY}")

      if (NOT TARGET MySQL::MySQL)
         add_library(MySQL::MySQL UNKNOWN IMPORTED)

         set_target_properties(MySQL::MySQL
            PROPERTIES
            IMPORTED_LOCATION  "${MySQL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES  "${MySQL_INCLUDE_DIR}"
         )
      endif()
   endif()
endif()

unset(_MySQL_use_pkgconfig)

