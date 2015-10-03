# - Try to find the CopperSpice Toolkit
#
# Once done this will define:
#
#  COPPERSPICE_FOUND - system has CopperSpice
#  COPPERSPICE_INCLUDES - the CopperSpice include directories
#  COPPERSPICE_LIBRARIES - the libraries needed to use CopperSpice
#
#  COPPERSPICE_<COMPONENT>_INCLUDES
#  COPPERSPICE_<COMPONENT>_LIBRARIES
#
#  COPPERSPICE_UIC_EXECUTABLE
#  COPPERSPICE_RCC_EXECUTABLE
#  COPPERSPICE_LUPDATE_EXECUTABLE
#  COPPERSPICE_LRELEASE_EXECUTABLE
#  COPPERSPICE_LCONVERT_EXECUTABLE
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(COPPERSPICE_INCLUDES AND COPPERSPICE_LIBRARIES)
    set(COPPERSPICE_FIND_QUIETLY TRUE)
endif()

if(CopperSpice_FIND_COMPONENTS)
    set(COPPERSPICECOMPONENTS ${CopperSpice_FIND_COMPONENTS})
else()
    set(COPPERSPICECOMPONENTS @BUILD_COMPONENTS@)
endif()
set(COPPERSPICETOOLS uic rcc lupdate lrelease lconvert)

set(COPPERSPICE_FOUND TRUE)
set(COPPERSPICE_INCLUDES)
set(COPPERSPICE_LIBRARIES)

foreach(tool ${COPPERSPICETOOLS})
    string(TOUPPER ${tool} uppertool)
    find_program(COPPERSPICE_${uppertool}_EXECUTABLE
        NAMES
        ${tool}@TOOLS_SUFFIX@
        HINTS
        /bin
        /usr/bin
        /usr/local/bin
        $ENV{CSDIR}/bin
    )
endforeach()

foreach(component ${COPPERSPICECOMPONENTS})
    string(TOUPPER COPPERSPICE_${component} uppercomp)
    set(component Cs${component})

    if(NOT WIN32)
        find_package(PkgConfig)
        pkg_check_modules(PC_${uppercomp} QUIET ${component})
    endif(NOT WIN32)

    find_path(FIND_${uppercomp}_INCLUDES
        NAMES
        ${component}
        PATH_SUFFIXES ${component}
        HINTS
        /include
        /usr/include
        /usr/local/include
        $ENV{CSDIR}/include
        ${PC_${uppercomp}_INCLUDEDIR}
        ${INCLUDE_INSTALL_DIR}
    )

    find_library(FIND_${uppercomp}_LIBRARIES
        ${component}
        HINTS
        /lib
        /usr/lib
        /usr/local/lib
        $ENV{CSDIR}/lib
        ${PC_${uppercomp}_LIBDIR}
        ${LIB_INSTALL_DIR}
    )

    set(COMPONENT_INCLUDES ${FIND_${uppercomp}_INCLUDES})
    set(COMPONENT_LIBRARIES ${FIND_${uppercomp}_LIBRARIES} ${PC_${uppercomp}_LIBRARIES})
    set(COMPONENT_VERSION ${PC_${uppercomp}_VERSION})
    if(NOT COMPONENT_VERSION)
        set(COMPONENT_VERSION "unknown")
    endif()
    if(NOT "${COMPONENT_INCLUDES}" STREQUAL "${uppercomp}_INCLUDES-NOTFOUND"
        AND NOT "${COMPONENT_LIBRARIES}" STREQUAL "${uppercomp}_LIBRARIES-NOTFOUND")
        message(STATUS "Found ${component}, version ${COMPONENT_VERSION}")
        set(${uppercomp}_FOUND TRUE)
        get_filename_component(parentinclude ${COMPONENT_INCLUDES} DIRECTORY) 
        set(COPPERSPICE_INCLUDES ${COPPERSPICE_INCLUDES} ${COMPONENT_INCLUDES} ${parentinclude})
        set(COPPERSPICE_LIBRARIES ${COPPERSPICE_LIBRARIES} ${COMPONENT_LIBRARIES})
        set(${uppercomp}_INCLUDES ${COMPONENT_INCLUDES})
        set(${uppercomp}_LIBRARIES ${COMPONENT_LIBRARIES})
    else()
        message(STATUS "Could not find: ${component}")
        set(${uppercomp}_FOUND FALSE)
        set(COPPERSPICE_FOUND FALSE)
    endif()
endforeach()

include(CopperSpiceMacros)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CopperSpice DEFAULT_MSG COPPERSPICE_INCLUDES COPPERSPICE_LIBRARIES)

mark_as_advanced(COPPERSPICE_INCLUDES COPPERSPICE_LIBRARIES)
