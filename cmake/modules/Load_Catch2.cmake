# ***********************************************************************
#
# Copyright (c) 2012-2021 Barbara Geller
# Copyright (c) 2012-2021 Ansel Sermersheim
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

find_package(Catch2 QUIET)

set_package_properties(Catch2 PROPERTIES
   PURPOSE "Required for Catch Unit Tests"
   DESCRIPTION "Unit test framework"
   URL "https://github.com/catchorg/Catch2"
   TYPE RECOMMENDED
)

if (NOT TARGET Catch2::Catch2)
   message(STATUS "Catch2 was not found, CopperSpice unit tests will not be built\n")
   return()
endif()
