# This module defines the following macros:
#   MACRO_GENERATE_PUBLIC()
#   MACRO_GENERATE_PRIVATE()
#   MACRO_GENERATE_MISC()
#   MACRO_GENERATE_RESOURCES()
#
# Usage:
#   MACRO_GENERATE_PUBLIC(<FancyHeaderName> [<FancyHeaderName2>] ... <subdir>)
#
#   MACRO_GENERATE_PRIVATE(<someheader.h> [<someheader2.h>] ... <subdir>)
#
#   MACRO_GENERATE_MISC(<someheader.h> [<someheader2.h>] ... <subdir>)
#
#   MACRO_GENERATE_RESOURCES(<someui.ui> [<someqrc.qrc>] ...)
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

macro(MACRO_GENERATE_PUBLIC PUBLIC_INCLUDES SUBDIR)
    foreach(pubheader ${PUBLIC_INCLUDES})
        string(TOLOWER ${pubheader} pubname)
        get_filename_component(pubname ${pubname} NAME)
        set(pubout ${CMAKE_BINARY_DIR}/include/${SUBDIR}/${pubheader})
        # message(STATUS "Writing public: ${pubout}")
        file(WRITE ${pubout} "#include <${pubname}.h>")
    endforeach(pubheader)
endmacro()

macro(MACRO_GENERATE_PRIVATE PRIVATE_INCLUDES SUBDIR)
    foreach(privheader ${PRIVATE_INCLUDES})
        get_filename_component(privname ${privheader} NAME)
        set(privout ${CMAKE_BINARY_DIR}/privateinclude/${SUBDIR}/${privname})
        # message(STATUS "Writing private: ${privout}")
        file(COPY ${privheader} DESTINATION ${CMAKE_BINARY_DIR}/privateinclude/${SUBDIR}/)
    endforeach(privheader)
endmacro()

macro(MACRO_GENERATE_MISC MISC_INCLUDES SUBDIR)
    foreach(header ${MISC_INCLUDES})
        get_filename_component(headername ${header} NAME)
        set(headout ${CMAKE_BINARY_DIR}/include/${SUBDIR}/${headername})
        # message(STATUS "Writing: ${headout}")
        file(COPY ${header} DESTINATION ${CMAKE_BINARY_DIR}/include/${SUBDIR}/)
    endforeach(header)
endmacro()

macro(MACRO_GENERATE_RESOURCES RESOURCES TARGET)
    foreach(resource ${RESOURCES})
        get_filename_component(rscext ${resource} EXT)
        get_filename_component(rscname ${resource} NAME_WE)
        if(${rscext} STREQUAL ".ui")
            set(rscout ${CMAKE_BINARY_DIR}/include/ui_${rscname}.h)
            add_custom_command(
                TARGET ${TARGET}
                PRE_BUILD
                COMMAND "uic" "${resource}" "-o" "${rscout}"
                COMMENT "Writing UI: ${rscout}"
                )
        elseif(${rscext} STREQUAL ".qrc")
            set(rscout ${CMAKE_BINARY_DIR}/include/qrc_${rscname}.cpp)
            add_custom_command(
                TARGET ${TARGET}
                PRE_BUILD
                COMMAND "rcc" "${resource}" "-o" "${rscout}"
                COMMENT "Writing QRC: ${rscout}"
            )
            # FIXME: add rscout to target sources
        endif()
    endforeach()
endmacro()