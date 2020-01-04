#
# Copyright (c) 2012-2020 Barbara Geller
# Copyright (c) 2012-2020 Ansel Sermersheim
# Copyright (c) 2015 Ivailo Monev, <xakepa10@gmail.com>
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

macro(COPPERSPICE_RESOURCES RESOURCES)

   foreach(resource ${RESOURCES} ${ARGN})
      get_filename_component(rscext ${resource}  EXT)
      get_filename_component(rscname ${resource} NAME_WE)

      if(${rscext} STREQUAL ".qrc")
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/qrc_${rscname}.cpp)

         add_custom_command(
            OUTPUT ${rscout}

            COMMAND CopperSpice::rcc "${resource}" -o "${rscout}" -name ${rscname}
            MAIN_DEPENDENCY "${resource}"
         )

         set_property(SOURCE ${resource} APPEND PROPERTY OBJECT_DEPENDS ${rscout})

      elseif(${rscext} STREQUAL ".ts")
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/${rscname}.qm)

         add_custom_command(
            OUTPUT ${rscout}
            COMMAND CopperSpice::lrelease "${resource}" -qm "${rscout}"
            MAIN_DEPENDENCY "${resource}"
         )

      elseif(${rscext} STREQUAL ".ui")
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/ui_${rscname}.h)

         add_custom_command(
            OUTPUT ${rscout}
            COMMAND CopperSpice::uic "${resource}" -o "${rscout}"
            MAIN_DEPENDENCY "${resource}"
         )

      endif()

   endforeach()
endmacro()
