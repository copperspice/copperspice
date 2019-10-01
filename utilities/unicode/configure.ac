AC_INIT([unicode], [1.0.0], [info@copperspice.com])

AC_CONFIG_AUX_DIR([autotools/build-aux])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_SYS_LARGEFILE

AC_CONFIG_SRCDIR([main.cpp])
AM_INIT_AUTOMAKE([foreign tar-pax])

LT_INIT([win32-dll disable-static])

AM_PROG_CC_C_O
AC_PROG_CXX

# set flags for c++14
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX(14,[noext])

case "$target" in
*-*-mingw*)
	OSTYPE=WIN ;;
*-*-linux-*)
	OSTYPE=LINUX ;;
*-*-darwin*)
	OSTYPE=DARWIN ;;
*-*-*bsd*)
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

AM_CONDITIONAL(OSTYPE_WIN, test "$OSTYPE" == WIN)
AM_CONDITIONAL(OSTYPE_LINUX, test "$OSTYPE" == LINUX)
AM_CONDITIONAL(OSTYPE_DARWIN, test "$OSTYPE" == DARWIN)
AM_CONDITIONAL(OSTYPE_BSD, test "$OSTYPE" == BSD)

real_dir_name()
{
eval realdir=${1}
eval realdir=${realdir}
eval realdir=${realdir}
}

echo
echo "Configuration Completed"

AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIR([autotools/m4])
AC_OUTPUT
