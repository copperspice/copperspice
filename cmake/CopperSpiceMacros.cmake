# ***********************************************************************
#
# Copyright (c) 2012-2026 Barbara Geller
# Copyright (c) 2012-2026 Ansel Sermersheim
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
# ***********************************************************************

macro(COPPERSPICE_RESOURCES RESOURCES)

   set(T_PATH "")

   if ("${CS_INSTALL_MODE}" STREQUAL "Package")

      if (CsSignal_FOUND)
         # cs was built in package mode

         if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
            set(EXTRA_PATH "$<TARGET_FILE_DIR:CsSignal::CsSignal>;$<TARGET_FILE_DIR:CsString::CsString>")
            set(T_PATH "DYLD_LIBRARY_PATH=$<JOIN:$ENV{DYLD_LIBRARY_PATH};${EXTRA_PATH},:>")

         elseif (CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
            set(T_PATH "LD_LIBRARY_PATH=$<JOIN:$ENV{LD_LIBRARY_PATH};$<TARGET_FILE_DIR:CsSignal::CsSignal>,:>")

         elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
            set(T_PATH "PATH=$<JOIN:$ENV{PATH};$<TARGET_FILE_DIR:CsSignal::CsSignal>,;>")

         endif()
      endif()

   endif()

      foreach(resource ${RESOURCES} ${ARGN})
         get_filename_component(rscext ${resource}  EXT)
         get_filename_component(rscname ${resource} NAME_WE)

         if("${rscext}" STREQUAL ".qrc")
            set(rscout ${CMAKE_CURRENT_BINARY_DIR}/qrc_${rscname}.cpp)

            if ("${T_PATH}" STREQUAL "")
               add_custom_command(
                  OUTPUT ${rscout}
                  COMMAND CopperSpice::rcc "${resource}" -o "${rscout}" -name ${rscname}
                  MAIN_DEPENDENCY "${resource}"
               )

            else()
               # cs was built in package mode

               add_custom_command(
                  OUTPUT ${rscout}
                  COMMAND cmake -E env "${T_PATH}" "$<TARGET_FILE:CopperSpice::rcc>"
                        "${resource}" -o "${rscout}" -name ${rscname}
                  MAIN_DEPENDENCY "${resource}"
               )
            endif()

            set_property(SOURCE ${resource} APPEND PROPERTY OBJECT_DEPENDS ${rscout})

         elseif("${rscext}" STREQUAL ".ts")

            if("${TS_OUTPUT_DIR}" STREQUAL "")
               set(rscout ${CMAKE_CURRENT_BINARY_DIR}/${rscname}.qm)
            else()
               set(rscout ${TS_OUTPUT_DIR}/${rscname}.qm)
            endif()

            if ("${T_PATH}" STREQUAL "")
               add_custom_command(
                  OUTPUT ${rscout}
                  COMMAND CopperSpice::lrelease "${resource}" -qm "${rscout}"
                  MAIN_DEPENDENCY "${resource}"
               )

            else()
               # cs was built in package mode

               add_custom_command(
                  OUTPUT ${rscout}
                  COMMAND cmake -E env "$<JOIN:${T_PATH},;>" "$<TARGET_FILE:CopperSpice::lrelease>"
                        "${resource}" -qm "${rscout}"
                  MAIN_DEPENDENCY "${resource}"
               )

            endif()

      elseif("${rscext}" STREQUAL ".ui")
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/ui_${rscname}.h)

         if ("${T_PATH}" STREQUAL "")
            add_custom_command(
               OUTPUT ${rscout}
               COMMAND CopperSpice::uic "${resource}" -o "${rscout}"
               MAIN_DEPENDENCY "${resource}"
            )

         else()
            # cs was built in package mode

            add_custom_command(
               OUTPUT ${rscout}
               COMMAND cmake -E env "$<JOIN:${T_PATH},;>" "$<TARGET_FILE:CopperSpice::uic>"
                     "${resource}" -o "${rscout}"
               MAIN_DEPENDENCY "${resource}"
            )

         endif()

      elseif("${rscext}" STREQUAL ".xml" AND "${rscname}" MATCHES "wl-.*")
         string(SUBSTRING "${rscname}" 3 -1 rscname)

         # cs wayland scanner (1a)
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/qwayland-server-${rscname}.h)

         if ("${T_PATH}" STREQUAL "")
            add_custom_command(
               OUTPUT ${rscout}
               COMMAND CopperSpice::cs_wayland_scanner server-header "${resource}" "${rscout}"
               MAIN_DEPENDENCY "${resource}"
            )

         else()
            # cs was built in package mode

            add_custom_command(
               OUTPUT ${rscout}
               COMMAND cmake -E env "$<JOIN:${T_PATH},;>" "$<TARGET_FILE:CopperSpice::cs_wayland_scanner>"
                     server-header "${resource}" "${rscout}"
               MAIN_DEPENDENCY "${resource}"
            )

         endif()

         # cs wayland scanner (2a)
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/qwayland-${rscname}.h)

         if ("${T_PATH}" STREQUAL "")
            add_custom_command(
               OUTPUT ${rscout}
               COMMAND CopperSpice::cs_wayland_scanner client-header "${resource}" "${rscout}"
               MAIN_DEPENDENCY "${resource}"
            )

         else()
            # cs was built in package mode

            add_custom_command(
               OUTPUT ${rscout}
               COMMAND cmake -E env "$<JOIN:${T_PATH},;>" "$<TARGET_FILE:CopperSpice::cs_wayland_scanner>"
                     client-header "${resource}" "${rscout}"
               MAIN_DEPENDENCY "${resource}"
            )

         endif()

         # cs wayland scanner (3a)
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/qwayland-server-${rscname}.cpp)

         if ("${T_PATH}" STREQUAL "")
            add_custom_command(
               OUTPUT ${rscout}
               COMMAND CopperSpice::cs_wayland_scanner server-code "${resource}" "${rscout}"
               MAIN_DEPENDENCY "${resource}"
            )

         else()
            # cs was built in package mode

            add_custom_command(
               OUTPUT ${rscout}
               COMMAND cmake -E env "$<JOIN:${T_PATH},;>" "$<TARGET_FILE:CopperSpice::cs_wayland_scanner>"
                     server-code "${resource}" "${rscout}"
               MAIN_DEPENDENCY "${resource}"
            )

         endif()

         # cs wayland scanner (4a)
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/qwayland-${rscname}.cpp)

         if ("${T_PATH}" STREQUAL "")
            add_custom_command(
               OUTPUT ${rscout}
               COMMAND CopperSpice::cs_wayland_scanner client-code "${resource}" "${rscout}"
               MAIN_DEPENDENCY "${resource}"
            )

         else()
            # cs was built in package mode

            add_custom_command(
               OUTPUT ${rscout}
               COMMAND cmake -E env "$<JOIN:${T_PATH},;>" "$<TARGET_FILE:CopperSpice::cs_wayland_scanner>"
                     client-code "${resource}" "${rscout}"
               MAIN_DEPENDENCY "${resource}"
            )

         endif()

         # upstream wayland scanner (1b)
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/wayland-${rscname}-server-protocol.h)

         add_custom_command(
            OUTPUT ${rscout}
            COMMAND wayland-scanner server-header "${resource}" "${rscout}"
            MAIN_DEPENDENCY "${resource}"
         )

         # upstream wayland scanner (2b)
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/wayland-${rscname}-client-protocol.h)

         add_custom_command(
            OUTPUT ${rscout}
            COMMAND wayland-scanner client-header "${resource}" "${rscout}"
            MAIN_DEPENDENCY "${resource}"
         )

         # upstream wayland scanner (3b)
         set(rscout ${CMAKE_CURRENT_BINARY_DIR}/wayland-${rscname}-protocol.c)

         add_custom_command(
            OUTPUT ${rscout}
            COMMAND wayland-scanner public-code "${resource}" "${rscout}"
            MAIN_DEPENDENCY "${resource}"
         )

      endif()

   endforeach()
endmacro()
