#  Copyright (C) 2012-2019 Barbara Geller
#  Copyright (C) 2012-2019 Ansel Sermersheim
#
# Copyright (c) 2008, Matthias Kretz, <kretz@kde.org>
# Copyright (c) 2009, Marcus Hufgard, <Marcus.Hufgard@hufgard.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
#
# PULSEAUDIO_FOUND            - system has the PulseAudio library
# PULSEAUDIO_INCLUDE_DIR      - the PulseAudio include directory
# PULSEAUDIO_LIBRARY          - the libraries needed to use PulseAudio
# PULSEAUDIO_MAINLOOP_LIBRARY - the libraries needed to use PulsAudio Mainloop

# minimum version
if(NOT PulseAudio_FIND_VERSION)
  set(PulseAudio_FIND_VERSION "0.9.9")
endif()

if (NOT WIN32)
   include(FindPkgConfig)
   pkg_check_modules(PC_PULSEAUDIO QUIET libpulse>=${PulseAudio_FIND_VERSION})
   pkg_check_modules(PC_PULSEAUDIO_MAINLOOP QUIET libpulse-mainloop-glib)
endif()

find_path(PULSEAUDIO_INCLUDE_DIR pulse/pulseaudio.h
   HINTS
   ${PC_PULSEAUDIO_INCLUDEDIR}
   ${PC_PULSEAUDIO_INCLUDE_DIRS}
)

find_library(PULSEAUDIO_LIBRARY NAMES pulse libpulse
   HINTS
   ${PC_PULSEAUDIO_LIBDIR}
   ${PC_PULSEAUDIO_LIBRARY_DIRS}
   )

find_library(PULSEAUDIO_MAINLOOP_LIBRARY NAMES pulse-mainloop pulse-mainloop-glib libpulse-mainloop-glib
   HINTS
   ${PC_PULSEAUDIO_LIBDIR}
   ${PC_PULSEAUDIO_LIBRARY_DIRS}
)

# Store the version number in the cache, so we don't have to search every time again:
if (PULSEAUDIO_INCLUDE_DIR  AND NOT  PULSEAUDIO_VERSION)

   # get PulseAudio's version from its version.h, and compare it with our minimum version
   file(STRINGS "${PULSEAUDIO_INCLUDE_DIR}/pulse/version.h" pulse_version_h
        REGEX ".*pa_get_headers_version\\(\\).*")

   string(REGEX REPLACE ".*pa_get_headers_version\\(\\)\ \\(\"([0-9]+\\.[0-9]+\\.[0-9]+)[^\"]*\"\\).*" "\\1"
                         _PULSEAUDIO_VERSION "${pulse_version_h}")

   set(PULSEAUDIO_VERSION "${_PULSEAUDIO_VERSION}" CACHE STRING "Version number of PulseAudio" FORCE)
endif()

# Use the new extended syntax of find_package_handle_standard_args(), which also handles version checking:
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PulseAudio REQUIRED_VARS PULSEAUDIO_LIBRARY PULSEAUDIO_INCLUDE_DIR
                                             VERSION_VAR PULSEAUDIO_VERSION )

mark_as_advanced(PULSEAUDIO_INCLUDE_DIR PULSEAUDIO_LIBRARY PULSEAUDIO_MAINLOOP_LIBRARY)
