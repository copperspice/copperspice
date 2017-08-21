/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QGLOBAL_H
#define QGLOBAL_H

#include <stddef.h>
#include <cs_build_info.h>

// usage:  #if (CS_VERSION >= CS_VERSION_CHECK(1, 1, 0))
#define CS_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

#include <qconfig.h>
#include <qfeatures.h>

#ifdef __cplusplus     // block a
#include <algorithm>

class QByteArray;
class QDataStream;
class QDebug;
class QNoDebug;
class QString;

#ifdef QT_NAMESPACE    // block b

#define QT_PREPEND_NAMESPACE(name) ::QT_NAMESPACE::name

#define QT_USE_NAMESPACE    using namespace ::QT_NAMESPACE;

#define QT_BEGIN_NAMESPACE  namespace QT_NAMESPACE {
#define QT_END_NAMESPACE    }

#define QT_BEGIN_INCLUDE_NAMESPACE  }
#define QT_END_INCLUDE_NAMESPACE    namespace QT_NAMESPACE {

#define QT_FORWARD_DECLARE_CLASS(name)  \
   QT_BEGIN_NAMESPACE class name;  \
   QT_END_NAMESPACE \
   using QT_PREPEND_NAMESPACE(name);

# define QT_FORWARD_DECLARE_STRUCT(name) \
    QT_BEGIN_NAMESPACE struct name;  \
    QT_END_NAMESPACE \
    using QT_PREPEND_NAMESPACE(name);

# define QT_MANGLE_NAMESPACE0(x) x
# define QT_MANGLE_NAMESPACE1(a, b)  a##_##b
# define QT_MANGLE_NAMESPACE2(a, b)  QT_MANGLE_NAMESPACE1(a,b)

# define QT_MANGLE_NAMESPACE(name)   QT_MANGLE_NAMESPACE2( QT_MANGLE_NAMESPACE0(name), QT_MANGLE_NAMESPACE0(QT_NAMESPACE))

namespace QT_NAMESPACE {}

#ifndef QT_NO_USING_NAMESPACE
QT_USE_NAMESPACE
#endif

#else  // block b

# define QT_PREPEND_NAMESPACE(name) ::name

# define QT_USE_NAMESPACE

# define QT_BEGIN_NAMESPACE
# define QT_END_NAMESPACE

# define QT_BEGIN_INCLUDE_NAMESPACE
# define QT_END_INCLUDE_NAMESPACE

# define QT_FORWARD_DECLARE_CLASS(name)  class name;
# define QT_FORWARD_DECLARE_STRUCT(name) struct name;

# define QT_MANGLE_NAMESPACE(name)       name

#endif  // block b


#else   // block a

#define QT_BEGIN_NAMESPACE
#define QT_END_NAMESPACE
#define QT_USE_NAMESPACE
#define QT_BEGIN_INCLUDE_NAMESPACE
#define QT_END_INCLUDE_NAMESPACE

#endif   // block a


// detect target architecture
#if defined(__x86_64__)
#define QT_ARCH_X86_64

#elif defined(__i386__)
#define QT_ARCH_I386

#else
#error Unable to detect architecture, please update above list

#endif

// detect target endianness
#if defined (__BYTE_ORDER__) && \
    (__BYTE_ORDER__ - 0 == __ORDER_BIG_ENDIAN__ - 0 || __BYTE_ORDER__ - 0 == __ORDER_LITTLE_ENDIAN__ - 0)

#define Q_BYTE_ORDER       __BYTE_ORDER__
#define Q_BIG_ENDIAN       __ORDER_BIG_ENDIAN__
#define Q_LITTLE_ENDIAN    __ORDER_LITTLE_ENDIAN__

#elif defined (__LITTLE_ENDIAN__)
#define Q_BIG_ENDIAN 1234
#define Q_LITTLE_ENDIAN 4321
#define Q_BYTE_ORDER Q_LITTLE_ENDIAN

#elif defined (__BIG_ENDIAN__)
#define Q_BIG_ENDIAN 1234
#define Q_LITTLE_ENDIAN 4321
#define Q_BYTE_ORDER Q_BIG_ENDIAN

#else
#error Unable to detect target endianness

#endif

// ******
#if defined(Q_OS_MAC) && ! defined(Q_CC_INTEL)
#define QT_BEGIN_INCLUDE_HEADER     }
#define QT_END_INCLUDE_HEADER     extern "C++" {

#else
#define QT_BEGIN_INCLUDE_HEADER
#define QT_END_INCLUDE_HEADER     extern "C++"

#endif

// ******
#if defined(__APPLE__) && defined(__GNUC__)
#  define Q_OS_DARWIN
#  define Q_OS_BSD4

#elif ! defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define Q_OS_WIN32
#  define Q_OS_WIN64

#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  define Q_OS_WIN32

#elif defined(__native_client__)
#  define Q_OS_NACL

#elif defined(__linux__) || defined(__linux)
#  define Q_OS_LINUX

#elif defined(__DragonFly__)
#  define Q_OS_DRAGONFLY
#  define Q_OS_BSD4

#elif defined(__FreeBSD__)
#  define Q_OS_FREEBSD
#  define Q_OS_BSD4

#elif defined(__NetBSD__)
#  define Q_OS_NETBSD
#  define Q_OS_BSD4

#elif defined(__OpenBSD__)
#  define Q_OS_OPENBSD
#  define Q_OS_BSD4

#elif defined(__MAKEDEPEND__)

#else
#  error "CopperSpice has not been ported to this OS"

#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#  define Q_OS_WIN
#endif

#if defined(Q_OS_DARWIN)
#  define Q_OS_MAC
#endif

#if defined(Q_OS_WIN)
#  undef Q_OS_UNIX

#elif ! defined(Q_OS_UNIX)
#  define Q_OS_UNIX

#endif

#if defined(Q_OS_DARWIN) && ! defined(QT_LARGEFILE_SUPPORT)
#  define QT_LARGEFILE_SUPPORT 64
#endif

#ifdef Q_OS_DARWIN
#  ifdef MAC_OS_X_VERSION_MIN_REQUIRED
#    undef MAC_OS_X_VERSION_MIN_REQUIRED
#  endif

#  define MAC_OS_X_VERSION_MIN_REQUIRED   MAC_OS_X_VERSION_10_8
#  include <AvailabilityMacros.h>

#  if !defined(MAC_OS_X_VERSION_10_9)
#     define MAC_OS_X_VERSION_10_9    MAC_OS_X_VERSION_10_8 + 10
#  endif

#  if ! defined(MAC_OS_X_VERSION_10_10)
#     define MAC_OS_X_VERSION_10_10 101000
#  endif

#  if ! defined(MAC_OS_X_VERSION_10_11)
#     define MAC_OS_X_VERSION_10_11 101100
#  endif

#  if ! defined(MAC_OS_X_VERSION_10_12)
#     define MAC_OS_X_VERSION_10_12 101200
#  endif

#endif

#ifdef __LSB_VERSION__
#  if __LSB_VERSION__ < 40
#    error "This version of the Linux Standard Base is unsupported"
#  endif

#ifndef QT_LINUXBASE
#  define QT_LINUXBASE
#endif

#endif

// ******
#if defined(__clang__)

#  if ( __clang_major__ == 3  &&  __clang_minor__ >= 2 ) || ( __clang_major__  >= 4  )
#    define Q_CC_CLANG
#    define Q_CC_GNU
#    define Q_C_CALLBACKS

#    define Q_ALIGNOF(type)   __alignof__(type)
#    define Q_LIKELY(expr)    __builtin_expect(!!(expr), true)
#    define Q_UNLIKELY(expr)  __builtin_expect(!!(expr), false)
#    define Q_PACKED          __attribute__ ((__packed__))

#    define Q_NO_PACKED_REFERENCE

#    ifndef __ARM_EABI__
#      define QT_NO_ARM_EABI
#    endif

#  else
#    error "CopperSpice requires Clang 3.2 or greater"

#  endif


#elif defined(__GNUC__)
//  **

#  if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 8)
#    error "CopperSpice requires GCC 4.9 or greater"
#  endif

#  define Q_CC_GNU
#  define Q_C_CALLBACKS

#  if defined(__MINGW32__)
#    define Q_CC_MINGW
#  endif

//  Intel C++ also masquerades as GCC
#  if defined(__INTEL_COMPILER)
#    define Q_CC_INTEL
#  endif

#  define Q_ALIGNOF(type)   __alignof__(type)
#  define Q_LIKELY(expr)    __builtin_expect(!!(expr), true)
#  define Q_UNLIKELY(expr)  __builtin_expect(!!(expr), false)

#  if (defined(Q_CC_GNU) || defined(Q_CC_INTEL))
#    define Q_PACKED __attribute__ ((__packed__))
#    define Q_NO_PACKED_REFERENCE

#    ifndef __ARM_EABI__
#      define QT_NO_ARM_EABI
#    endif
#  endif


#elif defined(__PGI)
//  **

#  define Q_CC_PGI

#  if defined(__EDG__)
#    define Q_CC_EDG
#  endif


#elif ! defined(Q_OS_HPUX) && (defined(__EDG) || defined(__EDG__))
//  **

#  define Q_CC_EDG

#  if defined(__COMO__)
#    define Q_CC_COMEAU
#    define Q_C_CALLBACKS

#  elif defined(__KCC)
#    define Q_CC_KAI

#  elif defined(__INTEL_COMPILER)
#    define Q_CC_INTEL

#  elif defined(CENTERLINE_CLPP) || defined(OBJECTCENTER)
#    define Q_CC_OC

#  elif defined(sinix)
#    define Q_CC_CDS

#  elif defined(__sgi)
#    define Q_CC_MIPS

#    if defined(_COMPILER_VERSION) && (_COMPILER_VERSION >= 740)
#      define Q_OUTOFLINE_TEMPLATE inline
#      pragma set woff 3624,3625,3649          // turn off some harmless warnings
#    endif
#  endif


#else
//  **

#  error "CopperSpice has not been tested with this Compiler"

#endif

#ifndef Q_PACKED
#  define Q_PACKED
#  undef Q_NO_PACKED_REFERENCE
#endif

#ifndef Q_LIKELY
#  define Q_LIKELY(x) (x)
#endif

#ifndef Q_UNLIKELY
#  define Q_UNLIKELY(x) (x)
#endif

#ifndef Q_CONSTRUCTOR_FUNCTION
# define Q_CONSTRUCTOR_FUNCTION0(AFUNC)    static const int AFUNC ## __init_variable__ = AFUNC();
# define Q_CONSTRUCTOR_FUNCTION(AFUNC)     Q_CONSTRUCTOR_FUNCTION0(AFUNC)
#endif

#ifndef Q_DESTRUCTOR_FUNCTION
# define Q_DESTRUCTOR_FUNCTION0(AFUNC) \
  class AFUNC ## __dest_class__ { \
    public: \
       inline AFUNC ## __dest_class__() { } \
       inline ~ AFUNC ## __dest_class__() { AFUNC(); } \
    } AFUNC ## __dest_instance__;

# define Q_DESTRUCTOR_FUNCTION(AFUNC)      Q_DESTRUCTOR_FUNCTION0(AFUNC)
#endif


// *****
#ifndef Q_REQUIRED_RESULT
#  if defined(Q_CC_GNU) && ! defined(Q_CC_INTEL)
#    define Q_REQUIRED_RESULT              __attribute__ ((warn_unused_result))
#  else
#    define Q_REQUIRED_RESULT
#  endif
#endif


// *****
#if defined(_WIN32_X11_)
#  define Q_WS_X11

#elif defined(Q_OS_UNIX)

#  if defined(Q_OS_MAC) && ! defined(__USE_WS_X11__) && ! defined(Q_WS_QWS) && ! defined(Q_WS_QPA)
#    define Q_OS_MAC

#  elif ! defined(Q_WS_QWS) && ! defined(Q_WS_QPA)
#    define Q_WS_X11
#  endif

#endif

// make sure to update QMetaType when changing these typedefs
#include <stdint.h>

typedef int8_t                 qint8;
typedef uint8_t                quint8;
typedef int16_t                qint16;
typedef uint16_t               quint16;
typedef int32_t                qint32;
typedef uint32_t               quint32;

typedef long long              qint64;
typedef unsigned long long     quint64;

#define Q_INT64_C(c)           static_cast<long long>(c ## LL)
#define Q_UINT64_C(c)          static_cast<unsigned long long>(c ## ULL)

typedef qint64                 qlonglong;
typedef quint64                qulonglong;

#ifndef QT_POINTER_SIZE
#define QT_POINTER_SIZE        sizeof(void *)
#endif

#define Q_INIT_RESOURCE_EXTERN(name) \
   extern int QT_MANGLE_NAMESPACE(qInitResources_ ## name) ();

#define Q_INIT_RESOURCE(name) \
   do { extern int QT_MANGLE_NAMESPACE(qInitResources_ ## name) ();       \
   QT_MANGLE_NAMESPACE(qInitResources_ ## name) (); } while (0)

#define Q_CLEANUP_RESOURCE(name) \
   do { extern int QT_MANGLE_NAMESPACE(qCleanupResources_ ## name) ();    \
   QT_MANGLE_NAMESPACE(qCleanupResources_ ## name) (); } while (0)


#if defined(__cplusplus)      // block c

/*
quintptr and qptrdiff are guaranteed to be the same size as a pointer
      sizeof(void *) == sizeof(quintptr)
      && sizeof(void *) == sizeof(qptrdiff)
*/

template<int>
struct QIntegerForSize;

template <>
struct QIntegerForSize<1> {
   typedef quint8  Unsigned;
   typedef qint8   Signed;
};

template <>
struct QIntegerForSize<2> {
   typedef quint16 Unsigned;
   typedef qint16  Signed;
};

template <>
struct QIntegerForSize<4> {
   typedef quint32 Unsigned;
   typedef qint32  Signed;
};

template <>
struct QIntegerForSize<8> {
   typedef quint64 Unsigned;
   typedef qint64  Signed;
};

template <class T>
struct QIntegerForSizeof : QIntegerForSize<sizeof(T)> {
};

typedef QIntegerForSizeof<void *>::Unsigned   quintptr;
typedef QIntegerForSizeof<void *>::Signed     qptrdiff;
typedef qptrdiff qintptr;

typedef unsigned char    uchar;
typedef unsigned short   ushort;
typedef unsigned int     uint;
typedef unsigned long    ulong;

// ****
#ifndef TRUE
#  define TRUE  true
#  define FALSE false
#endif

// warnings for deprecated classes or methods
#if defined(QT_DEPRECATED_WARNINGS)

#  if defined(Q_CC_GNU) && ! defined(Q_CC_INTEL)
#    define QT_DEPRECATED       __attribute__ ((__deprecated__))
#  else
#    define QT_DEPRECATED
#  endif

#else
#  define QT_DEPRECATED

#endif


#if defined(__i386__) || defined(_WIN32)
#  if defined(Q_CC_GNU)

#    if ! defined(Q_CC_INTEL)
#       define QT_FASTCALL      __attribute__((regparm(3)))
#    else
#       define QT_FASTCALL
#    endif

#  else
#     define QT_FASTCALL
#  endif

#else
#  define QT_FASTCALL

#endif

// defines the type for WNDPROC on windows
// alignment needs to be forced for sse2 to not crash with mingw

#if defined(Q_OS_WIN)
#  if defined(Q_CC_MINGW)
#    define QT_ENSURE_STACK_ALIGNED_FOR_SSE    __attribute__((force_align_arg_pointer))
#  else
#    define QT_ENSURE_STACK_ALIGNED_FOR_SSE
#  endif

#  define QT_WIN_CALLBACK CALLBACK             QT_ENSURE_STACK_ALIGNED_FOR_SSE
#endif

typedef int QNoImplicitBoolCast;

#if defined(QT_ARCH_ARM) || defined(QT_ARCH_ARMV6) || defined(QT_ARCH_AVR32) ||  \
       (defined(QT_ARCH_MIPS) && (defined(Q_WS_QWS) || defined(Q_WS_QPA))) ||    \
        defined(QT_ARCH_SH) || defined(QT_ARCH_SH4A)

#define QT_NO_FPU
#endif

// must match both files:  qmetatype.h & qglobal.h
#if defined(QT_COORD_TYPE)
   typedef QT_COORD_TYPE  qreal;

#elif defined(QT_NO_FPU) || defined(QT_ARCH_ARM)
   typedef float          qreal;

#else
   typedef double         qreal;

#endif


// utility macros and inline functions
template <typename T>
constexpr inline T qAbs(const T &t)
{
   return t >= 0 ? t : -t;
}

constexpr inline int qRound(qreal d)
{
   return d >= qreal(0.0) ? int(d + qreal(0.5)) : int(d - int(d - 1) + qreal(0.5)) + int(d - 1);
}

#if defined(QT_NO_FPU) || defined(QT_ARCH_ARM)
constexpr inline qint64 qRound64(double d)
{
   return d >= 0.0 ? qint64(d + 0.5) : qint64(d - qreal(qint64(d - 1)) + 0.5) + qint64(d - 1);
}

#else
constexpr inline qint64 qRound64(qreal d)
{
   return d >= qreal(0.0) ? qint64(d + qreal(0.5)) : qint64(d - qreal(qint64(d - 1)) + qreal(0.5)) + qint64(d - 1);
}

#endif

// enhanced to support size_type which can be 32 bit or 64 bit
// the larger data type size will be returned
template <typename T1, typename T2>
constexpr inline auto qMin(const T1 &a, const T2 &b) -> decltype((a < b) ? a : b)
{
   return (a < b) ? a : b;
}

template <typename T1, typename T2>
constexpr inline auto qMax(const T1 &a, const T2 &b) -> decltype((a < b) ? b : a)
{
   return (a < b) ? b : a;
}

template <typename T1, typename T2, typename T3>
constexpr inline auto qBound(const T1 &min, const T2 &val, const T3 &max) -> decltype(qMax(min, qMin(max, val)))
{
   return qMax(min, qMin(max, val));
}

#ifndef QT_BUILD_KEY
#define QT_BUILD_KEY "(copperspice)"
#endif

#if defined(Q_OS_MAC)
#  ifndef QMAC_QMENUBAR_NO_EVENT
#    define QMAC_QMENUBAR_NO_EVENT
#  endif
#endif

#endif          //  block c


#ifndef Q_DECL_EXPORT
#  if defined(Q_OS_WIN)
#    define Q_DECL_EXPORT    __declspec(dllexport)

#  elif defined(QT_VISIBILITY_AVAILABLE)
#    define Q_DECL_EXPORT    __attribute__((visibility("default")))
#    define Q_DECL_HIDDEN    __attribute__((visibility("hidden")))
#  endif

#  ifndef Q_DECL_EXPORT
#    define Q_DECL_EXPORT
#  endif

#endif

#ifndef Q_DECL_IMPORT
#  if defined(Q_OS_WIN)
#    define Q_DECL_IMPORT    __declspec(dllimport)
#  else
#    define Q_DECL_IMPORT
#  endif
#endif

#ifndef Q_DECL_HIDDEN
#  define Q_DECL_HIDDEN
#endif


#if defined(Q_OS_WIN) &&  ! defined(QT_STATIC)        // create a DLL library

#    if defined(QT_BUILD_CORE_LIB)
#      define Q_CORE_EXPORT          Q_DECL_EXPORT
#    else
#      define Q_CORE_EXPORT          Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_GUI_LIB)
#      define Q_GUI_EXPORT           Q_DECL_EXPORT
#    else
#      define Q_GUI_EXPORT           Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_SQL_LIB)
#      define Q_SQL_EXPORT           Q_DECL_EXPORT
#    else
#      define Q_SQL_EXPORT           Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_NETWORK_LIB)
#      define Q_NETWORK_EXPORT       Q_DECL_EXPORT
#    else
#      define Q_NETWORK_EXPORT       Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_SVG_LIB)
#      define Q_SVG_EXPORT           Q_DECL_EXPORT
#    else
#      define Q_SVG_EXPORT           Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_DECLARATIVE_LIB)
#      define Q_DECLARATIVE_EXPORT   Q_DECL_EXPORT
#    else
#      define Q_DECLARATIVE_EXPORT   Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_OPENGL_LIB)
#      define Q_OPENGL_EXPORT        Q_DECL_EXPORT
#    else
#      define Q_OPENGL_EXPORT        Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_MULTIMEDIA_LIB)
#      define Q_MULTIMEDIA_EXPORT    Q_DECL_EXPORT
#    else
#      define Q_MULTIMEDIA_EXPORT    Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_XML_LIB)
#      define Q_XML_EXPORT           Q_DECL_EXPORT
#    else
#      define Q_XML_EXPORT           Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_XMLPATTERNS_LIB)
#      define Q_XMLPATTERNS_EXPORT   Q_DECL_EXPORT
#    else
#      define Q_XMLPATTERNS_EXPORT   Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_SCRIPT_LIB)
#      define Q_SCRIPT_EXPORT        Q_DECL_EXPORT
#    else
#      define Q_SCRIPT_EXPORT        Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_SCRIPTTOOLS_LIB)
#      define Q_SCRIPTTOOLS_EXPORT   Q_DECL_EXPORT
#    else
#      define Q_SCRIPTTOOLS_EXPORT   Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_DBUS_LIB)
#      define Q_DBUS_EXPORT          Q_DECL_EXPORT
#    else
#      define Q_DBUS_EXPORT          Q_DECL_IMPORT
#    endif

#endif

#if ! defined(Q_CORE_EXPORT)

#  if ! defined(QT_STATIC)
#    define Q_CORE_EXPORT           Q_DECL_EXPORT
#    define Q_GUI_EXPORT            Q_DECL_EXPORT
#    define Q_SQL_EXPORT            Q_DECL_EXPORT
#    define Q_NETWORK_EXPORT        Q_DECL_EXPORT
#    define Q_SVG_EXPORT            Q_DECL_EXPORT
#    define Q_DECLARATIVE_EXPORT    Q_DECL_EXPORT
#    define Q_OPENGL_EXPORT         Q_DECL_EXPORT
#    define Q_MULTIMEDIA_EXPORT     Q_DECL_EXPORT
#    define Q_XML_EXPORT            Q_DECL_EXPORT
#    define Q_XMLPATTERNS_EXPORT    Q_DECL_EXPORT
#    define Q_SCRIPT_EXPORT         Q_DECL_EXPORT
#    define Q_SCRIPTTOOLS_EXPORT    Q_DECL_EXPORT
#    define Q_DBUS_EXPORT           Q_DECL_EXPORT
#  else
#    define Q_CORE_EXPORT
#    define Q_GUI_EXPORT
#    define Q_SQL_EXPORT
#    define Q_NETWORK_EXPORT
#    define Q_SVG_EXPORT
#    define Q_DECLARATIVE_EXPORT
#    define Q_OPENGL_EXPORT
#    define Q_MULTIMEDIA_EXPORT
#    define Q_XML_EXPORT
#    define Q_XMLPATTERNS_EXPORT
#    define Q_SCRIPT_EXPORT
#    define Q_SCRIPTTOOLS_EXPORT
#    define Q_DBUS_EXPORT
#  endif

#endif


#if defined(__cplusplus)      // block d

inline void qt_noop(void) {}

#define QT_TRY       try
#define QT_CATCH(A)  catch (A)
#define QT_THROW(A)  throw A
#define QT_RETHROW   throw

// System information
class Q_CORE_EXPORT QSysInfo
{
 public:
   enum Sizes {
      WordSize = (sizeof(void *) << 3)
   };

#if defined(Q_BYTE_ORDER)
   enum Endian {
      BigEndian,
      LittleEndian,

#  if Q_BYTE_ORDER == Q_BIG_ENDIAN
      ByteOrder = BigEndian

#  elif Q_BYTE_ORDER == Q_LITTLE_ENDIAN
      ByteOrder = LittleEndian

#  endif
   };
#endif

#if defined(Q_OS_WIN)
   enum WinVersion {
      WV_32s        = 0x0001,
      WV_95         = 0x0002,
      WV_98         = 0x0003,
      WV_Me         = 0x0004,
      WV_DOS_based  = 0x000f,

      WV_NT         = 0x0010,
      WV_2000       = 0x0020,
      WV_XP         = 0x0030,
      WV_2003       = 0x0040,
      WV_VISTA      = 0x0080,
      WV_WINDOWS7   = 0x0090,
      WV_WINDOWS8   = 0x00a0,
      WV_WINDOWS8_1 = 0x00b0,
      WV_WINDOWS10  = 0x00c0,
      WV_NT_based   = 0x00f0,

      WV_4_0        = WV_NT,
      WV_5_0        = WV_2000,
      WV_5_1        = WV_XP,
      WV_5_2        = WV_2003,
      WV_6_0        = WV_VISTA,
      WV_6_1        = WV_WINDOWS7,
      WV_6_2        = WV_WINDOWS8,
      WV_6_3        = WV_WINDOWS8_1,
      WV_10_0       = WV_WINDOWS10
   };

   static const WinVersion WindowsVersion;
   static WinVersion windowsVersion();

#endif

#ifdef Q_OS_MAC
   enum MacVersion {
      MV_Unknown = 0x0000,

      MV_9     = 0x0001,
      MV_10_0  = 0x0002,
      MV_10_1  = 0x0003,
      MV_10_2  = 0x0004,
      MV_10_3  = 0x0005,
      MV_10_4  = 0x0006,
      MV_10_5  = 0x0007,
      MV_10_6  = 0x0008,
      MV_10_7  = 0x0009,
      MV_10_8  = 0x000A,
      MV_10_9  = 0x000B,
      MV_10_10 = 0x000C,
      MV_10_11 = 0x000D,
      MV_10_12 = 0x000E,

      MV_CHEETAH      = MV_10_0,
      MV_PUMA         = MV_10_1,
      MV_JAGUAR       = MV_10_2,
      MV_PANTHER      = MV_10_3,
      MV_TIGER        = MV_10_4,
      MV_LEOPARD      = MV_10_5,
      MV_SNOWLEOPARD  = MV_10_6,
      MV_LION         = MV_10_7,
      MV_MOUNTAINLION = MV_10_8,
      MV_MAVERICKS    = MV_10_9,
      MV_YOSEMITE     = MV_10_10,
      MV_EL_CAPITAN   = MV_10_11,
      MV_SIERRA       = MV_10_12
   };

   static const MacVersion MacintoshVersion;

#endif

   static QString buildCpuArchitecture();
   static QString machineHostName();
};

Q_CORE_EXPORT const char *qVersion();
Q_CORE_EXPORT bool qSharedBuild();

#if defined(Q_OS_MAC)
inline int qMacVersion()
{
   return QSysInfo::MacintoshVersion;
}
#endif

// not needed, used 115 times
#ifndef Q_OUTOFLINE_TEMPLATE
#  define Q_OUTOFLINE_TEMPLATE
#endif

// not needed, used over 400 times
#ifndef Q_INLINE_TEMPLATE
#  define Q_INLINE_TEMPLATE inline
#endif

// avoid "unused parameter" warnings
#  define Q_UNUSED(x) (void)x;


// Debugging and error handling
#if ! defined(QT_NO_DEBUG) && ! defined(QT_DEBUG)
#  define QT_DEBUG
#endif

#ifndef qPrintable
#  define qPrintable(string) QString(string).toLocal8Bit().constData()
#endif

Q_CORE_EXPORT QString qt_error_string(int errorCode = -1);
Q_CORE_EXPORT void    qErrnoWarning(int code, const char *msg, ...);
Q_CORE_EXPORT void    qErrnoWarning(const char *msg, ...);


Q_CORE_EXPORT void qDebug(const char *, ...)
#if defined(Q_CC_GNU) && ! defined(__INSURE__)
__attribute__ ((format (printf, 1, 2)))
#endif
;

Q_CORE_EXPORT void qWarning(const char *, ...)
#if defined(Q_CC_GNU) && ! defined(__INSURE__)
__attribute__ ((format (printf, 1, 2)))
#endif
;

Q_CORE_EXPORT void qCritical(const char *, ...)
#if defined(Q_CC_GNU) && ! defined(__INSURE__)
__attribute__ ((format (printf, 1, 2)))
#endif
;

Q_CORE_EXPORT void qFatal(const char *, ...)
#if defined(Q_CC_GNU) && ! defined(__INSURE__)
__attribute__ ((format (printf, 1, 2)))
#endif
;

inline QDebug qDebug();
inline QDebug qWarning();
inline QDebug qCritical();

#define QT_NO_QDEBUG_MACRO   while (false) qDebug
#define QT_NO_QWARNING_MACRO while (false) qWarning

//
Q_CORE_EXPORT void qt_assert(const char *assertion, const char *file, int line);

#if !defined(Q_ASSERT)
#  ifndef QT_NO_DEBUG
#    define Q_ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())
#  else
#    define Q_ASSERT(cond) qt_noop()
#  endif
#endif

#if defined(QT_NO_DEBUG) && ! defined(QT_PAINT_DEBUG)
#define QT_NO_PAINT_DEBUG
#endif

Q_CORE_EXPORT void qt_assert_x(const char *where, const char *what, const char *file, int line);

#if !defined(Q_ASSERT_X)
#  ifndef QT_NO_DEBUG
#    define Q_ASSERT_X(cond, where, what) ((!(cond)) ? qt_assert_x(where, what,__FILE__,__LINE__) : qt_noop())
#  else
#    define Q_ASSERT_X(cond, where, what) qt_noop()
#  endif
#endif

Q_CORE_EXPORT void qt_check_pointer(const char *, int);
Q_CORE_EXPORT void qBadAlloc();

#define Q_CHECK_PTR(p) do { if (!(p)) qBadAlloc(); } while (0)

template <typename T>
inline T *q_check_ptr(T *p)
{
   Q_CHECK_PTR(p);
   return p;
}

#if (defined(Q_CC_GNU) && !defined(Q_OS_SOLARIS)) || defined(Q_CC_HPACC)
#  define Q_FUNC_INFO            __PRETTY_FUNCTION__

#else
#   if defined(Q_OS_SOLARIS) || defined(Q_CC_XLC)
#      define Q_FUNC_INFO        __FILE__ "(line number unavailable)"

#   else
#       define QT_STRINGIFY2(x)  #x
#       define QT_STRINGIFY1(x)  QT_STRINGIFY2(x)

#       define Q_FUNC_INFO      __FILE__ ":" QT_STRINGIFY1(__LINE__)

#       undef QT_STRINGIFY2
#       undef QT_STRINGIFY1
#   endif

#endif

enum QtMsgType { QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtSystemMsg = QtCriticalMsg };

Q_CORE_EXPORT void qt_message_output(QtMsgType, const char *buf);

using QtMsgHandler = void (*)(QtMsgType, const char*);
Q_CORE_EXPORT QtMsgHandler qInstallMsgHandler(QtMsgHandler);


// * *
template <typename T>
class QAtomicPointer;

template <typename T>
class QGlobalStatic
{
 public:
   QAtomicPointer<T> pointer;
   bool destroyed;
};

// Created as a function-local static to delete a QGlobalStatic<T>
template <typename T>
class QGlobalStaticDeleter
{
 public:
   QGlobalStatic<T> &globalStatic;
   QGlobalStaticDeleter(QGlobalStatic<T> &_globalStatic)
      : globalStatic(_globalStatic) {
   }

   inline ~QGlobalStaticDeleter() {
      delete globalStatic.pointer.load();
      globalStatic.pointer.store(0);
      globalStatic.destroyed = true;
   }
};

#define Q_GLOBAL_STATIC_INIT(TYPE, NAME)   \
   static QGlobalStatic<TYPE> this_ ## NAME = { QAtomicPointer<TYPE>(0), false }

#define Q_GLOBAL_STATIC(TYPE, NAME)                                       \
   static TYPE *NAME()                                                    \
   {                                                                      \
   Q_GLOBAL_STATIC_INIT(TYPE, _StaticVar_);                               \
   if (!this__StaticVar_.pointer.load() && !this__StaticVar_.destroyed) { \
       TYPE *x = new TYPE;                                                \
       if (!this__StaticVar_.pointer.testAndSetOrdered(0, x))             \
      delete x;                                                           \
       else                                                               \
      static QGlobalStaticDeleter<TYPE > cleanup(this__StaticVar_);       \
   }                                                                      \
   return this__StaticVar_.pointer.load();                                \
   }

#define Q_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                       \
   static TYPE *NAME()                                                    \
   {                                                                      \
   Q_GLOBAL_STATIC_INIT(TYPE, _StaticVar_);                               \
   if (!this__StaticVar_.pointer.load() && !this__StaticVar_.destroyed) { \
       TYPE *x = new TYPE ARGS;                                           \
       if (!this__StaticVar_.pointer.testAndSetOrdered(0, x))             \
      delete x;                                                           \
       else                                                               \
      static QGlobalStaticDeleter<TYPE > cleanup(this__StaticVar_);       \
   }                                                                      \
   return this__StaticVar_.pointer.load();                                \
   }

#define Q_GLOBAL_STATIC_WITH_INITIALIZER(TYPE, NAME, INITIALIZER)         \
   static TYPE *NAME()                                                    \
   {                                                                      \
   Q_GLOBAL_STATIC_INIT(TYPE, _StaticVar_);                               \
   if (!this__StaticVar_.pointer.load() && !this__StaticVar_.destroyed) { \
       QScopedPointer<TYPE > x(new TYPE);                                 \
       INITIALIZER;                                                       \
       if (this__StaticVar_.pointer.testAndSetOrdered(0, x.data())) {     \
      static QGlobalStaticDeleter<TYPE > cleanup(this__StaticVar_);       \
      x.take();                                                           \
       }                                                                  \
   }                                                                      \
   return this__StaticVar_.pointer.load();                                \
   }

constexpr inline bool qFuzzyCompare(double p1, double p2)
{
   return (qAbs(p1 - p2) <= 0.000000000001 * qMin(qAbs(p1), qAbs(p2)));
}

constexpr inline bool qFuzzyCompare(float p1, float p2)
{
   return (qAbs(p1 - p2) <= 0.00001f * qMin(qAbs(p1), qAbs(p2)));
}

// internal
constexpr inline bool qFuzzyIsNull(double d)
{
   return qAbs(d) <= 0.000000000001;
}

// internal
constexpr inline bool qFuzzyIsNull(float f)
{
   return qAbs(f) <= 0.00001f;
}


// tests a double for a null value. It does not check whether the
// actual value is 0 or close to 0, but whether it is binary 0.
static inline bool qIsNull(double d)
{
   union U {
      double d;
      quint64 u;
   };

   U val;
   val.d = d;
   return val.u == quint64(0);
}


// tests a float for a null value, it does not check whether the
// actual value is 0 or close to 0, but whether it is binary 0.
static inline bool qIsNull(float f)
{
   union U {
      float f;
      quint32 u;
   };
   U val;
   val.f = f;
   return val.u == 0u;
}


/*
   QTypeInfo     - type trait functionality
   qIsDetached   - data sharing functionality
*/

//
template <typename T> inline bool qIsDetached(T &)
{
   return true;
}

template <typename T>
class QTypeInfo
{
 public:
   enum {
      isPointer = false,
      isComplex = true,
      isStatic  = true,
      isLarge   = (sizeof(T) > sizeof(void *)),
      isDummy   = false
   };
};

template <typename T>
class QTypeInfo<T *>
{
 public:
   enum {
      isPointer = true,
      isComplex = false,
      isStatic = false,
      isLarge = false,
      isDummy = false
   };
};

/*
   Specialize a specific type with: Q_DECLARE_TYPEINFO(type, flags);

   where 'type' is the name of the type to specialize and 'flags' is
   logically-OR'ed combination of the flags below.
*/
enum { /* TYPEINFO flags */
   Q_COMPLEX_TYPE   = 0,
   Q_PRIMITIVE_TYPE = 0x1,
   Q_STATIC_TYPE    = 0,
   Q_MOVABLE_TYPE   = 0x2,
   Q_DUMMY_TYPE     = 0x4
};

#define Q_DECLARE_TYPEINFO_BODY(TYPE, FLAGS) \
class QTypeInfo<TYPE > \
{ \
public: \
   enum { \
   isComplex = (((FLAGS) & Q_PRIMITIVE_TYPE) == 0), \
   isStatic = (((FLAGS) & (Q_MOVABLE_TYPE | Q_PRIMITIVE_TYPE)) == 0), \
   isLarge = (sizeof(TYPE)>sizeof(void*)), \
   isPointer = false, \
   isDummy = (((FLAGS) & Q_DUMMY_TYPE) != 0) \
   }; \
    static inline const char *name() { return #TYPE; } \
}

#define Q_DECLARE_TYPEINFO(TYPE, FLAGS) \
template <> \
Q_DECLARE_TYPEINFO_BODY(TYPE, FLAGS)


template <typename T>
inline void qSwap(T &value1, T &value2)
{
   using std::swap;
   swap(value1, value2);
}


#define Q_DECLARE_SHARED_STL(TYPE) \
QT_END_NAMESPACE \
namespace std { \
    template<> inline void swap<QT_PREPEND_NAMESPACE(TYPE)>(QT_PREPEND_NAMESPACE(TYPE) &value1, QT_PREPEND_NAMESPACE(TYPE) &value2) \
    { swap(value1.data_ptr(), value2.data_ptr()); } \
} \
QT_BEGIN_NAMESPACE


#define Q_DECLARE_SHARED(TYPE)    \
template <>                       \
inline bool qIsDetached<TYPE>(TYPE &t) { return t.isDetached(); } \
template <>                       \
inline void qSwap<TYPE>(TYPE &value1, TYPE &value2) \
{ qSwap(value1.data_ptr(), value2.data_ptr()); } \
Q_DECLARE_SHARED_STL(TYPE)


//  QTypeInfo primitive specializations
Q_DECLARE_TYPEINFO(signed char, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(bool,    Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(char,    Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(uchar,   Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(short,   Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(ushort,  Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(int,     Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(uint,    Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(long,    Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(ulong,   Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(float,   Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(double,  Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(qint64,  Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(quint64, Q_PRIMITIVE_TYPE);

#ifndef Q_OS_DARWIN
Q_DECLARE_TYPEINFO(long double, Q_PRIMITIVE_TYPE);
#endif

Q_CORE_EXPORT void *qMalloc(size_t size);
Q_CORE_EXPORT void *qRealloc(void *ptr, size_t size);
Q_CORE_EXPORT void *qMallocAligned(size_t size, size_t alignment);
Q_CORE_EXPORT void *qReallocAligned(void *ptr, size_t size, size_t oldsize, size_t alignment);
Q_CORE_EXPORT void qFree(void *ptr);
Q_CORE_EXPORT void qFreeAligned(void *ptr);


//  used to avoid older compiler issues
#if ! defined(QT_CC_WARNINGS)
#  define QT_NO_WARNINGS
#endif

class Q_CORE_EXPORT QFlag
{
  int i;

  public:
     inline QFlag(int i);
     inline operator int() const {
       return i;
     }
};

inline QFlag::QFlag(int ai) : i(ai) {}

class Q_CORE_EXPORT QIncompatibleFlag
{
   int i;

   public:
     inline explicit QIncompatibleFlag(int i);
     inline operator int() const {
        return i;
     }
};

inline QIncompatibleFlag::QIncompatibleFlag(int ai) : i(ai) {}


#ifndef Q_NO_TYPESAFE_FLAGS

template<typename Enum>
class QFlags
{
   typedef void **Zero;
   int i;

 public:
   typedef Enum enum_type;

   constexpr inline QFlags(const QFlags &f) : i(f.i) {}
   constexpr inline QFlags(Enum f) : i(f) {}
   constexpr inline QFlags(Zero = 0) : i(0) {}

   inline QFlags(QFlag f) : i(f) {}

   inline QFlags &operator=(const QFlags &f) {
      i = f.i;
      return *this;
   }
   inline QFlags &operator&=(int mask)  {
      i &= mask;
      return *this;
   }
   inline QFlags &operator&=(uint mask) {
      i &= mask;
      return *this;
   }
   inline QFlags &operator|=(QFlags f)  {
      i |= f.i;
      return *this;
   }
   inline QFlags &operator|=(Enum f)    {
      i |= f;
      return *this;
   }
   inline QFlags &operator^=(QFlags f)  {
      i ^= f.i;
      return *this;
   }
   inline QFlags &operator^=(Enum f)    {
      i ^= f;
      return *this;
   }

   constexpr  inline operator int() const {
      return i;
   }

   constexpr inline QFlags operator|(QFlags f) const {
      return QFlags(Enum(i | f.i));
   }

   constexpr inline QFlags operator|(Enum f) const {
      return QFlags(Enum(i | f));
   }

   constexpr inline QFlags operator^(QFlags f) const {
      return QFlags(Enum(i ^ f.i));
   }

   constexpr inline QFlags operator^(Enum f) const {
      return QFlags(Enum(i ^ f));
   }

   constexpr inline QFlags operator&(int mask) const {
      return QFlags(Enum(i & mask));
   }

   constexpr inline QFlags operator&(uint mask) const {
      return QFlags(Enum(i & mask));
   }

   constexpr inline QFlags operator&(Enum f) const {
      return QFlags(Enum(i & f));
   }

   constexpr inline QFlags operator~() const {
      return QFlags(Enum(~i));
   }

   constexpr inline bool operator!() const {
      return !i;
   }

   inline bool testFlag(Enum f) const {
      return (i & f) == f && (f != 0 || i == int(f) );
   }
};


#define Q_DECLARE_FLAGS(Flags, Enum) \
typedef QFlags<Enum> Flags;


#define Q_DECLARE_INCOMPATIBLE_FLAGS(Flags) \
inline QIncompatibleFlag operator|(Flags::enum_type f1, int f2) \
   { return QIncompatibleFlag(int(f1) | f2); }


#define Q_DECLARE_OPERATORS_FOR_FLAGS(Flags) \
constexpr inline QFlags<Flags::enum_type> operator|(Flags::enum_type f1, Flags::enum_type f2) \
   { return QFlags<Flags::enum_type>(f1) | f2; } \
constexpr inline QFlags<Flags::enum_type> operator|(Flags::enum_type f1, QFlags<Flags::enum_type> f2) \
   { return f2 | f1; } Q_DECLARE_INCOMPATIBLE_FLAGS(Flags)

#else
// Q_NO_TYPESAFE_FLAGS

#define Q_DECLARE_FLAGS(Flags, Enum) \
typedef uint Flags;
#define Q_DECLARE_OPERATORS_FOR_FLAGS(Flags)

#endif

// expands to a C++11 range based for (undocumented)
#define Q_FOREACH(variable, container)  \
for (variable : container)

#define Q_FOREVER for(;;)

#ifndef QT_NO_KEYWORDS
#  ifndef foreach
#    define foreach Q_FOREACH
#  endif

#  ifndef forever
#    define forever Q_FOREVER
#  endif
#endif

// raw pointer ( QEasingCurvePrivate, QStringMatcherPrivate, maybe a few other classes 12/28/2013 )
template <typename T>
T *qGetPtrHelper(T *ptr)
{
   return ptr;
}

// smart pointer
template <typename Wrapper>
typename Wrapper::pointer qGetPtrHelper(const Wrapper &p)
{
   return p.data();
}

#define Q_DECLARE_PRIVATE(Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(qGetPtrHelper(d_ptr)); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(qGetPtrHelper(d_ptr)); } \
    friend class Class##Private;

// used in declarative
#define Q_DECLARE_PRIVATE_D(Dptr, Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(Dptr); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(Dptr); } \
    friend class Class##Private;

#define Q_DECLARE_PUBLIC(Class)                                    \
    inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
    inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
    friend class Class;

#define Q_D(Class) Class##Private * const d = d_func()
#define Q_Q(Class) Class * const q = q_func()

#define QT_TR_NOOP(x) (x)
#define QT_TR_NOOP_UTF8(x) (x)
#define QT_TRANSLATE_NOOP(scope, x) (x)
#define QT_TRANSLATE_NOOP_UTF8(scope, x) (x)
#define QT_TRANSLATE_NOOP3(scope, x, comment) {x, comment}
#define QT_TRANSLATE_NOOP3_UTF8(scope, x, comment) {x, comment}

#ifndef QT_NO_TRANSLATION

// Defined in qcoreapplication.cpp
// The better name qTrId() is reserved for an upcoming function which would
// return a much more powerful QStringFormatter instead of a QString
Q_CORE_EXPORT QString qtTrId(const char *id, int n = -1);

#define QT_TRID_NOOP(id) id
#endif

//  Some classes do not permit copies to be made of an object. These classes contain a private
//  copy constructor and assignment operator to disable copying (the compiler gives an error message)
#define Q_DISABLE_COPY(ClassName)           \
    ClassName(const ClassName &) = delete;  \
    ClassName &operator=(const ClassName &) = delete;

Q_CORE_EXPORT QByteArray qgetenv(const char *varName);
Q_CORE_EXPORT bool       qputenv(const char *varName, const QByteArray &value);

inline int qIntCast(double f)
{
   return int(f);
}
inline int qIntCast(float f)
{
   return int(f);
}

Q_CORE_EXPORT void qsrand(uint seed);
Q_CORE_EXPORT int qrand();

#if defined (__ELF__)
#  if defined (Q_OS_LINUX) || defined (Q_OS_SOLARIS) || defined (Q_OS_FREEBSD) || defined (Q_OS_OPENBSD) || defined (Q_OS_DRAGONFLY)
#    define Q_OF_ELF
#  endif
#endif

#if ! defined(Q_OS_WIN) && ! defined(Q_OS_MAC) && ! (defined(Q_WS_X11) && ! defined(QT_NO_FREETYPE)) && ! (defined(Q_WS_QPA))
#  define QT_NO_RAWFONT
#endif

#endif      // block d

#endif
