AC_INIT([copperspice], [1.4.5], [info@copperspice.com])

# derive CS hex version from "@PACKAGE_VERSION@"
HEX_VERSION=$(printf "0x%02x%02x%02x" `echo $PACKAGE_VERSION | tr '.' ' '`)
BUILD_ABI=$(echo $PACKAGE_VERSION | cut -d. -f1-2)
AC_SUBST([HEX_VERSION])
AC_SUBST([BUILD_ABI])

AC_CONFIG_AUX_DIR([autotools/build-aux])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_SYS_LARGEFILE

AC_CONFIG_SRCDIR([src/core/kernel/qobject.cpp])
AM_INIT_AUTOMAKE([foreign tar-pax 1.12.1])

LT_INIT([win32-dll disable-static])
LT_PROG_RC

AC_PROG_INSTALL
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_OBJCXX

# set flags for c++14
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX(14,[noext])

AC_CONFIG_HEADERS(include/qt-acconfig.h)

case "$target" in
*-*-mingw*)
	OSTYPE=WIN ;;
*-*-linux-*)
	OSTYPE=LINUX ;;
*-*-darwin*)
	OSTYPE=DARWIN ;;
*-*-*bsd*|*-*-dragonfly*)
	OSTYPE=BSD ;;
esac

AC_CHECK_SIZEOF(size_t)

case "$ac_cv_sizeof_size_t" in
4)
	OSTYPE_DISPLAY=$OSTYPE"-32" ;;
8)
	OSTYPE_DISPLAY=$OSTYPE"-64" ;;
*)
	OSTYPE_DISPLAY=$OSTYPE"-unknown";;
esac

AC_PROG_OBJCXX

# not mandatory
echo ""
PKG_CHECK_MODULES(SSL, openssl >= 1.0.0, SSL="yes", true)
PKG_CHECK_MODULES(ZLIB, zlib, ZLIB="yes", true)

AC_CHECK_HEADERS([cups/cups.h])
AC_CHECK_HEADERS([features.h])
AC_CHECK_HEADERS([pthread.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([dirent.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([grp.h])
AC_CHECK_HEADERS([pwd.h])
AC_CHECK_HEADERS([signal.h])
AC_CHECK_HEADERS([dlfcn.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/ipc.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([sys/shm.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/wait.h])
AC_CHECK_HEADERS([sys/param.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([net/if.h], [], [],
[#ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif
])

AC_SEARCH_LIBS(clock_gettime, rt)

if test "$OSTYPE" == LINUX -o "$OSTYPE" == BSD; then

# verify dependencies
PKG_CHECK_MODULES(FONTCONFIG, fontconfig, FONTCONFIG="yes", missing="$missing FontConfig")
PKG_CHECK_MODULES(FREETYPE, freetype2, FREETYPE="yes", missing="$missing FreeType")
PKG_CHECK_MODULES(GLIB, glib-2.0, GLIB="yes", missing="$missing GLIB")

if test "x$enable_phonon" != "xno"; then
PKG_CHECK_MODULES(GSTREAMER, gstreamer-0.10, GSTREAMER="yes", missing="$missing Gstreamer")
PKG_CHECK_MODULES(GSTREAMER_AUDIO, gstreamer-audio-0.10, GSTREAMER_AUDIO="yes", missing="$missing Gstreamer-Plugins")
fi

# not mandatory
PKG_CHECK_MODULES(GTK, gtk+-2.0, GTK="yes", true)
PKG_CHECK_MODULES(ALSA, alsa, ALSA="yes", true)
PKG_CHECK_MODULES(PULSEAUDIO, libpulse >= 0.9.15, PULSEAUDIO="yes", true)
PKG_CHECK_MODULES(PULSEAUDIO_DEVICE_MANAGER, libpulse >= 0.9.21, PULSEAUDIO_DEVICE_MANAGER="yes", true)
PKG_CHECK_MODULES(PULSEAUDIO_MAINLOOP_GLIB, libpulse-mainloop-glib, PULSEAUDIO_MAINLOOP_GLIB="yes", true)

# verify dependencies
AC_SEARCH_LIBS([IceOpenConnection], [ICE], [], missing="$missing ICE")
AC_SEARCH_LIBS([AuOpenServer], [audio], [], missing="$missing Audio")
AC_SEARCH_LIBS([glClear], [GL], [], missing="$missing OpenGL")
AC_SEARCH_LIBS([pthread_create], [pthread], [], missing="$missing pthread")
AC_SEARCH_LIBS([SmcOpenConnection], [SM], [], missing="$missing SM")
AC_SEARCH_LIBS([XcursorImageCreate], [Xcursor], [], missing="$missing Xcursor")
AC_SEARCH_LIBS([XShmCreateImage], [Xext], [], missing="$missing Xext")
AC_SEARCH_LIBS([XFixesQueryVersion], [Xfixes], [], missing="$missing Xfixes")
AC_SEARCH_LIBS([XGetExtensionVersion], [Xi], [], missing="$missing Xi")
AC_SEARCH_LIBS([XineramaQueryExtension], [Xinerama], [], missing="$missing Xinerama")
AC_SEARCH_LIBS([XRRSetScreenConfigAndRate], [Xrandr], [], missing="$missing Xrandr")
AC_SEARCH_LIBS([XRenderCreateCursor], [Xrender], [], missing="$missing Xrender")
AC_SEARCH_LIBS([XCreateWindow], [X11], [], missing="$missing X11")
fi


if test "x$missing" != "x"; then
   echo "------------------------------------------------------------"
   echo "The following libraries are missing and must be installed to build CopperSpice"
   echo "Refer to the CS documentation for package names or consult your platform package manager"
   echo $missing
   echo "------------------------------------------------------------"

   if test -z "${missing##*OpenGL*}" && test -e /var/lib/parallels-tools/.backup/.libgl/libGL.so.1.2.0 ; then
     echo "If you are on Parallels, execute the following command so CopperSpice can link with OpenGL"
     echo "sudo cp /var/lib/parallels-tools/.backup/.libgl/libGL.so.1.2.0 /usr/lib/x86_64-linux-gnu/mesa/"
   fi

   echo ""
   echo "CopperSpice configuration aborted"
   exit
fi

AM_CONDITIONAL(OSTYPE_WIN, test "$OSTYPE" == WIN)
AM_CONDITIONAL(OSTYPE_LINUX, test "$OSTYPE" == LINUX)
AM_CONDITIONAL(OSTYPE_DARWIN, test "$OSTYPE" == DARWIN)
AM_CONDITIONAL(OSTYPE_BSD, test "$OSTYPE" == BSD)

AM_CONDITIONAL(X11, test "$OSTYPE" == LINUX -o "$OSTYPE" == BSD)
AM_CONDITIONAL(COCOA, test "$OSTYPE" == DARWIN)

AM_CONDITIONAL(CUPS, test "x$ac_cv_header_cups_cups_h" = x""yes)
AM_CONDITIONAL(GTK, test "x$GTK" = x""yes -a "$OSTYPE" != DARWIN)

#AM_CONDITIONAL(GLIB, test "x$GLIB" = x""yes)
#AM_CONDITIONAL(FREETYPE, test "x$FREETYPE" = x""yes)

AM_CONDITIONAL(GLIB, test "x$OSTYPE" = x""LINUX -o "x$OSTYPE" = x""BSD)
AM_CONDITIONAL(FREETYPE, test "x$OSTYPE" = x""LINUX -o "x$OSTYPE" = x""BSD)
AM_CONDITIONAL(ALSA, test "$ALSA" = "yes")
AM_CONDITIONAL(PULSEAUDIO, test "$PULSEAUDIO" = "yes")
AM_CONDITIONAL(PULSEAUDIO_DEVICE_MANAGER, test "$PULSEAUDIO_DEVICE_MANAGER" = "yes")
AM_CONDITIONAL(PULSEAUDIO_MAINLOOP_GLIB, test "$PULSEAUDIO_MAINLOOP_GLIB" = "yes")

# turns zlib off always
AM_CONDITIONAL(ZLIB, test "x" = x""yes)

AC_ARG_ENABLE([openssl],    AS_HELP_STRING([--disable-openssl],    [Do not include OpenSSL in CsNetwork]))
AC_ARG_ENABLE([multimedia], AS_HELP_STRING([--disable-multimedia], [Do not build CsMultimedia]))
AC_ARG_ENABLE([phonon],	    AS_HELP_STRING([--disable-phonon],     [Do not build CsPhonon]))
AC_ARG_ENABLE([webkit],	    AS_HELP_STRING([--disable-webkit],     [Do not build CsWebKit]))

AM_CONDITIONAL(MULTIMEDIA, test "x$enable_multimedia" != "xno")
AM_CONDITIONAL(PHONON, test "x$enable_phonon" != "xno")
AM_CONDITIONAL(WEBKIT, test "x$enable_webkit" != "xno")


real_dir_name()
{
eval realdir=${1}
eval realdir=${realdir}
eval realdir=${realdir}
}

echo
echo "Checking PostgreSQL client library"

AC_CHECK_HEADERS([libpq-fe.h], [HAVE_LIBPQ_FE_H=yes])
AC_CHECK_HEADERS([pg_config.h], [HAVE_PG_CONFIG_H=yes])
AC_CHECK_LIB([pq], [PQconnectdbParams], [HAVE_LIBPQ=yes])

AM_CONDITIONAL(BUILD_PSQL_PLUGIN, test "x"$HAVE_LIBPQ = x""yes -a "x"$HAVE_LIBPQ_FE_H = x""yes -a "x"$HAVE_PG_CONFIG_H = x""yes)

if test "x"$BUILD_PSQL_PLUGIN_TRUE = x""; then
   echo "PostgreSQL client drivers will be built"
else
   echo "PostgreSQL client library not found, client drivers will not be built"
fi

echo
echo "Checking MySQL client library"
AC_CHECK_HEADERS([mysql.h], [HAVE_MYSQL_H=yes])
AC_CHECK_LIB([mysqlclient], [my_init], [HAVE_LIBMYSQLCLIENT=yes])
AM_CONDITIONAL(BUILD_MYSQL_PLUGIN, test "x"$HAVE_MYSQL_H = x""yes -a "x"$HAVE_LIBMYSQLCLIENT = x""yes)
if test "x"$BUILD_MYSQL_PLUGIN_TRUE = x""; then
   echo "MySQL client drivers will be built"
else
   echo "MySQL client library not found, client drivers will not be built"
fi

AM_CONDITIONAL(SSL, test "x$SSL" = x""yes -a "x$enable_openssl" != "xno")
if test "x$SSL" = x""yes -a "x$enable_openssl" != "xno"; then
   echo "Building with OpenSSL"

   if ! pkg-config openssl --atleast-version=1.0.1g; then
      echo "------------------------------------------------------------"
      echo "WARNING WARNING WARNING"
      echo "The version of OpenSSL on this system may be susceptible to the Heartbleed vulnerability"
      echo "Please ensure that your vendor has applied the appropriate patches"
      echo "------------------------------------------------------------"
   fi
else
   echo "OpenSSL not found or disabled, HTTPS support will not be built into CsNetwork"
fi

echo
echo "CopperSpice configured to run on:     " $OSTYPE_DISPLAY
echo "CopperSpice will be built in:         " `pwd`
echo "CopperSpice will be installed in:     " ${prefix}
echo
echo "CopperSpice is now configured"

AC_CONFIG_FILES([Makefile    src/webkit/Makefile     src/core/global/cs_build_info.h])
AC_CONFIG_MACRO_DIR([autotools/m4])
AC_OUTPUT
