#
# Copyright (C) 2012-2018 Barbara Geller
# Copyright (C) 2012-2018 Ansel Sermersheim
# All rights reserved.    
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

# This module defines the following macros:
#
#   MACRO_GENERATE_PUBLIC()
#   MACRO_GENERATE_PRIVATE()
#   MACRO_GENERATE_MISC()
#   MACRO_GENERATE_RESOURCES()
#   MACRO_WINDOWS_RESOURCES()
#   MACRO_GENERATE_PACKAGE()
#   FUNCTION_VARIABLE_FIXUP()
#
# Usage:
#   MACRO_GENERATE_PUBLIC(<FancyHeaderName> [<FancyHeaderName2>] ... <component>)
#
#   MACRO_GENERATE_PRIVATE(<header.h> [<header2.h>] ... <component>)
#
#   MACRO_GENERATE_MISC(<header.h> [<header2.h>] ... <component>)
#
#   MACRO_GENERATE_RESOURCES(<userinterface.ui> [<resource.qrc>] ...)
#
#   MACRO_WINDOWS_RESOURCES(<windowsmanifest.manifest> [<windowsresource.rc>] ...)
#
#   MACRO_GENERATE_PACKAGE(<name> <realname> <cxxflags> <libraries> <requires>)
#
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

macro(MACRO_GENERATE_RESOURCES RESOURCES)
    foreach(resource ${RESOURCES} ${ARGN})
        get_filename_component(rscext ${resource} EXT)
        get_filename_component(rscname ${resource} NAME_WE)
        if(${rscext} STREQUAL ".ui")
            set(rscout ${CMAKE_CURRENT_BINARY_DIR}/ui_${rscname}.h)
            add_custom_command(
                OUTPUT ${rscout}
                COMMAND uic${TOOLS_SUFFIX} "${resource}" -o "${rscout}"
                MAIN_DEPENDENCY "${resource}"
            )
        elseif(${rscext} STREQUAL ".qrc")
            set(rscout ${CMAKE_CURRENT_BINARY_DIR}/qrc_${rscname}.cpp)
	    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/qrc_{$rscname}.cpp PROPERTIES GENERATED 1)
            add_custom_command(
                OUTPUT ${rscout}
                COMMAND rcc${TOOLS_SUFFIX} "${resource}" -o "${rscout}" -name ${rscname}
                MAIN_DEPENDENCY ${resource}
            )
            set_property(SOURCE ${resource} APPEND PROPERTY OBJECT_DEPENDS ${rscout})
        endif()
    endforeach()
endmacro()

macro(MACRO_WINDOWS_RESOURCES RESOURCES RSCNAME)
    foreach(resource ${RESOURCES})
        get_filename_component(rscext ${resource} EXT)
        get_filename_component(rscname ${resource} NAME_WE)
        if(${rscext} MATCHES ".manifest" AND NOT MINGW)
            set(rscout ${CMAKE_CURRENT_BINARY_DIR}/${rscname})
            execute_process(
                COMMAND ${MT_EXECUTABLE} -nologo -manifest ${resource} -outputresource:${rscout}
                RESULT_VARIABLE ${rscname}_ERROR
            )
            if(NOT ${rscname}_ERROR EQUAL 0)
                message(SEND_ERROR "running ${MT_EXECUTABLE} on ${resource} failed")
            endif()
            set(${RSCNAME} ${rscout})
        elseif(${rscext} STREQUAL ".rc" AND MSVC)
            # MinGW, manifest alternative on Windows host
            set(rscout ${CMAKE_CURRENT_BINARY_DIR}/${rscname}.res)
            execute_process(
                COMMAND ${WINDRES_EXECUTABLE} --input ${resource} --output ${rscout}
                RESULT_VARIABLE ${rscname}_ERROR
            )
            if(NOT ${rscname}_ERROR EQUAL 0)
                message(SEND_ERROR "running ${WINDRES_EXECUTABLE} on ${resource} failed")
            endif()
            set(${RSCNAME} ${rscout})
        elseif(${rscext} STREQUAL ".rc")
            # MinGW, manifest alternative on GNU host
            set(rscout ${CMAKE_CURRENT_BINARY_DIR}/${rscname}.o)
            execute_process(
                COMMAND ${WINDRES_EXECUTABLE} --input ${resource} --output ${rscout}
                RESULT_VARIABLE ${rscname}_ERROR
            )
            if(NOT ${rscname}_ERROR EQUAL 0)
                message(SEND_ERROR "running ${WINDRES_EXECUTABLE} on ${resource} failed")
            endif()
            set(${RSCNAME} ${rscout})
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
