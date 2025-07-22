/*
  Configuration defines.
*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <qglobal.h>

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Support CCITT Group 3 & 4 algorithms */
/* #undef CCITT_SUPPORT */

/* Pick up YCbCr subsampling info from the JPEG data stream to support files
   lacking the tag (default enabled). */
/* #undef CHECK_JPEG_YCBCR_SUBSAMPLING */

/* enable partial strip reading for large strips (experimental) */
/* #undef CHUNKY_STRIP_READ_SUPPORT */

/* Support C++ stream API (requires C++ compiler) */
/* #undef CXX_SUPPORT */

/* Treat extra sample as alpha (default enabled). The RGBA interface will
   treat a fourth sample with no EXTRASAMPLE_ value as being ASSOCALPHA. Many
   packages produce RGBA files but don't mark the alpha properly. */
/* #undef DEFAULT_EXTRASAMPLE_AS_ALPHA */

/* enable deferred strip/tile offset/size loading (experimental) */
/* #undef DEFER_STRILE_LOAD */

/* Define to 1 if you have the <assert.h> header file. */
#define HAVE_ASSERT_H 1

/* Define to 1 if you have the declaration of `optarg', and to 0 if you don't.
*/
#define HAVE_DECL_OPTARG 0

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
/* #undef HAVE_FSEEKO */

/* Define as 0 or 1 according to the floating point format suported by the
   machine */
/* #undef HAVE_IEEEFP */

/* Define to 1 if you have the <io.h> header file. */
/* #undef HAVE_IO_H */

/* Define to 1 if you have the `jbg_newlen' function. */
/* #undef HAVE_JBG_NEWLEN */

/* Define to 1 if you have the `mmap' function. */
/* #undef HAVE_MMAP */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#if ! defined(Q_OS_WIN)
#define HAVE_UNISTD_H 1
#endif

/* Support ISO JBIG compression (requires JBIG-KIT library) */
/* #undef JBIG_SUPPORT */

/* 8/12 bit libjpeg dual mode enabled */
/* #undef JPEG_DUAL_MODE_8_12 */

/* Support JPEG compression (requires IJG JPEG library) */
/* #undef JPEG_SUPPORT */

/* 12bit libjpeg primary include file with path */
/* #undef LIBJPEG_12_PATH */

/* Support LogLuv high dynamic range encoding */
/* #undef LOGLUV_SUPPORT */

/* Support LZMA2 compression */
/* #undef LZMA_SUPPORT */

/* Support LZW algorithm */
/* #undef LZW_SUPPORT */

/* Support Microsoft Document Imaging format */
/* #undef MDI_SUPPORT */

/* Support NeXT 2-bit RLE algorithm */
/* #undef NEXT_SUPPORT */

/* Support Old JPEG compresson (read-only) */
/* #undef OJPEG_SUPPORT */

/* Name of package */
/* #undef PACKAGE */

/* Define to the address where bug reports for this package should be sent. */
/* #undef PACKAGE_BUGREPORT */

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.7.0"

/* Support Macintosh PackBits algorithm */
/* #undef PACKBITS_SUPPORT */

/* Support Pixar log-format algorithm (requires Zlib) */
/* #undef PIXARLOG_SUPPORT */

/* The size of `size_t', as computed by sizeof. */
#if SIZE_MAX == 0xffffffff
#define SIZEOF_SIZE_T 4
#elif SIZE_MAX == 0xffffffffffffffff
#define SIZEOF_SIZE_T 8
#endif

/* Support strip chopping (whether or not to convert single-strip uncompressed
   images to mutiple strips of specified size to reduce memory usage) */
/* #undef STRIPCHOP_DEFAULT TIFF_STRIPCHOP */

/* Default size of the strip in bytes (when strip chopping enabled) */
/* #undef STRIP_SIZE_DEFAULT */

/* Pointer difference type formatter */
#define TIFF_PTRDIFF_FORMAT "%ld"

/* Pointer difference type */
#define TIFF_PTRDIFF_T ptrdiff_t

/* Size type formatter */
/* #undef TIFF_SIZE_FORMAT */

/* Signed size type formatter */
#define TIFF_SSIZE_FORMAT "%zd"

/* Unsigned 32-bit type formatter */
#define TIFF_UINT32_FORMAT "%u"

/* Unsigned 64-bit type formatter */
#define TIFF_UINT64_FORMAT "%llu"

/* define to use win32 IO system */
#ifdef Q_OS_WIN
#define USE_WIN32_FILEIO 1
#endif

/* Version number of package */
/* #undef VERSION */

/* Support webp compression */
/* #undef WEBP_SUPPORT */

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if (Q_BYTE_ORDER == Q_BIG_ENDIAN)
#define WORDS_BIGENDIAN 1
#endif

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* Support Deflate compression */
/* #undef ZIP_SUPPORT */

/* Enable large inode numbers on Mac OS X 10.5.  */
#if defined(Q_OS_DARWIN)
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

