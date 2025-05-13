# ***********************************************************************
#
# Copyright (c) 2012-2025 Barbara Geller
# Copyright (c) 2012-2025 Ansel Sermersheim
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

#  GSTREAMER_FOUND        - system has GStreamer
#  GSTREAMER_INCLUDE_DIRS - the GStreamer include directories
#  GSTREAMER_LIBRARY      - the libraries needed to use GStreamer
#
#  For plugin libraries specified in the COMPONENTS of find_package, defines the following
#
#  GSTREAMER_<plugin_lib>_LIBRARY_FOUND - system has <plugin_lib>
#  GSTREAMER_<plugin_lib>_INCLUDE_DIRS  - the <plugin_lib> include directory
#  GSTREAMER_<plugin_lib>_LIBRARY       - the <plugin_lib> library

find_package(PkgConfig)

# Helper macro to find a GStreamer plugin (or GStreamer itself)
#   _component_prefix is prepended to the _INCLUDE_DIRS and _LIBRARIES variables (eg. "GSTREAMER_AUDIO")
#   _pkgconfig_name   is the component pkg-config name (eg. "gstreamer-1.0", or "gstreamer-video-1.0").
#   _header           is the component's header, relative to the gstreamer-1.0 directory (eg. "gst/gst.h")
#   _library          is the component library name (eg. "gstreamer-1.0" or "gstvideo-1.0")

macro(FIND_GSTREAMER_COMPONENT _component_prefix _pkgconfig_name _library _header)
   pkg_check_modules(PKG_${_component_prefix} ${_pkgconfig_name})

	find_path(${_component_prefix}_INCLUDE_DIRS
		NAMES ${_header}
		HINTS ${PKG_${_component_prefix}_INCLUDE_DIRS} ${PKG_${_component_prefix}_INCLUDEDIR}
		PATH_SUFFIXES gstreamer-1.0
	)

   find_library(${_component_prefix}_LIBRARIES
      NAMES ${_library}
      HINTS ${PKG_${_component_prefix}_LIBRARY_DIRS} ${PKG_${_component_prefix}_LIBDIR}
   )
endmacro()

# find headers and libraries
FIND_GSTREAMER_COMPONENT(GSTREAMER gstreamer-1.0 gstreamer-1.0 gst/gst.h)
FIND_GSTREAMER_COMPONENT(GSTREAMER_BASE gstreamer-base-1.0 gstbase-1.0 gst/base/gstadapter.h)

if (GSTREAMER_ABI_VERSION STREQUAL "1.0")
   # already set up

else()
   if (NOT GSTREAMER_LIBRARIES STREQUAL "GSTREAMER_LIBRARIES-NOTFOUND")
      set(GSTREAMER_ABI_VERSION "1.0")

   else()
     # nothing was found
     return()

   endif()
endif()


# find plugins
FIND_GSTREAMER_COMPONENT(GSTREAMER_APP   gstreamer-app-${GSTREAMER_ABI_VERSION}   gstapp-${GSTREAMER_ABI_VERSION}    gst/app/gstappsink.h)
FIND_GSTREAMER_COMPONENT(GSTREAMER_AUDIO gstreamer-audio-${GSTREAMER_ABI_VERSION} gstaudio-${GSTREAMER_ABI_VERSION}  gst/audio/audio.h)
FIND_GSTREAMER_COMPONENT(GSTREAMER_VIDEO gstreamer-video-${GSTREAMER_ABI_VERSION} gstvideo-${GSTREAMER_ABI_VERSION}  gst/video/video.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GStreamer
   DEFAULT_MSG
   GSTREAMER_LIBRARIES
   GSTREAMER_INCLUDE_DIRS
   GSTREAMER_ABI_VERSION
)

set (GSTREAMER_ABI_VERSION "${GSTREAMER_ABI_VERSION}" CACHE INTERNAL "")
mark_as_advanced(GSTREAMER_ABI_VERSION)
