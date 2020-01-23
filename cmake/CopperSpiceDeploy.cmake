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

# location of CsCore
get_target_property(CS_CORE_LIB CopperSpice::CsCore LOCATION)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   get_filename_component(CS_INSTALLED_LIB_DIR "${CS_CORE_LIB}" DIRECTORY)
   get_filename_component(CS_PLUGIN_DIR        "${CS_INSTALLED_LIB_DIR}/../lib" ABSOLUTE)
else()
   get_filename_component(CS_PLUGIN_DIR "${CS_CORE_LIB}" DIRECTORY)
endif()


function(cs_copy_library LIB_NAME)
   # location of the cs library
   get_target_property(CS_${LIB_NAME}_LIB CopperSpice::${LIB_NAME} LOCATION)

   if(ARGN EQUAL 1)
      set(APP_INSTALL_DIR ${ARG0})
   else()
      set(APP_INSTALL_DIR .)
   endif()

   install(FILES ${CS_${LIB_NAME}_LIB} DESTINATION ${APP_INSTALL_DIR})
endfunction()


function(cs_copy_plugins LIB_NAME)

   if(ARGN EQUAL 1)
      set(APP_INSTALL_DIR ${ARG0})
   else()
      set(APP_INSTALL_DIR .)
   endif()

   if(LIB_NAME STREQUAL "CsGui")

      if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
         install(FILES ${CS_PLUGIN_DIR}/CsGuiCocoa1.6.so DESTINATION ${APP_INSTALL_DIR}/platforms)

      elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
         install(FILES ${CS_PLUGIN_DIR}/CsGuiXcb1.6.so DESTINATION ${APP_INSTALL_DIR}/platforms)

      elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
         install(FILES ${CS_PLUGIN_DIR}/CsGuiWin1.6.dll DESTINATION ${APP_INSTALL_DIR}/platforms)

      endif()
   endif()

   if(LIB_NAME STREQUAL "CsMultimedia")

      if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
         install(FILES ${CS_PLUGIN_DIR}/CsMultimedia_avf_mediaplayer1.6.so DESTINATION ${APP_INSTALL_DIR}/mediaservices)
         install(FILES ${CS_PLUGIN_DIR}/CsMultimedia_m3u1.6.so DESTINATION ${APP_INSTALL_DIR}/playlistformats)

      elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
         install(FILES ${CS_PLUGIN_DIR}/CsMultimedia_gst_mediaplayer1.6.so DESTINATION ${APP_INSTALL_DIR}/mediaservices)
         install(FILES ${CS_PLUGIN_DIR}/CsMultimedia_gst_audiodecoder1.6.so DESTINATION ${APP_INSTALL_DIR}/audio)
         install(FILES ${CS_PLUGIN_DIR}/CsMultimedia_m3u1.6.so DESTINATION ${APP_INSTALL_DIR}/playlistformats)

      elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
#        get_target_property(DIRECTSHOW_LIB  CopperSpice::CsMultimedia_DirectShow LOCATION)
#        install(FILES  ${DIRECTSHOW_LIB}  DESTINATION ${APP_INSTALL_DIR}/mediaservices)

         install(FILES ${CS_PLUGIN_DIR}/CsMultimedia_DirectShow1.6.dll DESTINATION ${APP_INSTALL_DIR}/mediaservices)
         install(FILES ${CS_PLUGIN_DIR}/CsMultimedia_m3u1.6.dll DESTINATION ${APP_INSTALL_DIR}/playlistformats)

      endif()
   endif()

   if(LIB_NAME STREQUAL "CsSqlPsql")

      if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
         install(FILES ${CS_PLUGIN_DIR}/CsSqlPsql1.6.so DESTINATION ${APP_INSTALL_DIR}/sqldrivers)

      elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
         install(FILES ${CS_PLUGIN_DIR}/CsSqlPsql1.6.so DESTINATION ${APP_INSTALL_DIR}/sqldrivers)

      elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
         install(FILES ${CS_PLUGIN_DIR}/CsSqlPsql1.6.dll DESTINATION ${APP_INSTALL_DIR}/sqldrivers)

      endif()
   endif()

   if(LIB_NAME STREQUAL "CsPrinterDriver")

      if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
         install(FILES ${CS_PLUGIN_DIR}/CsPrinterDriverCups1.6.so DESTINATION ${APP_INSTALL_DIR}/printerdrivers)

      elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
         install(FILES ${CS_PLUGIN_DIR}/CsPrinterDriverCups1.6.so DESTINATION ${APP_INSTALL_DIR}/printerdrivers)

      elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
         install(FILES ${CS_PLUGIN_DIR}/CsPrinterDriverWin1.6.dll DESTINATION ${APP_INSTALL_DIR}/printerdrivers)

      endif()
   endif()

endfunction()


