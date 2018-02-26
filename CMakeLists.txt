cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)

# silence a warning about rpath on OS X
cmake_policy(SET CMP0042 OLD)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

project(copperspice)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(FeatureSummary)

include(CopperSpiceBuildMacros)
include(TestLargeFiles)

# file locations for installing
if(UNIX)
    include(GNUInstallDirs)
else()
    set(CMAKE_INSTALL_BINDIR bin)
    set(CMAKE_INSTALL_LIBDIR lib)
    set(CMAKE_INSTALL_INCLUDEDIR include)
endif()

set(BUILD_MAJOR "1")
set(BUILD_MINOR "4")
set(BUILD_MICRO "5")
set(BUILD_ABI ${BUILD_MAJOR}.${BUILD_MINOR})

# required
set(BUILD_COMPONENTS "Core")

# may add DBus, Declarative and ScriptTools
set(_OPTIONAL_COMPONENTS Gui Multimedia Network OpenGL Phonon Script Sql Svg WebKit Xml XmlPatterns)

set(PACKAGE "copperspice")
set(PACKAGE_NAME "copperspice")
set(PACKAGE_VERSION "${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_STRING "copperspice ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_TARNAME "copperspice")
set(PACKAGE_BUGREPORT "info@copperspice.com")
set(PACKAGE_URL "http://www.copperspice.com/")

# used in cs_build_info.h.in
set(HEX_VERSION "0x0${BUILD_MAJOR}0${BUILD_MINOR}0${BUILD_MICRO}")
set(VERSION "${PACKAGE_VERSION}")

# used in cs_build_info.h.in
set(prefix "${CMAKE_INSTALL_PREFIX}")

if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang|AppleClang)")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpmachine
        OUTPUT_VARIABLE DUMPMACHINE_OUTPUT
        RESULT_VARIABLE DUMPMACHINE_EXITCODE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT DUMPMACHINE_EXITCODE EQUAL 0)
        message(SEND_ERROR "Failed to obtain machine from ${CMAKE_CXX_COMPILER}")
    endif()
    set(target "${DUMPMACHINE_OUTPUT}")
else()
    message(WARNING
        "Your compiler (${CMAKE_CXX_COMPILER}) may not be supported. "
        "The plugin key which will be used is: ${CMAKE_SYSTEM}"
    )
    # more compiler handlers may be required, review CMAKE_LANG_COMPILER_ID
    set(target "${CMAKE_SYSTEM}")
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "CopperSpice")
set(CPACK_PACKAGE_VERSION_MAJOR ${BUILD_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BUILD_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BUILD_MICRO})

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ GUI library")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")

set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git;${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CMAKE_INSTALL_PREFIX})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

include(CPack)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-undefined,error")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,error")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-undefined,error")
else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
endif()

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
add_definitions(-DQT_SHARED -DHAVE_CONFIG_H -DBUILD_DATE="${BUILD_DATE}")
set(TOOLS_SUFFIX)

check_include_file(cups/cups.h HAVE_CUPS_CUPS_H)
check_include_file(features.h HAVE_FEATURES_H)
check_include_file(pthread.h HAVE_PTHREAD_H)
check_include_file(pthread_np.h HAVE_PTHREAD_NP_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(dirent.h HAVE_DIRENT_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(grp.h HAVE_GRP_H)
check_include_file(pwd.h HAVE_PWD_H)
check_include_file(signal.h HAVE_SIGNAL_H)
check_include_file(dlfcn.h HAVE_DLFCN_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_file(sys/ipc.h HAVE_SYS_IPC_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(sys/shm.h HAVE_SYS_SHM_H)
check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/wait.h HAVE_SYS_WAIT_H)
check_include_file(sys/param.h HAVE_SYS_PARAM_H)
check_include_file(netinet/in.h HAVE_NETINET_IN_H)
check_include_file(net/if.h HAVE_NET_IF_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(libpq-fe.h HAVE_LIBPQ_FE_H)
check_include_file(pg_config.h HAVE_PG_CONFIG_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_include_file(mysql/mysql.h HAVE_MYSQL_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_files("dlfcn.h;stdint.h;stddef.h;inttypes.h;stdlib.h;strings.h;string.h;float.h" STDC_HEADERS)
check_type_size(size_t SIZEOF_SIZE_T)
opj_test_large_files(HAVE_LARGEFILESUPPORT)

option(WITH_PSQL_PLUGIN "Build PostgreSQL database plugin (if possible)" ON)
add_feature_info(PostgreSQL WITH_PSQL_PLUGIN "Popular open source database")

option(WITH_MYSQL_PLUGIN "Build MySQL database plugin (if possible)" ON)
add_feature_info(MySQL WITH_MYSQL_PLUGIN "Popular open source database")

foreach(component ${_OPTIONAL_COMPONENTS})
    string(TOUPPER ${component} uppercomp)
    option(WITH_${uppercomp} "Build ${component} component" ON)
endforeach()

message("")
message(STATUS "Search for packages:\n")

find_package(OpenSSL 1.0)
set_package_properties(OpenSSL PROPERTIES
    PURPOSE "Required for HTTPS support"
    DESCRIPTION "Support for secure network communications using OpenSSL for SSL and TLS"
    URL "http://openssl.org"
    TYPE RECOMMENDED
)

find_package(Security)
set_package_properties(Security PROPERTIES
    PURPOSE "Required for HTTPS support"
    DESCRIPTION "Support for secure network communications using Secure Trasnport for SSL and TLS"
    URL "https://developer.apple.com/documentation/security/secure_transport"
    TYPE RECOMMENDED
)

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    PURPOSE "Required for compression support"
    DESCRIPTION "A compression Library"
    URL "http://zlib.net"
    TYPE RECOMMENDED
)

find_package(Cups)
set_package_properties(Cups PROPERTIES
    PURPOSE "Required for printing support"
    DESCRIPTION "CUPS is the standards-based open source printing system"
    URL "http://www.cups.org"
    TYPE RECOMMENDED
)

find_package(ALSA)
set_package_properties(ALSA PROPERTIES
    PURPOSE "Required for ALSA audio support"
    DESCRIPTION "Advanced Linux Sound Architecture"
    URL "http://www.alsa-project.org"
    TYPE RECOMMENDED
)

find_package(PostgreSQL)
set_package_properties(PostgreSQL PROPERTIES
    PURPOSE "Required for PostgreSQL database support"
    DESCRIPTION "Popular open source database"
    URL "http://www.postgresql.org"
    TYPE RECOMMENDED
)

find_package(MySQL)
set_package_properties(MySQL PROPERTIES
    PURPOSE "Required for MySQL database support"
    DESCRIPTION "Popular open source database"
    URL "http://www.mysql.com"
    TYPE RECOMMENDED
)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_package(GTK2)
    set_package_properties(GTK2 PROPERTIES
        PURPOSE "Required for GTK style and application integration support"
        DESCRIPTION "Multi-platform toolkit for creating graphical user interfaces"
        URL "http://www.gtk.org"
        TYPE RECOMMENDED
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
    # required by core component
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
        DESCRIPTION "Core application building blocks for libraries and applications written in C"
        URL "https://developer.gnome.org/glib"
        TYPE REQUIRED
    )

    find_package(GOBJECT2)
    set_package_properties(GOBJECT2 PROPERTIES
        PURPOSE "Required for glib mainloop support"
        DESCRIPTION "The object system used for Pango and GTK+"
        URL "https://developer.gnome.org/gobject"
        TYPE REQUIRED
    )

    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        find_package(Iconv)
        set_package_properties(Iconv PROPERTIES
            PURPOSE "Iconv support"
            DESCRIPTION "iconv() implementation, for use on systems which do not have one or lack Unicode support"
            URL "http://www.gnu.org/software/libiconv/"
            TYPE REQUIRED
        )
    endif()

    if(WITH_GUI)
        find_package(NAS)
        set_package_properties(NAS PROPERTIES
            PURPOSE "Required for basic audio support"
            DESCRIPTION "Network transparent, client/server audio transport system"
            URL "http://www.radscan.com/nas.html"
            TYPE REQUIRED
        )

        find_package(X11 COMPONENTS ICE SM Xcursor Xext Xfixes Xi Xinerama Xrandr Xrender X11)
        set_package_properties(X11 PROPERTIES
            PURPOSE "Required for X11/X.Org integration support"
            DESCRIPTION "Open source implementation of the X Window System"
            URL "http://www.x.org"
            TYPE REQUIRED
        )

        find_package(Fontconfig)
        set_package_properties(Fontconfig PROPERTIES
            PURPOSE "Required for fonts configuration support"
            DESCRIPTION "Library for configuring and customizing font access"
            URL "http://www.freedesktop.org/wiki/Software/fontconfig/"
            TYPE REQUIRED
        )

        find_package(Freetype)
        set_package_properties(Freetype PROPERTIES
            PURPOSE "Required for fonts configuration support"
            DESCRIPTION "Freely available software library to render fonts"
            URL "http://www.freetype.org"
            TYPE REQUIRED
        )
    endif()

    if(WITH_OPENGL)
        find_package(OpenGL)
        set_package_properties(OpenGL PROPERTIES
            PURPOSE "Required for OpenGL support"
            DESCRIPTION "The Mesa 3D Graphics Library"
            URL "http://www.mesa3d.org/"
            TYPE REQUIRED
        )
    endif()

    if(WITH_MULTIMEDIA)
        find_package(GStreamer)
        set_package_properties(GStreamer PROPERTIES
            PURPOSE "Required for multimedia support"
            DESCRIPTION "Open source multimedia framework"
            URL "http://gstreamer.freedesktop.org"
            TYPE REQUIRED
        )
    endif()

    if(WITH_WEBKIT)
        find_package(LibXml2)
        set_package_properties(LibXml2 PROPERTIES
            PURPOSE "Required for XML support in WebKit"
            DESCRIPTION "XML C parser and toolkit developed for the Gnome project"
            URL "http://www.xmlsoft.org/"
            TYPE REQUIRED
        )
    endif()
endif()

if(WIN32)
    add_definitions(-DUNICODE)
endif()
if(NOT CUPS_FOUND)
    add_definitions(-DQT_NO_CUPS)
endif()
if(NOT GLIB2_FOUND)
    add_definitions(-DQT_NO_GLIB)
endif()
if(NOT FREETYPE_FOUND)
    add_definitions(-DQT_NO_FREETYPE)
endif()
if(NOT ZLIB_FOUND)
    include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty/zlib)
endif()
if(HAVE_LARGEFILESUPPORT)
    if(_FILE_OFFSET_BITS)
        add_definitions(-D_FILE_OFFSET_BITS=64)
    endif()
    if(_LARGE_FILES)
        add_definitions(-D_LARGE_FILES)
    endif()
    if(_LARGEFILE_SOURCE)
        add_definitions(-D_LARGEFILE_SOURCE)
    endif()
endif()

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/qt-acconfig.h.cmake
    ${CMAKE_BINARY_DIR}/include/qt-acconfig.h
)
configure_file(
    ${CMAKE_SOURCE_DIR}/src/core/global/cs_build_info.h.in
    ${CMAKE_BINARY_DIR}/include/QtCore/cs_build_info.h
)

# file locations for building
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# The Ninja generator does not yet know how to build archives in pieces, and so response
# files must be used to deal with very long linker command lines.
# See https://bugs.webkit.org/show_bug.cgi?id=129771
set(CMAKE_NINJA_FORCE_RESPONSE_FILE 1)

# check components dependencies
# TODO: revisit once _OPTIONAL_COMPONENTS has changed
if(WITH_MULTIMEDIA AND NOT WITH_GUI)
    message(SEND_ERROR "Multimedia component requires Gui component")
elseif(WITH_PHONON AND NOT (WITH_GUI AND WITH_NETWORK))
    message(SEND_ERROR "Phonon component requires Gui and Network components")
elseif(WITH_OPENGL AND NOT WITH_GUI)
    message(SEND_ERROR "OpenGL component requires Gui component")
elseif(WITH_SVG AND NOT WITH_GUI)
    message(SEND_ERROR "Svg component requires Gui component")
elseif(WITH_XMLPATTERNS AND NOT WITH_NETWORK)
    message(SEND_ERROR "XmlPatterns component requires Network component")
elseif(WITH_WEBKIT AND NOT (WITH_GUI AND WITH_NETWORK AND WITH_SCRIPT))
    message(SEND_ERROR "WebKit component requires Gui, Network and Script components")
endif()

add_subdirectory(src/core)
foreach(component ${_OPTIONAL_COMPONENTS})
    string(TOUPPER ${component} uppercomp)
    if(WITH_${uppercomp})
        string(TOLOWER ${component} lowercomp)
        add_subdirectory(src/${lowercomp})
        set(BUILD_COMPONENTS "${BUILD_COMPONENTS} ${component}")
    endif()
endforeach()
if(WITH_XML)
    add_subdirectory(src/tools/shared)
    add_subdirectory(src/tools/lconvert)
    add_subdirectory(src/tools/lrelease)
    add_subdirectory(src/tools/lupdate)
endif()
if(WITH_XML AND WITH_GUI)
    add_subdirectory(src/tools/rcc)
    add_subdirectory(src/tools/uic)
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

install(
   FILES
      ${CMAKE_BINARY_DIR}/CopperSpiceConfig.cmake
      ${CMAKE_BINARY_DIR}/CopperSpiceConfigVersion.cmake
      ${CMAKE_SOURCE_DIR}/cmake/CopperSpiceMacros.cmake
      ${CMAKE_SOURCE_DIR}/cmake/InstallMinGW.cmake
   DESTINATION cmake/CopperSpice
)

install(EXPORT CopperSpiceLibraryTargets
    NAMESPACE CopperSpice::
    FILE CopperSpiceLibraryTargets.cmake
    DESTINATION cmake/CopperSpice
)

install(
    EXPORT CopperSpiceBinaryTargets
    NAMESPACE CopperSpice::
    FILE CopperSpiceBinaryTargets.cmake
    DESTINATION cmake/CopperSpice
)

message("")
message(STATUS "The following components will be built:")
message("\n * ${BUILD_COMPONENTS}\n")

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
