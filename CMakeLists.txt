cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

cmake_policy(VERSION 3.16..3.23)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19.0")
   # allows spaces in ctest names
   cmake_policy(SET CMP0110 NEW)
endif()

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20.0")
   # enable RTTI on MSVC
   cmake_policy(SET CMP0117 OLD)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

project(copperspice)

set(BUILD_MAJOR "1")
set(BUILD_MINOR "9")
set(BUILD_MICRO "1")

# used to control RPATH and cmake/pc variables when building RPM
option(BUILDING_RPM "Are we building RPM" OFF)
option(BUILDING_DEBIAN "Are we building DEBIAN" OFF)

set(BUILD_ABI ${BUILD_MAJOR}.${BUILD_MINOR})

# enable catch2 unit tests in config
option(BUILD_TESTS "Enables building the Catch2 Unit Tests" OFF)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(FeatureSummary)
include(CheckAtomics)
include(CheckLibraryExists)

include(ResourceMacros)
include(TestLargeFiles)



# location for install or package
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   include(GNUInstallDirs)

   # rpath used for rcc and uic when compiling a user application
   file(RELATIVE_PATH CS_BIN_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
                                   "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

   set(CMAKE_INSTALL_RPATH "@executable_path/${CS_BIN_RPATH}")

elseif (CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
   include(GNUInstallDirs)

   # rpath used for rcc and uic when compiling a user application
   file(RELATIVE_PATH CS_BIN_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
                                   "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/copperspice/bin")

   if (BUILDING_RPM OR BUILDING_DEBIAN)
       set(CMAKE_INSTALL_RPATH "$ORIGIN/${CS_BIN_RPATH}:$ORIGIN/..")
   else()
       set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/copperspice/bin:$ORIGIN/${CS_BIN_RPATH}:$ORIGIN/..")
   endif()
   set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include/copperspice")

   message( STATUS, "CMAKE_INSTALL_RPATH   ${CMAKE_INSTALL_RPATH}" )
   message( STATUS, "CS_BIN_RPATH          ${CS_BIN_RPATH}" )
   message( STATUS, "CMAKE_INSTALL_BINDIR  ${CMAKE_INSTALL_BINDIR}" )

elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
   set(CMAKE_INSTALL_BINDIR bin)
   set(CMAKE_INSTALL_LIBDIR lib)
   set(CMAKE_INSTALL_INCLUDEDIR include)

endif()

#
# When building for a package we install to /usr under a 
# fakeroot or buildroot during build. We don't want
# the build install path but the system target path.
#
if (BUILDING_RPM OR BUILDING_DEBIAN)
    message(STATUS "** We are building a package ***")
    #
    # wish I could get around the hard code 
    # 
    set(CS_INST_PREFIX  "/usr")    
    set(CS_INST_LIB     "${CS_INST_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
    set(CS_INST_INCLUDE "${CS_INST_PREFIX}/include/copperspice")
else()
    set(CS_INST_PREFIX  "${CMAKE_INSTALL_PREFIX}")    
    set(CS_INST_LIB     "${CMAKE_INSTALL_FULL_LIBDIR}")
    set(CS_INST_INCLUDE "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
endif()

# Note: webkit2.pc.in also uses prefix
#
set(prefix "${CS_INST_PREFIX}")


message(STATUS "   ***   ")
message(STATUS "   CS_INST_PREFIX     ${CS_INST_PREFIX}")
message(STATUS "   CS_INST_LIB        ${CS_INST_LIB}")
message(STATUS "   CS_INST_INCLUDE    ${CS_INST_INCLUDE}")
message(STATUS "   CMAKE_BINARY_DIR   ${CMAKE_BINARY_DIR}")

# required libraries
set(BUILD_COMPONENTS "Core Xml")

# unused at this time: DBus, Declarative, and ScriptTools
set(CS_OPTIONAL_COMPONENTS Gui Multimedia Network OpenGL Script Sql Svg Vulkan WebKit XmlPatterns)

set(PACKAGE           "copperspice")
set(PACKAGE_NAME      "CopperSpice")
set(PACKAGE_VERSION   "${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_STRING    "copperspice ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_TARNAME   "copperspice")
set(PACKAGE_BUGREPORT "info@copperspice.com")
set(PACKAGE_URL       "https://www.copperspice.com/")

set(RPM_VERSION       "${BUILD_MAJOR}.${BUILD_MINOR}")
set(RPM_RELEASE       "${BUILD_MICRO}")


# values used in cs_build_info.h.in
set(HEX_VERSION "0x0${BUILD_MAJOR}0${BUILD_MINOR}0${BUILD_MICRO}")
set(VERSION     "${PACKAGE_VERSION}")


if (MSVC)
   enable_language(ASM_MASM)
   set(target "MSVC")

elseif (CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang|AppleClang)")
   execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -dumpmachine
      OUTPUT_VARIABLE DUMPMACHINE_OUTPUT
      RESULT_VARIABLE DUMPMACHINE_EXITCODE
      OUTPUT_STRIP_TRAILING_WHITESPACE
   )

   if (NOT DUMPMACHINE_EXITCODE EQUAL 0)
      message(SEND_ERROR "Failed to obtain machine from ${CMAKE_CXX_COMPILER}")
   endif()

   set(target "${DUMPMACHINE_OUTPUT}")

else()
   message(WARNING
      "Your compiler (${CMAKE_CXX_COMPILER}) may not be supported. "
      "The plugin key which will be used is: ${CMAKE_SYSTEM}"
   )

   set(target "${CMAKE_SYSTEM}")
endif()

set(CPACK_PACKAGE_NAME    "LS-CS")
set(CPACK_PACKAGE_VENDOR  "Logikal Solutions")
set(CPACK_PACKAGE_CONTACT "originalseasonedgeek@gmail.com")

set(CPACK_PACKAGE_VERSION_MAJOR ${BUILD_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BUILD_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BUILD_MICRO})

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cross platform C++ GUI libraries")
set(CPACK_PACKAGE_DESCRIPTION_FILE    "${CMAKE_SOURCE_DIR}/README.md")

set(CPACK_PACKAGE_INSTALL_DIRECTORY   ${CMAKE_INSTALL_PREFIX})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY  OFF)

set(CPACK_SOURCE_PACKAGE_FILE_NAME    "${PROJECT_NAME}-${PACKAGE_VERSION}")
set(CPACK_SOURCE_GENERATOR            ZIP TBZ2)

file (READ "description.txt" CPACK_PACKAGE_DESCRIPTION_TEXT)

file(STRINGS cmake/dist_ignore.txt    CPACK_SOURCE_IGNORE_FILES)

include(CPack)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,-undefined,error")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,error")
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-undefined,error")

elseif (CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD|DragonFly)")
   set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-undefined")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ")
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")

elseif (MSVC)
   string (REGEX REPLACE "/W3" "" CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}"  )
   string (REGEX REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

   add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/utf-8>")

else()
   # Linux, Windows (MinGW)

   set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-undefined")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")

   if (CMAKE_SYSTEM_NAME MATCHES "Windows")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
      set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wa,-mbig-obj")
   endif()

endif()

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
add_definitions(-DQT_SHARED)

check_include_file(dlfcn.h       HAVE_DLFCN_H)
check_include_file(dirent.h      HAVE_DIRENT_H)
check_include_file(features.h    HAVE_FEATURES_H)
check_include_file(fcntl.h       HAVE_FCNTL_H)
check_include_file(grp.h         HAVE_GRP_H)
check_include_file(inttypes.h    HAVE_INTTYPES_H)
check_include_file(libpq-fe.h    HAVE_LIBPQ_FE_H)
check_include_file(memory.h      HAVE_MEMORY_H)
check_include_file(pg_config.h   HAVE_PG_CONFIG_H)
check_include_file(pthread.h     HAVE_PTHREAD_H)
check_include_file(pthread_np.h  HAVE_PTHREAD_NP_H)
check_include_file(pwd.h         HAVE_PWD_H)
check_include_file(signal.h      HAVE_SIGNAL_H)
check_include_file(stdint.h      HAVE_STDINT_H)
check_include_file(stdlib.h      HAVE_STDLIB_H)
check_include_file(strings.h     HAVE_STRINGS_H)
check_include_file(string.h      HAVE_STRING_H)
check_include_file(unistd.h      HAVE_UNISTD_H)

check_include_file(cups/cups.h   HAVE_CUPS_CUPS_H)
check_include_file(mysql/mysql.h HAVE_MYSQL_H)
check_include_file(net/if.h      HAVE_NET_IF_H)
check_include_file(netinet/in.h  HAVE_NETINET_IN_H)
check_include_file(sys/eventfd.h HAVE_SYS_EVENTFD_H)
check_include_file(sys/types.h   HAVE_SYS_TYPES_H)
check_include_file(sys/ioctl.h   HAVE_SYS_IOCTL_H)
check_include_file(sys/ipc.h     HAVE_SYS_IPC_H)
check_include_file(sys/time.h    HAVE_SYS_TIME_H)
check_include_file(sys/shm.h     HAVE_SYS_SHM_H)
check_include_file(sys/socket.h  HAVE_SYS_SOCKET_H)
check_include_file(sys/stat.h    HAVE_SYS_STAT_H)
check_include_file(sys/wait.h    HAVE_SYS_WAIT_H)
check_include_file(sys/param.h   HAVE_SYS_PARAM_H)

check_include_files("dlfcn.h;stdint.h;stddef.h;inttypes.h;stdlib.h;strings.h;string.h;float.h" STDC_HEADERS)
check_type_size(size_t SIZEOF_SIZE_T)
opj_test_large_files(HAVE_LARGEFILESUPPORT)

# Raspberry Pi does not have built in atomic support
check_working_cxx_atomics(HAVE_CXX_ATOMICS_WITHOUT_LIB)

if (NOT HAVE_CXX_ATOMICS_WITHOUT_LIB)
   check_library_exists(atomic __atomic_fetch_add_4 "" HAVE_LIBATOMIC)

   if (HAVE_LIBATOMIC)
      # handled in CsCore cmake file
   else()
      message(SEND_ERROR "Unable to configure, missing libatomic")
   endif()
endif()

option(WITH_PSQL_PLUGIN "Build PostgreSQL database plugin (if possible)" ON)

option(WITH_MYSQL_PLUGIN "Build MySQL database plugin (if possible)" ON)

option(WITH_ODBC_PLUGIN "Build ODBC database plugin (if possible)" ON)

foreach(component ${CS_OPTIONAL_COMPONENTS})
   string(TOUPPER ${component} uppercomp)
   option(WITH_${uppercomp} "Build ${component} component" ON)
endforeach()

# start output messages
message("")
message("Configuring CMake for CopperSpice\n\n")
message("-- Searching for required packages\n")

find_package(CsLibGuarded QUIET)

if (CsLibGuarded_FOUND)
   message(STATUS "CsLibGuarded was found \n"
      " * Path is ${CsLibGuarded_INCLUDES}\n")
else()
   message(STATUS "CsLibGuarded was not found, bundled library will be used\n")
endif()

find_package(CsString QUIET)

if (CsString_FOUND)
   message(STATUS "CsString was found \n"
      " * Path is ${CsString_LIBRARIES}\n")
else()
   message(STATUS "CsString was not found, bundled library will be used\n")
endif()

find_package(CsSignal QUIET)

if (CsSignal_FOUND)
   message(STATUS "CsSignal was found \n"
      " * Path is ${CsSignal_LIBRARIES}\n")
else()
   message(STATUS "CsSignal was not found, bundled library will be used\n")
endif()

find_package(OpenSSL 1.0)
set_package_properties(OpenSSL PROPERTIES
   PURPOSE "Required for HTTPS support"
   DESCRIPTION "Support for SSL and TLS"
   URL "https://openssl.org"
   TYPE RECOMMENDED
)

if (OpenSSL_FOUND)
   message(STATUS "OpenSSL was found \n"
      " * Version: ${OPENSSL_VERSION}\n"
      " * Path: ${OPENSSL_CRYPTO_LIBRARY}\n")
endif()

find_package(ZLIB QUIET)
set_package_properties(ZLIB PROPERTIES
   PURPOSE "Required for compression support"
   DESCRIPTION "Compression Library"
   URL "https://zlib.net"
   TYPE RECOMMENDED
)

if (ZLIB_FOUND)
   message(STATUS "ZLIB was found, path is ${ZLIB_LIBRARIES}\n")
else()
   message(STATUS "ZLIB was not found, bundled library will be used\n")
endif()

find_package(JPEG QUIET)
set_package_properties(JPEG PROPERTIES
   PURPOSE "Used for JPEG compression for images"
   DESCRIPTION "JPEG Compression Library"
   URL "https://ijg.org"
   TYPE RECOMMENDED
)

if (NOT JPEG_FOUND)
   message(STATUS "libjpeg was not found, bundled library will be used\n")
endif()


if (NOT CMAKE_SYSTEM_NAME MATCHES "Windows")

find_package(Cups)
set_package_properties(Cups PROPERTIES
   PURPOSE "Required for printing support"
   DESCRIPTION "CUPS is the standards-based open source printing system"
   URL "https://www.cups.org"
   TYPE RECOMMENDED
)

find_package(ALSA)
set_package_properties(ALSA PROPERTIES
   PURPOSE "Required for ALSA audio support"
   DESCRIPTION "Advanced Linux Sound Architecture"
   URL "https://www.alsa-project.org"
   TYPE RECOMMENDED
)
endif()


if (CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")

find_package(PulseAudio)
set_package_properties(PulseAudio PROPERTIES
   PURPOSE "Required for pulseaudio support"
   DESCRIPTION "Sound processing daemon for Unix"
   URL "https://www.pulseaudio.org"
   TYPE RECOMMENDED
)
endif()


find_package(PostgreSQL QUIET)
set_package_properties(PostgreSQL PROPERTIES
   PURPOSE "Required for PostgreSQL database support"
   DESCRIPTION "Popular open source database"
   URL "https://www.postgresql.org"
   TYPE RECOMMENDED
)

if (CMAKE_DISABLE_FIND_PACKAGE_PostgreSQL)
   message(STATUS "PostgreSQL disabled, CopperSpice plugin will not be built\n")

elseif (NOT PostgreSQL_FOUND)
   message(STATUS "PostgreSQL was not found, CopperSpice plugin will not be built\n")

else()
   message(STATUS "PostgreSQL was found, CopperSpice plugin will be built\n")

endif()


if (PostgreSQL_FOUND AND CMAKE_SYSTEM_NAME MATCHES "Windows" AND NOT MSVC)

   foreach (fname IN LISTS PostgreSQL_INCLUDE_DIRS)
      # testing for 'EXISTS', a query for if the file can be read or written

      if (EXISTS "${fname}/pthread.h")
         # Postgres bug

         message(WARNING "")
         message(WARNING "------------------------------------------------------------")
         message(WARNING "WARNING WARNING")
         message(WARNING "The '${fname}' directory contains a file called pthread.h")
         message(WARNING "")
         message(WARNING "A conflict exists between pthread.h and the standard library ")
         message(WARNING "Rename or remove pthread.h in order to build the CsSqlPsql plugin")
         message(WARNING "------------------------------------------------------------")
         message(WARNING "")

         # force to not found
         set(PostgreSQL_FOUND false)

         message(STATUS "PostgreSQL client library was found, CopperSpice plugin will not be built")
         message(STATUS "")
      endif()

   endforeach()
endif()

find_package(MySQL QUIET)
set_package_properties(MySQL PROPERTIES
   PURPOSE "Required for MySQL database support"
   DESCRIPTION "Popular open source database"
   URL "https://www.mysql.com"
   TYPE RECOMMENDED
)

if (CMAKE_DISABLE_FIND_PACKAGE_MySQL)
   message(STATUS "MySQL disabled, CopperSpice plugin will not be built\n")

elseif (NOT MySQL_FOUND)
   message(STATUS "MySQL was not found, CopperSpice plugin will not be built\n")

else()
   message(STATUS "MySQL was found, CopperSpice plugin will be built\n")

endif()

find_package(ODBC QUIET)
set_package_properties(ODBC PROPERTIES
   PURPOSE "Required for ODBC SQL database support"
   DESCRIPTION "ODBC SQL driver implementation"
   TYPE RECOMMENDED
)

if (CMAKE_DISABLE_FIND_PACKAGE_ODBC)
   message(STATUS "ODBC SQL Driver disabled, CopperSpice plugin will not be built\n")

elseif (NOT ODBC_FOUND)
   message(STATUS "ODBC SQL Driver was not found, CopperSpice plugin will not be built\n")

else()
   message(STATUS "ODBC SQL Driver was found, CopperSpice plugin will be built\n")

endif()

find_package(SQLite3 QUIET)
set_package_properties(SQLite3 PROPERTIES
   PURPOSE "Required for SQLite3 database support"
   DESCRIPTION "Popular open source database"
   URL "https://www.sqlite.org"
   TYPE RECOMMENDED
)

if (WITH_VULKAN)
   find_package(Vulkan 1.1)
   set_package_properties(Vulkan PROPERTIES
      PURPOSE "Required for Vulkan support"
      DESCRIPTION "The Vulkan SDK"
      URL "https://www.lunarg.com/"
      TYPE REQUIRED
   )

   if (NOT TARGET Vulkan::Vulkan)
      set(WITH_VULKAN NO)

      if (Vulkan_FOUND)
         message(STATUS "Partial Vulkan installation exists however the SDK was not found, "
            "CsVulkan library will not be built\n")
      else()
         message(STATUS "Vulkan SDK was not found, CsVulkan library will not be built\n")
      endif()

   endif()
endif()

# emerald - temporarily turn off GTK as a workaround, conflicts with freetype version in Harfbuzz
set(CMAKE_DISABLE_FIND_PACKAGE_GTK2 TRUE)

if (NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
   find_package(GTK2)
   set_package_properties(GTK2 PROPERTIES
      PURPOSE "Required for GTK style and application integration support"
      DESCRIPTION "Multi-platform toolkit for creating graphical user interfaces"
      URL "http://www.gtk.org"
      TYPE RECOMMENDED
   )
endif()

#if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
#   find_package(Security)
#   set_package_properties(Security PROPERTIES
#      PURPOSE "Required for HTTPS support"
#      DESCRIPTION "Support for secure network communications using Secure Trasnport for SSL and TLS"
#      URL "https://developer.apple.com/documentation/security/secure_transport"
#      TYPE RECOMMENDED
#   )
#endif()

if (CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
   find_package(Threads)
   set_package_properties(Threads PROPERTIES
      PURPOSE "Required for threading support"
      DESCRIPTION "Platform dependant threading library"
      URL ""
      TYPE REQUIRED
   )

   find_package(GLib2)
   set_package_properties(GLib2 PROPERTIES
      PURPOSE "Required for glib mainloop support"
      DESCRIPTION "GNOME core library"
      URL "https://developer.gnome.org/glib"
      TYPE REQUIRED
   )

   find_package(GObject2)
   set_package_properties(GObject2 PROPERTIES
      PURPOSE "Required for glib mainloop support"
      DESCRIPTION "The object system used for Pango and GTK+"
      URL "https://developer.gnome.org/gobject"
      TYPE REQUIRED
   )

   find_package(Iconv)
   set_package_properties(Iconv PROPERTIES
      PURPOSE "Iconv support"
      DESCRIPTION "For use on systems which do not have Iconv or lack Unicode support"
      URL "http://www.gnu.org/software/libiconv/"
      TYPE REQUIRED
   )

   if (WITH_GUI)
      find_package(X11 COMPONENTS ICE SM Xcursor Xext Xfixes Xi Xinerama Xrandr Xrender)
      set_package_properties(X11 PROPERTIES
         PURPOSE "Required for X11/X.Org integration support"
         DESCRIPTION "Open source implementation of the X Window System"
         URL "https://www.x.org"
         TYPE REQUIRED
      )

      find_package(XCB COMPONENTS xcb xcb-image xcb-icccm xcb-sync xcb-xfixes xcb-shm xcb-randr
                   xcb-shape xcb-keysyms xcb-xinerama xcb-xkb xcb-render xcb-render-util xcb-glx)
      set_package_properties(XCB PROPERTIES
         PURPOSE "Required for XCB/X.Org integration support"
         DESCRIPTION "Open source implementation of the XCB Interface for the X11 Window System"
         URL "https://xcb.freedesktop.org"
         TYPE REQUIRED
      )

      find_package(X11_XCB)
      set_package_properties(X11_XCB PROPERTIES
         PURPOSE "Required for X11 XCB support"
         DESCRIPTION "Integration between X11 and XCB"
         URL "https://xcb.freedesktop.org"
         TYPE REQUIRED
      )

      find_package(XKBCommon)
      set_package_properties(XKBCommon PROPERTIES
         PURPOSE "Required for XKB integration support"
         DESCRIPTION "Keyboard library"
         URL "https://xkbcommon.org"
         TYPE REQUIRED
      )

      find_package(XKBCommon_X11)
      set_package_properties(XKBCommon_X11 PROPERTIES
         PURPOSE "Required for XKB X11 integration support"
         DESCRIPTION "Keyboard library"
         URL "https://xkbcommon.org"
         TYPE REQUIRED
      )

      find_package(Fontconfig)
      set_package_properties(Fontconfig PROPERTIES
         PURPOSE "Required for fonts configuration support"
         DESCRIPTION "Library for configuring and customizing font access"
         URL "http://www.freedesktop.org/wiki/Software/fontconfig/"
         TYPE REQUIRED
      )
   endif()

   if (WITH_MULTIMEDIA)
      find_package(GStreamer)
      set_package_properties(GStreamer PROPERTIES
         PURPOSE "Required for multimedia audio and video support"
         DESCRIPTION "Open source media playback library"
         URL "http://gstreamer.freedesktop.org"
         TYPE REQUIRED
      )
   endif()

   if (WITH_OPENGL)
      find_package(OpenGL)
      set_package_properties(OpenGL PROPERTIES
         PURPOSE "Required for OpenGL support"
         DESCRIPTION "The Mesa 3D Graphics Library"
         URL "http://www.mesa3d.org/"
         TYPE REQUIRED
      )
   endif()

   if (WITH_WEBKIT)
      find_package(LibXml2)
      set_package_properties(LibXml2 PROPERTIES
         PURPOSE "Required for XML support in WebKit"
         DESCRIPTION "XML C parser and toolkit developed for the Gnome project"
         URL "http://www.xmlsoft.org/"
         TYPE REQUIRED
      )
   endif()
endif()

if (WIN32)
   add_definitions(-DUNICODE)
endif()

if (NOT Cups_FOUND)
   add_definitions(-DQT_NO_CUPS)
endif()

if (NOT GLIB2_FOUND)
   add_definitions(-DQT_NO_GLIB)
endif()

if (NOT ZLIB_FOUND)
   include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty/zlib)
endif()

if (HAVE_LARGEFILESUPPORT)
   if (_FILE_OFFSET_BITS)
      add_definitions(-D_FILE_OFFSET_BITS=64)
   endif()

   if (_LARGE_FILES)
      add_definitions(-D_LARGE_FILES)
   endif()

   if (_LARGEFILE_SOURCE)
      add_definitions(-D_LARGEFILE_SOURCE)
   endif()
endif()

configure_file(
   ${CMAKE_SOURCE_DIR}/cmake/cs-config.h.cmake
   ${CMAKE_BINARY_DIR}/include/cs-config.h
)
configure_file(
   ${CMAKE_SOURCE_DIR}/src/core/global/cs_build_info.h.in
   ${CMAKE_BINARY_DIR}/include/QtCore/cs_build_info.h
)

# file locations for building
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Ninja generator does not yet know how to build archives in pieces, so response
# files must be used to deal with very long linker command lines.
set(CMAKE_NINJA_FORCE_RESPONSE_FILE 1)

# check components dependencies
if (WITH_MULTIMEDIA AND NOT (WITH_GUI AND WITH_NETWORK AND WITH_OPENGL))
   message(SEND_ERROR "CsMultimedia component requires CsGui, CsNetwork, and CsOpenGL components")

elseif (WITH_OPENGL AND NOT WITH_GUI)
   message(SEND_ERROR "CsOpenGL component requires CsGui component")

elseif (WITH_SVG AND NOT WITH_GUI)
   message(SEND_ERROR "CsSvg component requires CsGui component")

elseif (WITH_XMLPATTERNS AND NOT WITH_NETWORK)
   message(SEND_ERROR "CsXmlPatterns component requires CsNetwork component")

elseif (WITH_WEBKIT AND NOT (WITH_GUI AND WITH_NETWORK AND WITH_SCRIPT))
   message(SEND_ERROR "CsWebKit component requires CsGui, CsNetwork, and CsScript components")

endif()

add_subdirectory(src/core)
add_subdirectory(src/xml)

foreach(component ${CS_OPTIONAL_COMPONENTS})
   string(TOUPPER ${component} uppercomp)

   if (WITH_${uppercomp})
      string(TOLOWER ${component} lowercomp)
      add_subdirectory(src/${lowercomp})
      set(BUILD_COMPONENTS "${BUILD_COMPONENTS} ${component}")
   endif()
endforeach()

add_subdirectory(src/tools/shared)
add_subdirectory(src/tools/lconvert)
add_subdirectory(src/tools/lrelease)
add_subdirectory(src/tools/lupdate)

add_subdirectory(src/tools/rcc)
add_subdirectory(src/tools/uic)

if (WITH_GUI)
   add_subdirectory(src/tools/linguist)
endif()

# platform plugins
if (WITH_GUI)
   if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
      set(BUILD_PLATFORMS_COCOA_PLUGIN TRUE)
   endif()

   if (CMAKE_SYSTEM_NAME MATCHES "Windows")
      set(BUILD_PLATFORMS_WINDOWS_PLUGIN TRUE)
   endif()

   if (CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
      set(BUILD_PLATFORMS_XCB_PLUGIN TRUE)
   endif()

   add_subdirectory(src/plugins/imageformats)
   add_subdirectory(src/plugins/platforms)
   add_subdirectory(src/plugins/printerdrivers)
endif()

# catch2
if (BUILD_TESTS)
   enable_testing()
   add_subdirectory(test/core)
   add_subdirectory(test/gui)
   add_subdirectory(test/network)
endif()

configure_file(
   ${CMAKE_SOURCE_DIR}/cmake/CopperSpiceConfig.cmake
   ${CMAKE_BINARY_DIR}/CopperSpiceConfig.cmake
   @ONLY
)

configure_file(
   ${CMAKE_SOURCE_DIR}/cmake/CopperSpiceConfigVersion.cmake
   ${CMAKE_BINARY_DIR}/CopperSpiceConfigVersion.cmake
   @ONLY
)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   set(PKG_PREFIX "CopperSpice.framework/Resources")

elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
   set(PKG_PREFIX "cmake/CopperSpice")

else()
   set(PKG_PREFIX "${CMAKE_INSTALL_LIBDIR}/cmake/CopperSpice")

endif()

if(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
    #  Update files needed for Debian
    #  Note: You need to update CPACK_PACKAGE_INSTALLED_SIZE to reflect current installed size
    #
   set(CPACK_PACKAGE_INSTALLED_SIZE "173793313")

   # this architecture value is only used on the control file for a debian package.
   # if the value is bogus or empty on other platforms it simply doesn't matter.
   #
   EXECUTE_PROCESS( COMMAND dpkg --print-architecture  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CPACK_PACKAGE_ARCHITECTURE)


   message( STATUS "Architecture: ${CPACK_PACKAGE_ARCHITECTURE}")


    configure_file(
       ${CMAKE_SOURCE_DIR}/deb_build.etc/control.in
       deb_build.etc/control
    )

    configure_file(
       ${CMAKE_SOURCE_DIR}/copperspice.spec.in
       copperspice.spec
    )

    #  postrm has some heavy syntax - just copy
    configure_file(
       ${CMAKE_SOURCE_DIR}/deb_build.etc/postrm.in
       deb_build.etc/postrm
       COPYONLY
    )

    #  postinst needs to know package name for logging
    configure_file(
       ${CMAKE_SOURCE_DIR}/deb_build.etc/postinst.in
       deb_build.etc/postinst
    )

    #  hopefully we never need to replace values in rules
    configure_file(
       ${CMAKE_SOURCE_DIR}/deb_build.etc/rules.in
       deb_build.etc/rules
       COPYONLY
    )

    configure_file(
        ${CMAKE_SOURCE_DIR}/README.md
        README.md
        COPYONLY
    )

endif()



install(
   FILES
      ${CMAKE_BINARY_DIR}/CopperSpiceConfig.cmake
      ${CMAKE_BINARY_DIR}/CopperSpiceConfigVersion.cmake

      ${CMAKE_SOURCE_DIR}/cmake/CopperSpiceDeploy.cmake
      ${CMAKE_SOURCE_DIR}/cmake/CopperSpiceMacros.cmake
      ${CMAKE_SOURCE_DIR}/cmake/InstallMinGW.cmake
   DESTINATION ${PKG_PREFIX}
)

install(
    FILES
       ${CMAKE_SOURCE_DIR}/license/LGPL_EXCEPTION.txt
       ${CMAKE_SOURCE_DIR}/license/LICENSE.FDL
       ${CMAKE_SOURCE_DIR}/license/LICENSE.LGPL
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/${PROJECT_NAME}/LICENSE
)

install(
   # generate cmake files containing rules about linking with CS libs

   EXPORT CopperSpiceLibraryTargets
   NAMESPACE CopperSpice::
   FILE CopperSpiceLibraryTargets.cmake
   DESTINATION ${PKG_PREFIX}
)

# generate cmake files containing rules about running CS binary tools
install(
   EXPORT CopperSpiceBinaryTargets
   NAMESPACE CopperSpice::
   FILE CopperSpiceBinaryTargets.cmake
   DESTINATION ${PKG_PREFIX}
)

message("\n")
message("-- The following CopperSpice libraries will be built:")
message("\n * ${BUILD_COMPONENTS}\n")

feature_summary(WHAT PACKAGES_FOUND PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
   set(TARGETBITS 32)
else()
   set(TARGETBITS 64)
endif()

message("")
message("CopperSpice configured to run on:  ${CMAKE_SYSTEM_NAME} ${TARGETBITS} bit, ${CMAKE_BUILD_TYPE} Mode")
message("CopperSpice will be built in:      ${CMAKE_BINARY_DIR}")
message("CopperSpice will be installed in:  ${CMAKE_INSTALL_PREFIX}")
message("prefix:                            ${prefix}")
message("\n")
