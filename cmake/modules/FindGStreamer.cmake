#  Copyright (C) 2012-2019 Barbara Geller
#  Copyright (C) 2012-2019 Ansel Sermersheim
#
#  Find GStreamer, will define
#
#  GSTREAMER_FOUND        - system has GStreamer
#  GSTREAMER_INCLUDE_DIRS - the GStreamer include directories
#  GSTREAMER_LIBRARY      - the libraries needed to use GStreamer
#  GSTREAMER_DEFINITIONS  - Compiler switches required for using GStreamer
#
#  For plugin libraries specified in the COMPONENTS of find_package, this module will define:
#
#  GSTREAMER_<plugin_lib>_LIBRARY_FOUND - system has <plugin_lib>
#  GSTREAMER_<plugin_lib>_INCLUDE_DIRS  - the <plugin_lib> include directory
#  GSTREAMER_<plugin_lib>_LIBRARY       - the <plugin_lib> library

find_package(PkgConfig)

# Helper macro to find a GStreamer plugin (or GStreamer itself)
#   _component_prefix is prepended to the _INCLUDE_DIRS and _LIBRARIES variables (eg. "GSTREAMER_AUDIO")
#   _pkgconfig_name is the component's pkg-config name (eg. "gstreamer-1.0", or "gstreamer-video-1.0").
#   _library is the component's library name (eg. "gstreamer-1.0" or "gstvideo-1.0")
macro(FIND_GSTREAMER_COMPONENT _component_prefix _pkgconfig_name _library)
    pkg_check_modules(PKG_${_component_prefix} ${_pkgconfig_name})
    set(${_component_prefix}_INCLUDE_DIRS ${PKG_${_component_prefix}_INCLUDE_DIRS})

    find_library(${_component_prefix}_LIBRARIES
        NAMES ${_library}
        HINTS ${PKG_${_component_prefix}_LIBRARY_DIRS} ${PKG_${_component_prefix}_LIBDIR}
    )
endmacro()

# find headers and libraries
FIND_GSTREAMER_COMPONENT(GSTREAMER gstreamer-1.0 gstreamer-1.0)
FIND_GSTREAMER_COMPONENT(GSTREAMER_BASE gstreamer-base-1.0 gstbase-1.0)

if (DEFINED GSTREAMER_LIBRARIES)
   set(GSTREAMER_ABI_VERSION "1.0")

else()
   FIND_GSTREAMER_COMPONENT(GSTREAMER gstreamer-0.10 gstreamer-0.10)
   FIND_GSTREAMER_COMPONENT(GSTREAMER_BASE gstreamer-base-0.10 gstbase-0.10)

   if (DEFINED GSTREAMER_LIBRARIES)
      set(GSTREAMER_ABI_VERSION "0.10")
   else()
      # nothing was found
      return()
   endif()
endif()


# find plugins
FIND_GSTREAMER_COMPONENT(GSTREAMER_APP   gstreamer-app-${GSTREAMER_ABI_VERSION}   gstapp-${GSTREAMER_ABI_VERSION})
FIND_GSTREAMER_COMPONENT(GSTREAMER_AUDIO gstreamer-audio-${GSTREAMER_ABI_VERSION} gstaudio-${GSTREAMER_ABI_VERSION})
FIND_GSTREAMER_COMPONENT(GSTREAMER_VIDEO gstreamer-video-${GSTREAMER_ABI_VERSION} gstvideo-${GSTREAMER_ABI_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GStreamer
   DEFAULT_MSG
   GSTREAMER_LIBRARIES
   GSTREAMER_INCLUDE_DIRS
)
