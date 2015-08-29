# NOTE: to find all FIXME/TODO related to the CMake build system (and those
# inherited from the autotools build system) execute the following in the
# top-level directory of the source tree:
#
# find -name '*.cmake' -exec grep -E 'TODO|FIXME' {} +
# find -name CMakeLists.txt -exec grep -E 'TODO|FIXME' {} +

project(copperspice)

cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
# cmake_policy(VERSION 2.8.11)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(FeatureSummary)
include(TestLargeFiles)
include(MacroCopperSpice)
if(UNIX)
    include(GNUInstallDirs)
else()
    set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX})
    set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX})
    set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX})
    set(CMAKE_INSTALL_DATAROOTDIR ${CMAKE_INSTALL_PREFIX})
endif()

install(
    FILES
    ${CMAKE_SOURCE_DIR}/cmake/modules/FindCopperSpice.cmake
    ${CMAKE_SOURCE_DIR}/cmake/modules/CopperSpiceMacros.cmake
    DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/cmake/modules
)

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
set(BUILD_MAJOR "1")
set(BUILD_MINOR "2")
set(BUILD_MICRO "0")
set(PACKAGE "copperspice")
set(PACKAGE_NAME "copperspice")
set(PACKAGE_VERSION "${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_STRING "copperspice ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_TARNAME "copperspice")
set(PACKAGE_BUGREPORT "ansel@copperspice.com")
set(PACKAGE_URL "http://www.copperspice.com/") # empty leaves it undefined
set(VERSION "${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(HEX_VERSION "0x0${BUILD_MAJOR}0${BUILD_MINOR}0${BUILD_MICRO}")
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(target "${CMAKE_SYSTEM_NAME}")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ library derived from the existing Qt 4.8 framework")
set(CPACK_PACKAGE_VENDOR "copperspice")
# set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/ReadMe.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license/LICENSE.LGPL")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git;${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_VERSION_MAJOR ${BUILD_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BUILD_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BUILD_MICRO})
include(CPack)

# TODO: cleanup include dirs now that this is turned on
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-DQT_SHARED -DHAVE_CONFIG_H -DBUILD_DATE="${BUILD_DATE}")

check_include_file(cups/cups.h HAVE_CUPS_CUPS_H)
check_include_file(features.h HAVE_FEATURES_H)
check_include_file(pthread.h HAVE_PTHREAD_H)
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
check_include_file(mysql.h HAVE_MYSQL_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_files("dlfcn.h;stdint.h;stddef.h;inttypes.h;stdlib.h;strings.h;string.h;float.h" STDC_HEADERS)
check_type_size(size_t SIZEOF_SIZE_T)
opj_test_large_files(HAVE_LARGEFILESUPPORT)

find_package(OpenSSL)
set_package_properties(OpenSSL PROPERTIES
    PURPOSE "Required for HTTPS support"
    DESCRIPTION "Support for secure network communications (SSL and TLS)"
    URL "http://openssl.org"
    TYPE RECOMMENDED
)

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    PURPOSE "Required for compression support"
    DESCRIPTION "A Massively Spiffy Yet Delicately Unobtrusive Compression Library"
    URL "http://zlib.net"
    TYPE RECOMMENDED
)

find_package(Cups)
set_package_properties(Cups PROPERTIES
    PURPOSE "Required for printing support"
    DESCRIPTION "CUPS is the standards-based, open source printing system"
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
    DESCRIPTION "The world's most advanced open source database"
    URL "http://www.postgresql.org"
    TYPE RECOMMENDED
)

find_package(MySQL)
set_package_properties(MySQL PROPERTIES
    PURPOSE "Required for MySQL database support"
    DESCRIPTION "The world's most popular open source database"
    URL "http://www.mysql.com"
    TYPE RECOMMENDED
)

# find_package(QPA) TODO: what is QPA?

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_package(GTK2)
    set_package_properties(GTK2 PROPERTIES
        PURPOSE "Required for GTK style and application integration support"
        DESCRIPTION "Multi-platform toolkit for creating graphical user interfaces"
        URL "http://www.gtk.org"
        TYPE RECOMMENDED
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD)")
    find_library(pthread_FOUND pthread) # pthread_create

    find_package(NAS)
    set_package_properties(NAS PROPERTIES
        PURPOSE "Required for basic audio support over wire"
        DESCRIPTION "Network transparent, client/server audio transport system"
        URL "http://www.radscan.com/nas.html"
        TYPE REQUIRED
    )

    find_package(OpenGL)
    set_package_properties(OpenGL PROPERTIES
        PURPOSE "Required for OpenGL support"
        DESCRIPTION "The Mesa 3D Graphics Library"
        URL "http://www.mesa3d.org/"
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

    find_package(GLIB2)
    set_package_properties(GLIB2 PROPERTIES
        PURPOSE "Required for glib mainloop support"
        DESCRIPTION "Core application building blocks for libraries and applications written in C"
        URL "https://developer.gnome.org/glib"
        TYPE REQUIRED
    )
    include_directories(${GLIB2_INCLUDE_DIR})

    find_package(GStreamer)
    set_package_properties(GStreamer PROPERTIES
        PURPOSE "Required for multimedia support"
        DESCRIPTION "Open source multimedia framework"
        URL "http://gstreamer.freedesktop.org"
        TYPE REQUIRED
    )

    find_package(LibXml2 REQUIRED)

    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        find_package(Iconv)
        set_package_properties(Iconv PROPERTIES
            PURPOSE "Iconv support"
            DESCRIPTION "iconv() implementation, for use on systems which don't have one or lack Unicode support"
            URL "http://www.gnu.org/software/libiconv/"
            TYPE REQUIRED
        )
    endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
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

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/qt-acconfig.h.cmake
    ${CMAKE_BINARY_DIR}/include/qt-acconfig.h
)
configure_file(
    ${CMAKE_SOURCE_DIR}/src/core/global/cs_build_info.h.in
    ${CMAKE_BINARY_DIR}/include/QtCore/cs_build_info.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD)")
    foreach(library pthread)
        if(NOT ${library}_FOUND)
            message(SEND_ERROR "${library} not found")
        endif()
    endforeach()
endif()

option(WITH_PSQL_PLUGIN "Build PostgreSQL database plugin (if possible)" ON)
add_feature_info(PostgreSQL WITH_PSQL_PLUGIN "the world's most advanced open source database")

option(WITH_MYSQL_PLUGIN "Build MySQL database plugin (if possible)" ON)
add_feature_info(MySQL WITH_MYSQL_PLUGIN "the world's most popular open source database")

option(WITH_WEBKIT "Build WebKit component" ON)
add_feature_info(WebKit WITH_WEBKIT "an open source web browser engine")

add_subdirectory(src/core)
# out for now add_subdirectory(declarative)
add_subdirectory(src/gui)
add_subdirectory(src/multimedia)
add_subdirectory(src/network)
add_subdirectory(src/opengl)
add_subdirectory(src/phonon)
add_subdirectory(src/script)
# out for now add_subdirectory(scripttools)
add_subdirectory(src/sql)
add_subdirectory(src/svg)
# TODO: support suffixed tools, after that export them
add_subdirectory(src/tools/shared)
add_subdirectory(src/tools/lconvert)
add_subdirectory(src/tools/lrelease)
add_subdirectory(src/tools/lupdate)
add_subdirectory(src/tools/rcc)
add_subdirectory(src/tools/uic)
if(WITH_WEBKIT)
    add_subdirectory(src/webkit)
endif()
add_subdirectory(src/xml)
add_subdirectory(src/xmlpatterns)

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
    DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/cmake/CopperSpice
)

install(
    EXPORT CopperSpiceLibraryTargets
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/CopperSpice
    FILE CopperSpiceLibraryTargets.cmake
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)