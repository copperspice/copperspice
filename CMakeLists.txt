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
include(MacroCopperSpice)

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
set(BUILD_MAJOR "1")
set(BUILD_MINOR "1")
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
# set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
# set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${BUILD_MINOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BUILD_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BUILD_MICRO})
include(CPack)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)
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

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_library(audio_FOUND audio) # AuOpenServer
    find_library(GL_FOUND GL) # glClear
    find_library(pthread_FOUND pthread) # pthread_create

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

    find_package(Freetype REQUIRED)
    find_package(GLIB2 REQUIRED)
    find_package(GStreamer REQUIRED)
    find_package(LibXml2 REQUIRED)
    include_directories(${GLIB2_INCLUDE_DIR})
endif()

configure_file(qt-acconfig.h.cmake ${CMAKE_BINARY_DIR}/include/qt-acconfig.h)

add_subdirectory(src)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    foreach(library audio GL pthread)
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

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)