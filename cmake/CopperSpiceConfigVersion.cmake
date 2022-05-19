#
# Copyright (c) 2012-2022 Barbara Geller
# Copyright (c) 2012-2022 Ansel Sermersheim
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

set(COPPERSPICE_VERSION_MAJOR "@BUILD_MAJOR@" PARENT_SCOPE)
set(COPPERSPICE_VERSION_MINOR "@BUILD_MINOR@" PARENT_SCOPE)
set(COPPERSPICE_VERSION_PATCH "@BUILD_MICRO@" PARENT_SCOPE)

set(COPPERSPICE_VERSION       "@BUILD_MAJOR@.@BUILD_MINOR@.@BUILD_MICRO@" PARENT_SCOPE)
set(COPPERSPICE_VERSION_API   "@BUILD_MAJOR@.@BUILD_MINOR@" PARENT_SCOPE)

set(PACKAGE_VERSION "@BUILD_MAJOR@.@BUILD_MINOR@.@BUILD_MICRO@")
