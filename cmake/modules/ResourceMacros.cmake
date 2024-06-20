# ***********************************************************************
#
# Copyright (c) 2012-2024 Barbara Geller
# Copyright (c) 2012-2024 Ansel Sermersheim
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

#   MACRO_GENERATE_PUBLIC(<FancyHeaderName> [<FancyHeaderName2>] ... <component>)
#   MACRO_GENERATE_PRIVATE(<header.h> [<header2.h>] ... <component>)
#   MACRO_GENERATE_MISC(<header.h> [<header2.h>] ... <component>)
#   FUNCTION_GENERATE_RESOURCES(target_library_or_application)
#   MACRO_WINDOWS_RESOURCES(<windowsmanifest.manifest> [<windowsresource.rc>] ...)
#   MACRO_GENERATE_PACKAGE(<name> <realname> <cxxflags> <libraries> <requires>)
#   FUNCTION_VARIABLE_FIXUP(<string|list> <variablename>)


# could be set by a toolchain file
if(NOT MT_EXECUTABLE)
   set(MT_EXECUTABLE mt)
endif()

if(NOT WINDRES_EXECUTABLE)
   set(WINDRES_EXECUTABLE windres)
endif()

macro(MACRO_GENERATE_PUBLIC PUBLIC_INCLUDES SUBDIR)
   foreach(pubheader ${PUBLIC_INCLUDES})
      string(TOLOWER ${pubheader} pubname)
      get_filename_component(pubname ${pubname} NAME)
      set(pubout ${CMAKE_BINARY_DIR}/include/${SUBDIR}/${pubheader})

      if(NOT EXISTS ${pubout})
         # message(STATUS "Writing public: ${pubout}")
         file(WRITE ${pubout} "#include <${pubname}.h>")
      endif()
   endforeach(pubheader)
endmacro()

macro(MACRO_GENERATE_PRIVATE PRIVATE_INCLUDES SUBDIR)
   foreach(privheader ${PRIVATE_INCLUDES})
      get_filename_component(privname ${privheader} NAME)
      set(privout ${CMAKE_BINARY_DIR}/privateinclude/${SUBDIR}/private/${privname})

      # message(STATUS "Writing private: ${privout}")
      configure_file(${privheader} ${privout} COPYONLY)
   endforeach(privheader)
endmacro()

macro(MACRO_GENERATE_MISC MISC_INCLUDES SUBDIR)
   foreach(mischeader ${MISC_INCLUDES})
      get_filename_component(headername ${mischeader} NAME)
      set(headout ${CMAKE_BINARY_DIR}/include/${SUBDIR}/${headername})

      # message(STATUS "Writing: ${headout}")
      configure_file(${mischeader} ${headout} COPYONLY)
   endforeach(mischeader)
endmacro()

macro(MACRO_GENERATE_MISC_PRIVATE MISC_INCLUDES SUBDIR)
   foreach(mischeader ${MISC_INCLUDES})
      get_filename_component(headername ${mischeader} NAME)
      set(headout ${CMAKE_BINARY_DIR}/privateinclude/${SUBDIR}/${headername})

      # message(STATUS "Writing: ${headout}")
      configure_file(${mischeader} ${headout} COPYONLY)
   endforeach(mischeader)
endmacro()

function(FUNCTION_GENERATE_RESOURCES LIB_APP)

   get_target_property(RESOURCE_LIST ${LIB_APP} SOURCES)

   foreach(resource ${RESOURCE_LIST})
      get_filename_component(resource_ext  ${resource} EXT)
      get_filename_component(resource_name ${resource} NAME_WE)

      if(resource_ext STREQUAL ".qrc")
         set(resource_out ${CMAKE_CURRENT_BINARY_DIR}/qrc_${resource_name}.cpp)
         set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/qrc_{$resource_name}.cpp PROPERTIES GENERATED 1)

         add_custom_command(
            OUTPUT ${resource_out}
            COMMAND rcc${TOOLS_SUFFIX} "${resource}" -o "${resource_out}" -name ${resource_name}
            MAIN_DEPENDENCY ${resource}
         )

      elseif(resource_ext STREQUAL ".ts")
         set(resource_out ${CMAKE_CURRENT_SOURCE_DIR}/${resource_name}.qm)

         add_custom_command(
            OUTPUT ${resource_out}
            COMMAND lrelease${TOOLS_SUFFIX} "${resource}" -qm "${resource_out}"
            MAIN_DEPENDENCY ${resource}
         )

      elseif(resource_ext STREQUAL ".ui")
         set(resource_out ${CMAKE_CURRENT_BINARY_DIR}/ui_${resource_name}.h)

         add_custom_command(
            OUTPUT ${resource_out}
            COMMAND uic${TOOLS_SUFFIX} "${resource}" -o "${resource_out}"
            MAIN_DEPENDENCY ${resource}
         )

         set_property(SOURCE ${resource} APPEND PROPERTY OBJECT_DEPENDS ${resource_out})

      endif()

   endforeach()
endfunction()

macro(MACRO_WINDOWS_RESOURCES RESOURCES RSCNAME)

   foreach(resource ${RESOURCES})
      get_filename_component(resource_ext  ${resource} EXT)
      get_filename_component(resource_name ${resource} NAME_WE)

      if(resource_ext MATCHES ".manifest" AND NOT MINGW)
         set(resource_out ${CMAKE_CURRENT_BINARY_DIR}/${resource_name})

         execute_process(
            COMMAND ${MT_EXECUTABLE} -nologo -manifest ${resource} -outputresource:${resource_out}
            RESULT_VARIABLE ${resource_name}_ERROR
         )

         if(NOT ${resource_name}_ERROR EQUAL 0)
            message(SEND_ERROR "running ${MT_EXECUTABLE} on ${resource} failed")
         endif()

         set(${RSCNAME} ${resource_out})

        elseif(resource_ext STREQUAL ".rc" AND MSVC)
            # manifest alternative on Windows host
            set(rscout ${CMAKE_CURRENT_BINARY_DIR}/${resource_name}.res)

            execute_process(
                COMMAND ${WINDRES_EXECUTABLE} --input ${resource} --output ${resource_out}
                RESULT_VARIABLE ${resource_name}_ERROR
            )

            if(NOT ${resource_name}_ERROR EQUAL 0)
                message(SEND_ERROR "running ${WINDRES_EXECUTABLE} on ${resource} failed")
            endif()

            set(${RSCNAME} ${resource_out})

        elseif(resource_ext STREQUAL ".rc")
            # MinGW, manifest alternative on GNU host
            set(resource_out ${CMAKE_CURRENT_BINARY_DIR}/${resource_name}.o)

            execute_process(
                COMMAND ${WINDRES_EXECUTABLE} --input ${resource} --output ${resource_out}
                RESULT_VARIABLE ${resource_name}_ERROR
            )

            if(NOT ${resource_name}_ERROR EQUAL 0)
               message(SEND_ERROR "running ${WINDRES_EXECUTABLE} on ${resource} failed")
            endif()

            set(${RSCNAME} ${resource_out})
        endif()

   endforeach()
endmacro()

macro(MACRO_GENERATE_PACKAGE PC_NAME PC_REALNAME PC_CFLAGS PC_REQUIRES)
   if(UNIX)
      # set again, otherwise the behaviour is undefined
      set(PC_NAME ${PC_NAME})
      set(PC_REALNAME ${PC_REALNAME})
      set(PC_CFLAGS ${PC_CFLAGS})
      set(PC_REQUIRES ${PC_REQUIRES})

      configure_file(
         ${CMAKE_SOURCE_DIR}/cmake/pkgconfig.cmake
         ${CMAKE_BINARY_DIR}/pkgconfig/${PC_NAME}.pc
         @ONLY
      )

      install(
         FILES ${CMAKE_BINARY_DIR}/pkgconfig/${PC_NAME}.pc
         DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
         COMPONENT Devel
      )

   endif()
endmacro()

function(FUNCTION_VARIABLE_FIXUP INSTR OUTSTR)
   if("${INSTR}" STREQUAL "")
      set(${OUTSTR} " " PARENT_SCOPE)
   else()
      string(REPLACE ";" " " modstring "${INSTR}")
      set(${OUTSTR} "${modstring}" PARENT_SCOPE)
   endif()
endfunction()
