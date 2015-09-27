# - Config file for the CopperSpice package
#
# It defines the following variables:
#
#  COPPERSPICE_INCLUDES - include directories for CopperSpice
#  COPPERSPICE_LIBRARIES    - libraries to link against

set(COPPERSPICE_FOUND true)

# Compute paths
get_filename_component(COPPERSPICE_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
get_filename_component(COPPERSPICE_PREFIX ${COPPERSPICE_CMAKE_DIR}/ ABSOLUTE)

# Our library and binary dependencies (contains definitions for IMPORTED targets)
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceLibraryTargets.cmake")
include("${COPPERSPICE_CMAKE_DIR}/CopperSpiceBinaryTargets.cmake")

# These are IMPORTED targets
set(COPPERSPICE_INCLUDES "@CMAKE_INSTALL_FULL_INCLUDEDIR@")
set(COPPERSPICE_LIBRARIES)
# TODO: add DBus, Declarative and ScriptTools once they build
set(COPPERSPICE_COMPONENTS Core Gui Network OpenGL Sql Svg Xml XmlPatterns Script WebKit)
foreach(component ${COPPERSPICE_COMPONENTS})
    string(TOUPPER ${component} uppercomp)
    set(COPPERSPICE_INCLUDES
        ${COPPERSPICE_INCLUDES}
        @CMAKE_INSTALL_FULL_INCLUDEDIR@/Qt${component}
    )
    set(COPPERSPICE_${uppercomp}_INCLUDES @CMAKE_INSTALL_FULL_INCLUDEDIR@/Qt${component})

    set(COPPERSPICE_LIBRARIES
        ${COPPERSPICE_LIBRARIES}
        Cs${component}@BUILD_MAJOR@
    )
    set(COPPERSPICE_${uppercomp}_LIBRARIES Cs${component}@BUILD_MAJOR@)
endforeach()
