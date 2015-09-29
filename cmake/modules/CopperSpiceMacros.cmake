# This module defines the following macros:
#
#   MACRO_GENERATE_RESOURCES()
#
# Usage:
#
#   MACRO_GENERATE_RESOURCES(<userinterface.ui> [<resource.qrc>] ...)
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

macro(MACRO_GENERATE_RESOURCES RESOURCES)
    foreach(resource ${RESOURCES})
        get_filename_component(rscext ${resource} EXT)
        get_filename_component(rscname ${resource} NAME_WE)
        if(${rscext} STREQUAL ".ui")
            set(rscout ${CMAKE_CURRENT_BINARY_DIR}/ui_${rscname}.h)
            add_custom_command(
                OUTPUT ${rscout}
                COMMAND CopperSpice::uic "${resource}" -o "${rscout}"
                MAIN_DEPENDENCY "${resource}"
            )
        elseif(${rscext} STREQUAL ".qrc")
            set(rscout ${CMAKE_CURRENT_BINARY_DIR}/qrc_${rscname}.cpp)
            add_custom_command(
                OUTPUT ${rscout}
                COMMAND CopperSpice::rcc "${resource}" -o "${rscout}" -name ${rscname}
                MAIN_DEPENDENCY ${resource}
            )
            set_property(SOURCE ${resource} APPEND PROPERTY OBJECT_DEPENDS ${rscout})
        endif()
    endforeach()
endmacro()
