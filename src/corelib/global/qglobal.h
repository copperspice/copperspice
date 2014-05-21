/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QGLOBAL_H
#define QGLOBAL_H

#include <stddef.h>

// CopperSpice Versoin
#define CS_VERSION_STR  "1.0.0"

// CopperSpice - Version  (major << 16) + (minor << 8) + patch
#define CS_VERSION 0x010000

// useage:  #if (CS_VERSION >= CS_VERSION_CHECK(1, 0, 0))
#define CS_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

#define QT_PACKAGEDATE_STR "YYYY-MM-DD"
#define QT_PACKAGE_TAG ""

#include <QtCore/qconfig.h>

#ifdef __cplusplus
#include <algorithm>

#ifndef QT_NAMESPACE

# define QT_PREPEND_NAMESPACE(name) ::name
# define QT_USE_NAMESPACE
# define QT_BEGIN_NAMESPACE
# define QT_END_NAMESPACE
# define QT_BEGIN_INCLUDE_NAMESPACE
# define QT_END_INCLUDE_NAMESPACE
# define QT_FORWARD_DECLARE_CLASS(name) class name;
# define QT_FORWARD_DECLARE_STRUCT(name) struct name;
# define QT_MANGLE_NAMESPACE(name) name

#else

# define QT_PREPEND_NAMESPACE(name) ::QT_NAMESPACE::name
# define QT_USE_NAMESPACE using namespace ::QT_NAMESPACE;
# define QT_BEGIN_NAMESPACE namespace QT_NAMESPACE {
# define QT_END_NAMESPACE }
# define QT_BEGIN_INCLUDE_NAMESPACE }
# define QT_END_INCLUDE_NAMESPACE namespace QT_NAMESPACE {

# define QT_FORWARD_DECLARE_CLASS(name) \
    QT_BEGIN_NAMESPACE class name; QT_END_NAMESPACE \
    using QT_PREPEND_NAMESPACE(name);

# define QT_FORWARD_DECLARE_STRUCT(name) \
    QT_BEGIN_NAMESPACE struct name; QT_END_NAMESPACE \
    using QT_PREPEND_NAMESPACE(name);

# define QT_MANGLE_NAMESPACE0(x) x
# define QT_MANGLE_NAMESPACE1(a, b)  a##_##b
# define QT_MANGLE_NAMESPACE2(a, b)  QT_MANGLE_NAMESPACE1(a,b)
# define QT_MANGLE_NAMESPACE(name)   QT_MANGLE_NAMESPACE2( \
         QT_MANGLE_NAMESPACE0(name), QT_MANGLE_NAMESPACE0(QT_NAMESPACE))

namespace QT_NAMESPACE {}

# ifndef QT_NO_USING_NAMESPACE
   QT_USE_NAMESPACE
# endif

#endif      // QT_NAMESPACE

#else

# define QT_BEGIN_NAMESPACE
# define QT_END_NAMESPACE
# define QT_USE_NAMESPACE
# define QT_BEGIN_INCLUDE_NAMESPACE
# define QT_END_INCLUDE_NAMESPACE

#endif


/*
   Detect the target architecture
*/
#if defined(__x86_64__)
#define QT_ARCH_X86_64
#elif defined(__i386__)
#define QT_ARCH_I386
#else
#error Unable to detect architecture, please update above list
#endif

/*
  Detect the target endianness
*/
#if defined (__BYTE_ORDER__) && \
         (__BYTE_ORDER__ - 0 == __ORDER_BIG_ENDIAN__ - 0 || __BYTE_ORDER__ - 0 == __ORDER_LITTLE_ENDIAN__ - 0)

#define Q_BYTE_ORDER    __BYTE_ORDER__
#define Q_BIG_ENDIAN    __ORDER_BIG_ENDIAN__
#define Q_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__

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


#if defined(Q_OS_MAC) && !defined(Q_CC_INTEL)
#define QT_BEGIN_INCLUDE_HEADER }
#define QT_END_INCLUDE_HEADER extern "C++" {
#else
#define QT_BEGIN_INCLUDE_HEADER
#define QT_END_INCLUDE_HEADER extern "C++"
#endif

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  define Q_OS_DARWIN
#  define Q_OS_BSD4
#  ifdef __LP64__
#    define Q_OS_DARWIN64
#  else
#    define Q_OS_DARWIN32
#  endif

#elif !defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define Q_OS_WIN32
#  define Q_OS_WIN64

#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  define Q_OS_WIN32

#elif defined(__MWERKS__) && defined(__INTEL__)
#  define Q_OS_WIN32

#elif defined(__sun) || defined(sun)
#  define Q_OS_SOLARIS

#elif defined(hpux) || defined(__hpux)
#  define Q_OS_HPUX

#elif defined(__native_client__)
#  define Q_OS_NACL

#elif defined(__linux__) || defined(__linux)
#  define Q_OS_LINUX

#elif defined(__FreeBSD__) || defined(__DragonFly__)
#  define Q_OS_FREEBSD
#  define Q_OS_BSD4

#elif defined(__NetBSD__)
#  define Q_OS_NETBSD
#  define Q_OS_BSD4

#elif defined(__OpenBSD__)
#  define Q_OS_OPENBSD
#  define Q_OS_BSD4

#elif defined(__USLC__)                      /* all SCO platforms + UDK or OUDK */
#  define Q_OS_UNIXWARE

#elif defined(__svr4__) && defined(i386)    /* Open UNIX 8 + GCC */
#  define Q_OS_UNIXWARE

#elif defined(__MAKEDEPEND__)

#else
#  error "CopperSpice has not been ported to this OS"
#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#  define Q_OS_WIN
#endif

#if defined(Q_OS_DARWIN)
#  define Q_OS_MAC                           /* Q_OS_MAC is more clear */
#  if defined(Q_OS_DARWIN64)
#     define Q_OS_MAC64
#  elif defined(Q_OS_DARWIN32)
#     define Q_OS_MAC32
#  endif
#endif

#if defined(Q_OS_WIN)
#  undef Q_OS_UNIX
#elif ! defined(Q_OS_UNIX)
#  define Q_OS_UNIX
#endif

#if defined(Q_OS_DARWIN) && !defined(QT_LARGEFILE_SUPPORT)
#  define QT_LARGEFILE_SUPPORT 64
#endif

#ifdef Q_OS_DARWIN
#  ifdef MAC_OS_X_VERSION_MIN_REQUIRED
#    undef MAC_OS_X_VERSION_MIN_REQUIRED
#  endif

#  define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_4
#  include <AvailabilityMacros.h>

#  if !defined(MAC_OS_X_VERSION_10_3)
#     define MAC_OS_X_VERSION_10_3 MAC_OS_X_VERSION_10_2 + 1
#  endif

#  if !defined(MAC_OS_X_VERSION_10_4)
#       define MAC_OS_X_VERSION_10_4 MAC_OS_X_VERSION_10_3 + 1
#  endif

#  if !defined(MAC_OS_X_VERSION_10_5)
#       define MAC_OS_X_VERSION_10_5 MAC_OS_X_VERSION_10_4 + 1
#  endif

#  if !defined(MAC_OS_X_VERSION_10_6)
#       define MAC_OS_X_VERSION_10_6 MAC_OS_X_VERSION_10_5 + 1
#  endif

#  if !defined(MAC_OS_X_VERSION_10_7)
#       define MAC_OS_X_VERSION_10_7 MAC_OS_X_VERSION_10_6 + 1
#  endif

#  if !defined(MAC_OS_X_VERSION_10_8)
#       define MAC_OS_X_VERSION_10_8 MAC_OS_X_VERSION_10_7 + 1
#  endif

#  if !defined(MAC_OS_X_VERSION_10_9)
#       define MAC_OS_X_VERSION_10_9 MAC_OS_X_VERSION_10_8 + 1
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

#if defined(__ghs)
# define Q_OUTOFLINE_TEMPLATE inline

// the following are necessary because the GHS C++ name mangling relies on __
# define Q_CONSTRUCTOR_FUNCTION0(AFUNC) \
    static const int AFUNC ## _init_variable_ = AFUNC();
# define Q_CONSTRUCTOR_FUNCTION(AFUNC) Q_CONSTRUCTOR_FUNCTION0(AFUNC)
# define Q_DESTRUCTOR_FUNCTION0(AFUNC) \
    class AFUNC ## _dest_class_ { \
    public: \
       inline AFUNC ## _dest_class_() { } \
       inline ~ AFUNC ## _dest_class_() { AFUNC(); } \
    } AFUNC ## _dest_instance_;
# define Q_DESTRUCTOR_FUNCTION(AFUNC) Q_DESTRUCTOR_FUNCTION0(AFUNC)
#endif

// Symantec C++ is now Digital Mars
#if defined(__DMC__) || defined(__SC__)
#  define Q_CC_SYM

// "explicit" semantics implemented in 8.1e but keyword recognized since 7.5
#  if defined(__SC__) && __SC__ < 0x750
#    define Q_NO_EXPLICIT_KEYWORD
#  endif

#elif defined(__clang__)
#  if ( __clang_major__  == 3  &&  __clang_minor__ >= 2 ) ||  ( __clang_major__  >= 4  )
#    define Q_CC_CLANG

// copy of stuff from below
#    define Q_CC_GNU
#    define Q_C_CALLBACKS

#  ifdef __APPLE__
#    define Q_NO_DEPRECATED_CONSTRUCTORS
#  endif

#  define Q_ALIGNOF(type)   __alignof__(type)
#  define Q_TYPEOF(expr)    __typeof__(expr)
#  define Q_DECL_ALIGN(n)   __attribute__((__aligned__(n)))
#  define Q_LIKELY(expr)    __builtin_expect(!!(expr), true)
#  define Q_UNLIKELY(expr)  __builtin_expect(!!(expr), false)

#  define Q_PACKED __attribute__ ((__packed__))
#  define Q_NO_PACKED_REFERENCE
#  ifndef __ARM_EABI__
#    define QT_NO_ARM_EABI
#  endif

// C++11
#  define Q_COMPILER_DECLTYPE
#  define Q_COMPILER_VARIADIC_TEMPLATES
#  define Q_COMPILER_AUTO_TYPE
#  define Q_COMPILER_EXTERN_TEMPLATES
#  define Q_COMPILER_DEFAULT_DELETE_MEMBERS
#  define Q_COMPILER_CLASS_ENUM
#  define Q_COMPILER_INITIALIZER_LISTS
#  define Q_COMPILER_LAMBDA
#  define Q_COMPILER_UNICODE_STRINGS
#  define Q_COMPILER_CONSTEXPR


#  else
#    error CopperSpice requires Clang 3.2 or greater

#  endif

#elif defined(__GNUC__)
#  if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 6)
#    error CopperSpice requires GCC 4.7 or greater
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

#  ifdef __APPLE__
#    define Q_NO_DEPRECATED_CONSTRUCTORS
#  endif

#  define Q_ALIGNOF(type)   __alignof__(type)
#  define Q_TYPEOF(expr)    __typeof__(expr)
#  define Q_DECL_ALIGN(n)   __attribute__((__aligned__(n)))
#  define Q_LIKELY(expr)    __builtin_expect(!!(expr), true)
#  define Q_UNLIKELY(expr)  __builtin_expect(!!(expr), false)

#  if (defined(Q_CC_GNU) || defined(Q_CC_INTEL))
#    define Q_PACKED __attribute__ ((__packed__))
#    define Q_NO_PACKED_REFERENCE
#    ifndef __ARM_EABI__
#      define QT_NO_ARM_EABI
#    endif
#  endif

// C++11
#  define Q_COMPILER_DECLTYPE
#  define Q_COMPILER_VARIADIC_TEMPLATES
#  define Q_COMPILER_AUTO_TYPE
#  define Q_COMPILER_EXTERN_TEMPLATES
#  define Q_COMPILER_DEFAULT_DELETE_MEMBERS
#  define Q_COMPILER_CLASS_ENUM
#  define Q_COMPILER_INITIALIZER_LISTS
#  define Q_COMPILER_LAMBDA
#  define Q_COMPILER_UNICODE_STRINGS
#  define Q_COMPILER_CONSTEXPR


#elif defined(__xlC__)
#  define Q_CC_XLC
#  define Q_FULL_TEMPLATE_INSTANTIATION
#  if __xlC__ < 0x400
#    error CopperSpice requires GCC 4.7 or greater
#  elif __xlC__ >= 0x0600
#    define Q_ALIGNOF(type)     __alignof__(type)
#    define Q_TYPEOF(expr)      __typeof__(expr)
#    define Q_DECL_ALIGN(n)     __attribute__((__aligned__(n)))
#    define Q_PACKED            __attribute__((__packed__))
#  endif


#elif defined(__DECCXX) || defined(__DECC)
#  define Q_CC_DEC
#  if defined(__EDG__)
#    define Q_CC_EDG
#  endif
#  if !defined(_BOOL_EXISTS)
#    define Q_NO_BOOL_TYPE
#  endif
#  if __DECCXX_VER < 60060000
#    define Q_TYPENAME
#    define Q_BROKEN_TEMPLATE_SPECIALIZATION
#    define Q_CANNOT_DELETE_CONSTANT
#  endif
#  define Q_OUTOFLINE_TEMPLATE inline


#elif defined(__PGI)
#  define Q_CC_PGI
#  if defined(__EDG__)
#    define Q_CC_EDG
#  endif


#elif !defined(Q_OS_HPUX) && (defined(__EDG) || defined(__EDG__))
#  define Q_CC_EDG
#  if !defined(_BOOL) && !defined(__BOOL_DEFINED)
#    define Q_NO_BOOL_TYPE
#  endif
#  if defined(__COMO__)
#    define Q_CC_COMEAU
#    define Q_C_CALLBACKS
#  elif defined(__KCC)
#    define Q_CC_KAI
#  elif defined(__INTEL_COMPILER)
#    define Q_CC_INTEL
#  elif defined(__ghs)
#    define Q_CC_GHS
#  elif defined(__USLC__) && defined(__SCO_VERSION__)
#    define Q_CC_USLC
#    if !defined(__SCO_VERSION__) || (__SCO_VERSION__ < 302200010)
#      define Q_OUTOFLINE_TEMPLATE inline
#    endif

// never tested
#  elif defined(CENTERLINE_CLPP) || defined(OBJECTCENTER)
#    define Q_CC_OC

#  elif defined(sinix)
#    define Q_CC_CDS

#  elif defined(__sgi)
#    define Q_CC_MIPS
#    define Q_NO_TEMPLATE_FRIENDS
#    if defined(_COMPILER_VERSION) && (_COMPILER_VERSION >= 740)
#      define Q_OUTOFLINE_TEMPLATE inline
#      pragma set woff 3624,3625,3649 /* turn off some harmless warnings */
#    endif
#  endif

#elif defined(__SUNPRO_CC) || defined(__SUNPRO_C)
#  define Q_CC_SUN
#  if __SUNPRO_CC >= 0x500
#    if __SUNPRO_CC >= 0x590
#      define Q_ALIGNOF(type)   __alignof__(type)
#      define Q_TYPEOF(expr)    __typeof__(expr)
#      define Q_DECL_ALIGN(n)   __attribute__((__aligned__(n)))
#    endif
#    if __SUNPRO_CC >= 0x550
#      define Q_DECL_EXPORT     __global
#    endif
#    if __SUNPRO_CC < 0x5a0
#      define Q_NO_TEMPLATE_FRIENDS
#    endif
#    if !defined(_BOOL)
#      define Q_NO_BOOL_TYPE
#    endif
#    define Q_C_CALLBACKS
#  else
#    define Q_NO_BOOL_TYPE
#    define Q_NO_EXPLICIT_KEYWORD
#  endif


#elif defined(sinix)
#  define Q_CC_EDG
#  define Q_CC_CDS
#  if !defined(_BOOL)
#    define Q_NO_BOOL_TYPE
#  endif
#  define Q_BROKEN_TEMPLATE_SPECIALIZATION

#elif defined(Q_OS_HPUX)
#  if defined(__HP_aCC) || __cplusplus >= 199707L
#    define Q_NO_TEMPLATE_FRIENDS
#    define Q_CC_HPACC
#    if __HP_aCC-0 < 060000
#      define Q_DECL_EXPORT     __declspec(dllexport)
#      define Q_DECL_IMPORT     __declspec(dllimport)
#    endif
#    if __HP_aCC-0 >= 061200
#      define Q_DECL_ALIGN(n)   __attribute__((aligned(n)))
#    endif
#    if __HP_aCC-0 >= 062000
#      define Q_DECL_EXPORT     __attribute__((visibility("default")))
#      define Q_DECL_HIDDEN     __attribute__((visibility("hidden")))
#      define Q_DECL_IMPORT     Q_DECL_EXPORT
#    endif
#  else
#    define Q_CC_HP
#    define Q_NO_BOOL_TYPE
#    define Q_FULL_TEMPLATE_INSTANTIATION
#    define Q_BROKEN_TEMPLATE_SPECIALIZATION
#    define Q_NO_EXPLICIT_KEYWORD
#  endif

#else
#  error "CopperSpice has not been tested with this compiler"

#endif


#ifdef Q_CC_INTEL
#  if __INTEL_COMPILER < 1200
#    define Q_NO_TEMPLATE_FRIENDS
#  endif
#  if defined(__GXX_EXPERIMENTAL_CXX0X__) || defined(__GXX_EXPERIMENTAL_CPP0X__)
#    if __INTEL_COMPILER >= 1100
#      define Q_COMPILER_EXTERN_TEMPLATES
#      define Q_COMPILER_DECLTYPE
#    elif __INTEL_COMPILER >= 1200
#      define Q_COMPILER_VARIADIC_TEMPLATES
#      define Q_COMPILER_AUTO_TYPE
#      define Q_COMPILER_DEFAULT_DELETE_MEMBERS
#      define Q_COMPILER_CLASS_ENUM
#      define Q_COMPILER_LAMBDA
#    endif
#  endif
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
# define Q_CONSTRUCTOR_FUNCTION0(AFUNC) \
   static const int AFUNC ## __init_variable__ = AFUNC();
# define Q_CONSTRUCTOR_FUNCTION(AFUNC) Q_CONSTRUCTOR_FUNCTION0(AFUNC)
#endif

#ifndef Q_DESTRUCTOR_FUNCTION
# define Q_DESTRUCTOR_FUNCTION0(AFUNC) \
    class AFUNC ## __dest_class__ { \
    public: \
       inline AFUNC ## __dest_class__() { } \
       inline ~ AFUNC ## __dest_class__() { AFUNC(); } \
    } AFUNC ## __dest_instance__;
# define Q_DESTRUCTOR_FUNCTION(AFUNC) Q_DESTRUCTOR_FUNCTION0(AFUNC)
#endif

#ifndef Q_REQUIRED_RESULT
#  if defined(Q_CC_GNU) && !defined(Q_CC_INTEL) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#    define Q_REQUIRED_RESULT __attribute__ ((warn_unused_result))
#  else
#    define Q_REQUIRED_RESULT
#  endif
#endif



#if defined(_WIN32_X11_)
#  define Q_WS_X11

#elif defined(Q_OS_WIN32)
#  define Q_WS_WIN32
#  if defined(Q_OS_WIN64)
#    define Q_WS_WIN64
#  endif

#elif defined(Q_OS_UNIX)
#  if defined(Q_OS_MAC) && !defined(__USE_WS_X11__) && !defined(Q_WS_QWS) && !defined(Q_WS_QPA)
#    define Q_WS_MAC

//   Always use cocoa
#    define QT_MAC_USE_COCOA
#    if defined(Q_OS_MAC64)
#      define Q_WS_MAC64
#    elif defined(Q_OS_MAC32)
#      define Q_WS_MAC32
#    endif
#  elif !defined(Q_WS_QWS) && !defined(Q_WS_QPA)
#    define Q_WS_X11
#  endif
#endif


#if defined(Q_WS_WIN32)
#  define Q_WS_WIN
#endif


QT_BEGIN_NAMESPACE

// make sure to update QMetaType when changing these typedefs
#include <stdint.h>

typedef int8_t    qint8;
typedef uint8_t   quint8;
typedef int16_t   qint16;
typedef uint16_t  quint16;
typedef int32_t   qint32;
typedef uint32_t  quint32;

typedef long long           qint64;
typedef unsigned long long  quint64;

#define Q_INT64_C(c)  static_cast<long long>(c ## LL)
#define Q_UINT64_C(c) static_cast<unsigned long long>(c ## ULL)

typedef qint64     qlonglong;
typedef quint64    qulonglong;

#ifndef QT_POINTER_SIZE
#define QT_POINTER_SIZE   sizeof(void *)
#endif

#define Q_INIT_RESOURCE_EXTERN(name) \
    extern int QT_MANGLE_NAMESPACE(qInitResources_ ## name) ();

#define Q_INIT_RESOURCE(name) \
    do { extern int QT_MANGLE_NAMESPACE(qInitResources_ ## name) ();       \
        QT_MANGLE_NAMESPACE(qInitResources_ ## name) (); } while (0)

#define Q_CLEANUP_RESOURCE(name) \
    do { extern int QT_MANGLE_NAMESPACE(qCleanupResources_ ## name) ();    \
        QT_MANGLE_NAMESPACE(qCleanupResources_ ## name) (); } while (0)

#if defined(__cplusplus)

/*
quintptr and qptrdiff are guaranteed to be the same size as a pointer
      sizeof(void *) == sizeof(quintptr)
      && sizeof(void *) == sizeof(qptrdiff)
*/

template <int> struct QIntegerForSize;
template <>    struct QIntegerForSize<1> { typedef quint8  Unsigned; typedef qint8  Signed; };
template <>    struct QIntegerForSize<2> { typedef quint16 Unsigned; typedef qint16 Signed; };
template <>    struct QIntegerForSize<4> { typedef quint32 Unsigned; typedef qint32 Signed; };
template <>    struct QIntegerForSize<8> { typedef quint64 Unsigned; typedef qint64 Signed; };
template <class T> struct QIntegerForSizeof: QIntegerForSize<sizeof(T)> { };
typedef QIntegerForSizeof<void*>::Unsigned quintptr;
typedef QIntegerForSizeof<void*>::Signed qptrdiff;


QT_BEGIN_INCLUDE_NAMESPACE
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
QT_END_INCLUDE_NAMESPACE

#if defined(Q_NO_BOOL_TYPE)
#error "Compiler does not support the bool type"
#endif


#ifndef QT_LINUXBASE
#  ifndef TRUE
#   define TRUE  true
#   define FALSE false
#  endif
#endif

#define QT_STATIC_CONST       static const
#define QT_STATIC_CONST_IMPL  const

// Warnings and errors when using deprecated methods
#if (defined(Q_CC_GNU) && !defined(Q_CC_INTEL) && (__GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2)))
#  define Q_DECL_DEPRECATED __attribute__ ((__deprecated__))
#else
#  define Q_DECL_DEPRECATED
#endif

#ifndef Q_DECL_VARIABLE_DEPRECATED
#  define Q_DECL_VARIABLE_DEPRECATED Q_DECL_DEPRECATED
#endif

#ifndef Q_DECL_CONSTRUCTOR_DEPRECATED
#  if defined(Q_NO_DEPRECATED_CONSTRUCTORS)
#    define Q_DECL_CONSTRUCTOR_DEPRECATED
#  else
#    define Q_DECL_CONSTRUCTOR_DEPRECATED Q_DECL_DEPRECATED
#  endif
#endif

#if defined(QT_NO_DEPRECATED)
#  undef QT_DEPRECATED
#  undef QT_DEPRECATED_VARIABLE
#  undef QT_DEPRECATED_CONSTRUCTOR
#elif defined(QT_DEPRECATED_WARNINGS)
#  undef QT_DEPRECATED
#  define QT_DEPRECATED Q_DECL_DEPRECATED
#  undef QT_DEPRECATED_VARIABLE
#  define QT_DEPRECATED_VARIABLE Q_DECL_VARIABLE_DEPRECATED
#  undef QT_DEPRECATED_CONSTRUCTOR
#  define QT_DEPRECATED_CONSTRUCTOR explicit Q_DECL_CONSTRUCTOR_DEPRECATED
#else
#  undef QT_DEPRECATED
#  define QT_DEPRECATED
#  undef QT_DEPRECATED_VARIABLE
#  define QT_DEPRECATED_VARIABLE
#  undef QT_DEPRECATED_CONSTRUCTOR
#  define QT_DEPRECATED_CONSTRUCTOR
#endif

#ifdef QT_ASCII_CAST_WARNINGS
#  define QT_ASCII_CAST_WARN Q_DECL_DEPRECATED
#  if defined(Q_CC_GNU) && __GNUC__ < 4
     /* gcc < 4 doesn't like Q_DECL_DEPRECATED in front of constructors */
#    define QT_ASCII_CAST_WARN_CONSTRUCTOR
#  else
#    define QT_ASCII_CAST_WARN_CONSTRUCTOR Q_DECL_CONSTRUCTOR_DEPRECATED
#  endif
#else
#  define QT_ASCII_CAST_WARN
#  define QT_ASCII_CAST_WARN_CONSTRUCTOR
#endif

#if defined(__i386__) || defined(_WIN32)
#  if defined(Q_CC_GNU)
#    if !defined(Q_CC_INTEL) && ((100*(__GNUC__ - 0) + 10*(__GNUC_MINOR__ - 0) + __GNUC_PATCHLEVEL__) >= 332)
#     define QT_FASTCALL __attribute__((regparm(3)))
#    else
#     define QT_FASTCALL
#    endif
#  else
#     define QT_FASTCALL
#  endif
#else
#  define QT_FASTCALL
#endif

#ifdef Q_COMPILER_DEFAULT_DELETE_MEMBERS
# define Q_DECL_EQ_DELETE = delete
#else
# define Q_DECL_EQ_DELETE
#endif

#ifdef Q_COMPILER_CONSTEXPR
# define Q_DECL_CONSTEXPR constexpr
#else
# define Q_DECL_CONSTEXPR
#endif

//defines the type for the WNDPROC on windows
//the alignment needs to be forced for sse2 to not crash with mingw
#if defined(Q_WS_WIN)
#  if defined(Q_CC_MINGW)
#    define QT_ENSURE_STACK_ALIGNED_FOR_SSE __attribute__ ((force_align_arg_pointer))
#  else
#    define QT_ENSURE_STACK_ALIGNED_FOR_SSE
#  endif
#  define QT_WIN_CALLBACK CALLBACK QT_ENSURE_STACK_ALIGNED_FOR_SSE
#endif

typedef int QNoImplicitBoolCast;

#if defined(QT_ARCH_ARM) || defined(QT_ARCH_ARMV6) || defined(QT_ARCH_AVR32) ||     \
            (defined(QT_ARCH_MIPS) && (defined(Q_WS_QWS) || defined(Q_WS_QPA))) ||  \
             defined(QT_ARCH_SH) || defined(QT_ARCH_SH4A)

#define QT_NO_FPU
#endif

// must match both files:  qmetatype.h & qglobal.h
#if defined(QT_COORD_TYPE)
typedef QT_COORD_TYPE qreal;
#elif defined(QT_NO_FPU) || defined(QT_ARCH_ARM)
typedef float qreal;
#else
typedef double qreal;
#endif


//   Utility macros and inline functions
template <typename T>
Q_DECL_CONSTEXPR inline T qAbs(const T &t) { return t >= 0 ? t : -t; }

Q_DECL_CONSTEXPR inline int qRound(qreal d)
{ return d >= qreal(0.0) ? int(d + qreal(0.5)) : int(d - int(d-1) + qreal(0.5)) + int(d-1); }

#if defined(QT_NO_FPU) || defined(QT_ARCH_ARM)
   Q_DECL_CONSTEXPR inline qint64 qRound64(double d)
   { return d >= 0.0 ? qint64(d + 0.5) : qint64(d - qreal(qint64(d-1)) + 0.5) + qint64(d-1); }
#else
   Q_DECL_CONSTEXPR inline qint64 qRound64(qreal d)
   { return d >= qreal(0.0) ? qint64(d + qreal(0.5)) : qint64(d - qreal(qint64(d-1)) + qreal(0.5)) + qint64(d-1); }
#endif

template <typename T>
Q_DECL_CONSTEXPR inline const T &qMin(const T &a, const T &b) { return (a < b) ? a : b; }
template <typename T>
Q_DECL_CONSTEXPR inline const T &qMax(const T &a, const T &b) { return (a < b) ? b : a; }
template <typename T>
Q_DECL_CONSTEXPR inline const T &qBound(const T &min, const T &val, const T &max)
{ return qMax(min, qMin(max, val)); }



// Data stream functions are provided by many classes (defined in qdatastream.h)
class QDataStream;

#ifndef QT_BUILD_KEY
#define QT_BUILD_KEY "(copperspice)"
#endif

#if defined(Q_WS_MAC)
#  ifndef QMAC_QMENUBAR_NO_EVENT
#    define QMAC_QMENUBAR_NO_EVENT
#  endif
#endif

# include <QtCore/qfeatures.h>

#define QT_SUPPORTS(FEATURE) (!defined(QT_NO_##FEATURE))

#ifndef Q_DECL_EXPORT
#  if defined(Q_OS_WIN)
#    define Q_DECL_EXPORT __declspec(dllexport)
#  elif defined(QT_VISIBILITY_AVAILABLE)
#    define Q_DECL_EXPORT __attribute__((visibility("default")))
#    define Q_DECL_HIDDEN __attribute__((visibility("hidden")))
#  endif
#  ifndef Q_DECL_EXPORT
#    define Q_DECL_EXPORT
#  endif
#endif

#ifndef Q_DECL_IMPORT
#  if defined(Q_OS_WIN)
#    define Q_DECL_IMPORT __declspec(dllimport)
#  else
#    define Q_DECL_IMPORT
#  endif
#endif

#ifndef Q_DECL_HIDDEN
#  define Q_DECL_HIDDEN
#endif


// Create DLL if QT_DLL is defined (Windows only)
#if defined(Q_OS_WIN)
#  if defined(QT_NODLL)
#    undef QT_MAKEDLL
#    undef QT_DLL
#  elif defined(QT_MAKEDLL)        /* create a DLL library */
#    if defined(QT_DLL)
#      undef QT_DLL
#    endif
#    if defined(QT_BUILD_CORE_LIB)
#      define Q_CORE_EXPORT Q_DECL_EXPORT
#    else
#      define Q_CORE_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_GUI_LIB)
#      define Q_GUI_EXPORT Q_DECL_EXPORT
#    else
#      define Q_GUI_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_SQL_LIB)
#      define Q_SQL_EXPORT Q_DECL_EXPORT
#    else
#      define Q_SQL_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_NETWORK_LIB)
#      define Q_NETWORK_EXPORT Q_DECL_EXPORT
#    else
#      define Q_NETWORK_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_SVG_LIB)
#      define Q_SVG_EXPORT Q_DECL_EXPORT
#    else
#      define Q_SVG_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_DECLARATIVE_LIB)
#      define Q_DECLARATIVE_EXPORT Q_DECL_EXPORT
#    else
#      define Q_DECLARATIVE_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_OPENGL_LIB)
#      define Q_OPENGL_EXPORT Q_DECL_EXPORT
#    else
#      define Q_OPENGL_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_MULTIMEDIA_LIB)
#      define Q_MULTIMEDIA_EXPORT Q_DECL_EXPORT
#    else
#      define Q_MULTIMEDIA_EXPORT Q_DECL_IMPORT
#    endif  
#    if defined(QT_BUILD_XML_LIB)
#      define Q_XML_EXPORT Q_DECL_EXPORT
#    else
#      define Q_XML_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_XMLPATTERNS_LIB)
#      define Q_XMLPATTERNS_EXPORT Q_DECL_EXPORT
#    else
#      define Q_XMLPATTERNS_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_SCRIPT_LIB)
#      define Q_SCRIPT_EXPORT Q_DECL_EXPORT
#    else
#      define Q_SCRIPT_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_SCRIPTTOOLS_LIB)
#      define Q_SCRIPTTOOLS_EXPORT Q_DECL_EXPORT
#    else
#      define Q_SCRIPTTOOLS_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_CANVAS_LIB)
#      define Q_CANVAS_EXPORT Q_DECL_EXPORT
#    else
#      define Q_CANVAS_EXPORT Q_DECL_IMPORT
#    endif
#    if defined(QT_BUILD_DBUS_LIB)
#      define Q_DBUS_EXPORT Q_DECL_EXPORT
#    else
#      define Q_DBUS_EXPORT Q_DECL_IMPORT
#    endif
#    define Q_TEMPLATEDLL
#  elif defined(QT_DLL) /* use a DLL library */
#    define Q_CORE_EXPORT Q_DECL_IMPORT
#    define Q_GUI_EXPORT Q_DECL_IMPORT
#    define Q_SQL_EXPORT Q_DECL_IMPORT
#    define Q_NETWORK_EXPORT Q_DECL_IMPORT
#    define Q_SVG_EXPORT Q_DECL_IMPORT
#    define Q_DECLARATIVE_EXPORT Q_DECL_IMPORT
#    define Q_CANVAS_EXPORT Q_DECL_IMPORT
#    define Q_OPENGL_EXPORT Q_DECL_IMPORT
#    define Q_MULTIMEDIA_EXPORT Q_DECL_IMPORT
#    define Q_XML_EXPORT Q_DECL_IMPORT
#    define Q_XMLPATTERNS_EXPORT Q_DECL_IMPORT
#    define Q_SCRIPT_EXPORT Q_DECL_IMPORT
#    define Q_SCRIPTTOOLS_EXPORT Q_DECL_IMPORT
#    define Q_DBUS_EXPORT Q_DECL_IMPORT
#    define Q_TEMPLATEDLL
#  endif
#  define Q_NO_DECLARED_NOT_DEFINED
#else
#  undef QT_MAKEDLL /* ignore these for other platforms */
#  undef QT_DLL
#endif

#if !defined(Q_CORE_EXPORT)
#  if defined(QT_SHARED)
#    define Q_CORE_EXPORT Q_DECL_EXPORT
#    define Q_GUI_EXPORT Q_DECL_EXPORT
#    define Q_SQL_EXPORT Q_DECL_EXPORT
#    define Q_NETWORK_EXPORT Q_DECL_EXPORT
#    define Q_SVG_EXPORT Q_DECL_EXPORT
#    define Q_DECLARATIVE_EXPORT Q_DECL_EXPORT
#    define Q_OPENGL_EXPORT Q_DECL_EXPORT
#    define Q_MULTIMEDIA_EXPORT Q_DECL_EXPORT
#    define Q_XML_EXPORT Q_DECL_EXPORT
#    define Q_XMLPATTERNS_EXPORT Q_DECL_EXPORT
#    define Q_SCRIPT_EXPORT Q_DECL_EXPORT
#    define Q_SCRIPTTOOLS_EXPORT Q_DECL_EXPORT
#    define Q_DBUS_EXPORT Q_DECL_EXPORT
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

inline void qt_noop(void) {}

#define QT_TRY       try
#define QT_CATCH(A)  catch (A)
#define QT_THROW(A)  throw A
#define QT_RETHROW   throw

// System information
class QString;

class Q_CORE_EXPORT QSysInfo {
public:
    enum Sizes {
        WordSize = (sizeof(void *)<<3)
    };

#if defined(Q_BYTE_ORDER)
    enum Endian {
        BigEndian,
        LittleEndian

#  if Q_BYTE_ORDER == Q_BIG_ENDIAN
        , ByteOrder = BigEndian
#  elif Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        , ByteOrder = LittleEndian
#  endif
    };
#endif

#if defined(Q_WS_WIN)
    enum WinVersion {
        WV_32s      = 0x0001,
        WV_95       = 0x0002,
        WV_98       = 0x0003,
        WV_Me       = 0x0004,
        WV_DOS_based= 0x000f,

        WV_NT       = 0x0010,
        WV_2000     = 0x0020,
        WV_XP       = 0x0030,
        WV_2003     = 0x0040,
        WV_VISTA    = 0x0080,
        WV_WINDOWS7 = 0x0090,
        WV_WINDOWS8 = 0x00a0,
        WV_WINDOWS8_1 = 0x00b0,
        WV_NT_based = 0x00f0,

        WV_4_0      = WV_NT,
        WV_5_0      = WV_2000,
        WV_5_1      = WV_XP,
        WV_5_2      = WV_2003,
        WV_6_0      = WV_VISTA,
        WV_6_1      = WV_WINDOWS7,
        WV_6_2      = WV_WINDOWS8,
        WV_6_3      = WV_WINDOWS8_1
    };
    static const WinVersion WindowsVersion;
    static WinVersion windowsVersion();

#endif
#ifdef Q_OS_MAC
    enum MacVersion {
        MV_Unknown = 0x0000,
 
        MV_9 = 0x0001,
        MV_10_0 = 0x0002,
        MV_10_1 = 0x0003,
        MV_10_2 = 0x0004,
        MV_10_3 = 0x0005,
        MV_10_4 = 0x0006,
        MV_10_5 = 0x0007,
        MV_10_6 = 0x0008,
        MV_10_7 = 0x0009,
        MV_10_8 = 0x000A,
        MV_10_9 = 0x000B,
      
        MV_CHEETAH = MV_10_0,
        MV_PUMA = MV_10_1,
        MV_JAGUAR = MV_10_2,
        MV_PANTHER = MV_10_3,
        MV_TIGER = MV_10_4,
        MV_LEOPARD = MV_10_5,
        MV_SNOWLEOPARD = MV_10_6,
        MV_LION = MV_10_7,
        MV_MOUNTAINLION = MV_10_8,
        MV_MAVERICKS = MV_10_9
    };
    static const MacVersion MacintoshVersion;
#endif
};

Q_CORE_EXPORT const char *qVersion();
Q_CORE_EXPORT bool qSharedBuild();

#if defined(Q_OS_MAC)
inline int qMacVersion() { return QSysInfo::MacintoshVersion; }
#endif

#ifndef Q_OUTOFLINE_TEMPLATE
#  define Q_OUTOFLINE_TEMPLATE
#endif
#ifndef Q_INLINE_TEMPLATE
#  define Q_INLINE_TEMPLATE inline
#endif

#ifndef Q_TYPENAME
#  define Q_TYPENAME typename
#endif

/*
   Avoid "unused parameter" warnings
*/

#if defined(Q_CC_INTEL) && !defined(Q_OS_WIN)
template <typename T>
inline void qUnused(T &x) { (void)x; }
#  define Q_UNUSED(x) qUnused(x);
#else
#  define Q_UNUSED(x) (void)x;
#endif


// Debugging and error handling
#if !defined(QT_NO_DEBUG) && !defined(QT_DEBUG)
#  define QT_DEBUG
#endif

#ifndef qPrintable
#  define qPrintable(string) QString(string).toLocal8Bit().constData()
#endif

Q_CORE_EXPORT void qDebug(const char *, ...) /* print debug message */
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_CORE_EXPORT void qWarning(const char *, ...) /* print warning message */
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

class QString;
Q_CORE_EXPORT QString qt_error_string(int errorCode = -1);
Q_CORE_EXPORT void qCritical(const char *, ...) /* print critical message */
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;
Q_CORE_EXPORT void qFatal(const char *, ...) /* print fatal message and exit */
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_CORE_EXPORT void qErrnoWarning(int code, const char *msg, ...);
Q_CORE_EXPORT void qErrnoWarning(const char *msg, ...);

#if (defined(QT_NO_DEBUG_OUTPUT) || defined(QT_NO_TEXTSTREAM)) && !defined(QT_NO_DEBUG_STREAM)
#define QT_NO_DEBUG_STREAM
#endif

//
class QDebug;
class QNoDebug;

#ifndef QT_NO_DEBUG_STREAM
inline QDebug qDebug();
inline QDebug qWarning();
inline QDebug qCritical();
#else
inline QNoDebug qDebug();
#endif

#ifdef QT_NO_WARNING_OUTPUT
inline QNoDebug qWarning();
#endif

#define QT_NO_QDEBUG_MACRO while (false) qDebug
#ifdef QT_NO_DEBUG_OUTPUT
#  define qDebug QT_NO_QDEBUG_MACRO
#endif

#define QT_NO_QWARNING_MACRO while (false) qWarning
#ifdef QT_NO_WARNING_OUTPUT
#  define qWarning QT_NO_QWARNING_MACRO
#endif


Q_CORE_EXPORT void qt_assert(const char *assertion, const char *file, int line);

#if !defined(Q_ASSERT)
#  ifndef QT_NO_DEBUG
#    define Q_ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())
#  else
#    define Q_ASSERT(cond) qt_noop()
#  endif
#endif

#if defined(QT_NO_DEBUG) && !defined(QT_PAINT_DEBUG)
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
inline T *q_check_ptr(T *p) { Q_CHECK_PTR(p); return p; }

#if (defined(Q_CC_GNU) && !defined(Q_OS_SOLARIS)) || defined(Q_CC_HPACC)
#  define Q_FUNC_INFO __PRETTY_FUNCTION__

#else
#   if defined(Q_OS_SOLARIS) || defined(Q_CC_XLC)
#      define Q_FUNC_INFO __FILE__ "(line number unavailable)"
#   else       
#       define QT_STRINGIFY2(x) #x
#       define QT_STRINGIFY(x) QT_STRINGIFY2(x)
#       define Q_FUNC_INFO __FILE__ ":" QT_STRINGIFY(__LINE__)
#   endif    
#   if !defined(Q_CC_MIPS)
#       undef QT_STRINGIFY2
#       undef QT_STRINGIFY
#   endif
#endif

enum QtMsgType { QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtSystemMsg = QtCriticalMsg };

Q_CORE_EXPORT void qt_message_output(QtMsgType, const char *buf);

typedef void (*QtMsgHandler)(QtMsgType, const char *);
Q_CORE_EXPORT QtMsgHandler qInstallMsgHandler(QtMsgHandler);


// forward declaration, since qatomic.h needs qglobal.h
template <typename T> class QBasicAtomicPointer;

// POD for Q_GLOBAL_STATIC
template <typename T>
class QGlobalStatic
{
public:
    QBasicAtomicPointer<T> pointer;
    bool destroyed;
};

// Created as a function-local static to delete a QGlobalStatic<T>
template <typename T>
class QGlobalStaticDeleter
{
public:
    QGlobalStatic<T> &globalStatic;
    QGlobalStaticDeleter(QGlobalStatic<T> &_globalStatic)
        : globalStatic(_globalStatic)
    { }

    inline ~QGlobalStaticDeleter()
    {
        delete globalStatic.pointer.load();
        globalStatic.pointer.store(0);
        globalStatic.destroyed = true;
    }
};

#define Q_GLOBAL_STATIC_INIT(TYPE, NAME)   \
        static QGlobalStatic<TYPE> this_ ## NAME = { QBasicAtomicPointer<TYPE>(0), false }

#define Q_GLOBAL_STATIC(TYPE, NAME)                                           \
    static TYPE *NAME()                                                       \
    {                                                                         \
        Q_GLOBAL_STATIC_INIT(TYPE, _StaticVar_);                              \
        if (!this__StaticVar_.pointer.load() && !this__StaticVar_.destroyed) {\
            TYPE *x = new TYPE;                                               \
            if (!this__StaticVar_.pointer.testAndSetOrdered(0, x))            \
                delete x;                                                     \
            else                                                              \
                static QGlobalStaticDeleter<TYPE > cleanup(this__StaticVar_); \
        }                                                                     \
        return this__StaticVar_.pointer.load();                               \
    }

#define Q_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                           \
    static TYPE *NAME()                                                       \
    {                                                                         \
        Q_GLOBAL_STATIC_INIT(TYPE, _StaticVar_);                              \
        if (!this__StaticVar_.pointer.load() && !this__StaticVar_.destroyed) {\
            TYPE *x = new TYPE ARGS;                                          \
            if (!this__StaticVar_.pointer.testAndSetOrdered(0, x))            \
                delete x;                                                     \
            else                                                              \
                static QGlobalStaticDeleter<TYPE > cleanup(this__StaticVar_); \
        }                                                                     \
        return this__StaticVar_.pointer.load();                               \
    }

#define Q_GLOBAL_STATIC_WITH_INITIALIZER(TYPE, NAME, INITIALIZER)             \
    static TYPE *NAME()                                                       \
    {                                                                         \
        Q_GLOBAL_STATIC_INIT(TYPE, _StaticVar_);                              \
        if (!this__StaticVar_.pointer.load() && !this__StaticVar_.destroyed) {\
            QScopedPointer<TYPE > x(new TYPE);                                \
            INITIALIZER;                                                      \
            if (this__StaticVar_.pointer.testAndSetOrdered(0, x.data())) {    \
                static QGlobalStaticDeleter<TYPE > cleanup(this__StaticVar_); \
                x.take();                                                     \
            }                                                                 \
        }                                                                     \
        return this__StaticVar_.pointer.load();                               \
    }


class QBool
{
    bool b;

public:
    inline explicit QBool(bool B) : b(B) {}
    inline operator const void *() const
    { return b ? static_cast<const void *>(this) : static_cast<const void *>(0); }
};

inline bool operator==(QBool b1, bool b2)  { return !b1 == !b2; }
inline bool operator==(bool b1, QBool b2)  { return !b1 == !b2; }
inline bool operator==(QBool b1, QBool b2) { return !b1 == !b2; }
inline bool operator!=(QBool b1, bool b2)  { return !b1 != !b2; }
inline bool operator!=(bool b1, QBool b2)  { return !b1 != !b2; }
inline bool operator!=(QBool b1, QBool b2) { return !b1 != !b2; }

Q_DECL_CONSTEXPR static inline bool qFuzzyCompare(double p1, double p2)
{
    return (qAbs(p1 - p2) <= 0.000000000001 * qMin(qAbs(p1), qAbs(p2)));
}

Q_DECL_CONSTEXPR static inline bool qFuzzyCompare(float p1, float p2)
{
    return (qAbs(p1 - p2) <= 0.00001f * qMin(qAbs(p1), qAbs(p2)));
}

// internal
Q_DECL_CONSTEXPR static inline bool qFuzzyIsNull(double d)
{
    return qAbs(d) <= 0.000000000001;
}

// internal
Q_DECL_CONSTEXPR static inline bool qFuzzyIsNull(float f)
{
    return qAbs(f) <= 0.00001f;
}

/*
   This function tests a double for a null value. It doesn't
   check whether the actual value is 0 or close to 0, but whether
   it is binary 0.
*/
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

/*
   This function tests a float for a null value. It doesn't
   check whether the actual value is 0 or close to 0, but whether
   it is binary 0.
*/
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
   Compilers which follow outdated template instantiation rules
   require a class to have a comparison operator to exist when
   a QList of this type is instantiated. It's not actually
   used in the list, though. Hence the dummy implementation.
   Just in case other code relies on it we better trigger a warning
   mandating a real implementation.
*/

#ifdef Q_FULL_TEMPLATE_INSTANTIATION
#  define Q_DUMMY_COMPARISON_OPERATOR(C) \
    bool operator==(const C&) const { \
        qWarning(#C"::operator==(const "#C"&) was called"); \
        return false; \
    }
#else
#  define Q_DUMMY_COMPARISON_OPERATOR(C)
#endif


/*
   QTypeInfo     - type trait functionality
   qIsDetached   - data sharing functionality
*/

//
template <typename T> inline bool qIsDetached(T &) { return true; }

template <typename T>
class QTypeInfo
{
public:
    enum {
        isPointer = false,
        isComplex = true,
        isStatic  = true,
        isLarge   = (sizeof(T)>sizeof(void*)),
        isDummy   = false
    };
};

template <typename T>
class QTypeInfo<T*>
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
   Specialize a specific type with:

   Q_DECLARE_TYPEINFO(type, flags);

   where 'type' is the name of the type to specialize and 'flags' is
   logically-OR'ed combination of the flags below.
*/
enum { /* TYPEINFO flags */
    Q_COMPLEX_TYPE = 0,
    Q_PRIMITIVE_TYPE = 0x1,
    Q_STATIC_TYPE = 0,
    Q_MOVABLE_TYPE = 0x2,
    Q_DUMMY_TYPE = 0x4
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
template<> \
Q_DECLARE_TYPEINFO_BODY(TYPE, FLAGS)


template <typename T>
inline void qSwap(T &value1, T &value2)
{
    using std::swap;
    swap(value1, value2);
}

/*
   Specialize a shared type with:

     Q_DECLARE_SHARED(type);

   where 'type' is the name of the type to specialize.  NOTE: shared
   types must declare a 'bool isDetached(void) const;' member for this
   to work.
*/

#define Q_DECLARE_SHARED_STL(TYPE) \
QT_END_NAMESPACE \
namespace std { \
    template<> inline void swap<QT_PREPEND_NAMESPACE(TYPE)>(QT_PREPEND_NAMESPACE(TYPE) &value1, QT_PREPEND_NAMESPACE(TYPE) &value2) \
    { swap(value1.data_ptr(), value2.data_ptr()); } \
} \
QT_BEGIN_NAMESPACE


#define Q_DECLARE_SHARED(TYPE)                                          \
template <> inline bool qIsDetached<TYPE>(TYPE &t) { return t.isDetached(); } \
template <> inline void qSwap<TYPE>(TYPE &value1, TYPE &value2) \
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
#if !defined(QT_CC_WARNINGS)
#  define QT_NO_WARNINGS
#endif

class Q_CORE_EXPORT QFlag
{
    int i;

public:
    inline QFlag(int i);
    inline operator int() const { return i; }
};

inline QFlag::QFlag(int ai) : i(ai) {}

class Q_CORE_EXPORT QIncompatibleFlag
{
    int i;

public:
    inline explicit QIncompatibleFlag(int i);
    inline operator int() const { return i; }
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
    Q_DECL_CONSTEXPR inline QFlags(const QFlags &f) : i(f.i) {}
    Q_DECL_CONSTEXPR inline QFlags(Enum f) : i(f) {}
    Q_DECL_CONSTEXPR inline QFlags(Zero = 0) : i(0) {}
    inline QFlags(QFlag f) : i(f) {}

    inline QFlags &operator=(const QFlags &f) { i = f.i; return *this; }
    inline QFlags &operator&=(int mask)  { i &= mask; return *this; }
    inline QFlags &operator&=(uint mask) { i &= mask; return *this; }
    inline QFlags &operator|=(QFlags f)  { i |= f.i; return *this; }
    inline QFlags &operator|=(Enum f)    { i |= f; return *this; }
    inline QFlags &operator^=(QFlags f)  { i ^= f.i; return *this; }
    inline QFlags &operator^=(Enum f)    { i ^= f; return *this; }

    Q_DECL_CONSTEXPR  inline operator int() const { return i; }

    Q_DECL_CONSTEXPR inline QFlags operator|(QFlags f) const { return QFlags(Enum(i | f.i)); }
    Q_DECL_CONSTEXPR inline QFlags operator|(Enum f) const { return QFlags(Enum(i | f)); }
    Q_DECL_CONSTEXPR inline QFlags operator^(QFlags f) const { return QFlags(Enum(i ^ f.i)); }
    Q_DECL_CONSTEXPR inline QFlags operator^(Enum f) const { return QFlags(Enum(i ^ f)); }
    Q_DECL_CONSTEXPR inline QFlags operator&(int mask) const { return QFlags(Enum(i & mask)); }
    Q_DECL_CONSTEXPR inline QFlags operator&(uint mask) const { return QFlags(Enum(i & mask)); }
    Q_DECL_CONSTEXPR inline QFlags operator&(Enum f) const { return QFlags(Enum(i & f)); }
    Q_DECL_CONSTEXPR inline QFlags operator~() const { return QFlags(Enum(~i)); }

    Q_DECL_CONSTEXPR inline bool operator!() const { return !i; }

    inline bool testFlag(Enum f) const { return (i & f) == f && (f != 0 || i == int(f) ); }
};

#define Q_DECLARE_FLAGS(Flags, Enum)\
typedef QFlags<Enum> Flags;

#define Q_DECLARE_INCOMPATIBLE_FLAGS(Flags) \
inline QIncompatibleFlag operator|(Flags::enum_type f1, int f2) \
   { return QIncompatibleFlag(int(f1) | f2); }

#define Q_DECLARE_OPERATORS_FOR_FLAGS(Flags) \
Q_DECL_CONSTEXPR inline QFlags<Flags::enum_type> operator|(Flags::enum_type f1, Flags::enum_type f2) \
   { return QFlags<Flags::enum_type>(f1) | f2; } \
Q_DECL_CONSTEXPR inline QFlags<Flags::enum_type> operator|(Flags::enum_type f1, QFlags<Flags::enum_type> f2) \
   { return f2 | f1; } Q_DECLARE_INCOMPATIBLE_FLAGS(Flags)

#else /* Q_NO_TYPESAFE_FLAGS */

#define Q_DECLARE_FLAGS(Flags, Enum)\
typedef uint Flags;
#define Q_DECLARE_OPERATORS_FOR_FLAGS(Flags)

#endif /* Q_NO_TYPESAFE_FLAGS */


#if defined(Q_CC_GNU) && ! defined(Q_CC_INTEL)
template <typename T>
class QForeachContainer {
public:
    inline QForeachContainer(const T& t) : c(t), brk(0), i(c.begin()), e(c.end()) { }
    const T c;
    int brk;
    typename T::const_iterator i, e;
};

#define Q_FOREACH(variable, container)                                \
for (QForeachContainer<__typeof__(container)> _container_(container); \
     !_container_.brk && _container_.i != _container_.e;              \
     __extension__  ({ ++_container_.brk; ++_container_.i; }))        \
    for (variable = *_container_.i;; __extension__ ({--_container_.brk; break;}))

#else

struct QForeachContainerBase {};

template <typename T>
class QForeachContainer : public QForeachContainerBase {
public:
    inline QForeachContainer(const T& t): c(t), brk(0), i(c.begin()), e(c.end()){};
    const T c;
    mutable int brk;
    mutable typename T::const_iterator i, e;
    inline bool condition() const { return (!brk++ && i != e); }
};

template <typename T> 
inline T *qForeachPointer(const T &)
   { return 0; }

template <typename T> 
inline QForeachContainer<T> qForeachContainerNew(const T& t)
   { return QForeachContainer<T>(t); }

template <typename T>
inline const QForeachContainer<T> *qForeachContainer(const QForeachContainerBase *base, const T *)
   { return static_cast<const QForeachContainer<T> *>(base); }


#if defined(Q_CC_MIPS)

#  define Q_FOREACH(variable,container)                                                             \
    if(0){}else                                                                                     \
    for (const QForeachContainerBase &_container_ = qForeachContainerNew(container);                \
         qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->condition();       \
         ++qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i)               \
        for (variable = *qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i; \
             qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk;           \
             --qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk)

#else
#  define Q_FOREACH(variable, container) \
    for (const QForeachContainerBase &_container_ = qForeachContainerNew(container); \
         qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->condition();       \
         ++qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i)               \
        for (variable = *qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i; \
             qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk;           \
             --qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk)
#endif //  MIPSpro

#endif


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
// return a much more powerful QStringFormatter instead of a QString.
Q_CORE_EXPORT QString qtTrId(const char *id, int n = -1);

#define QT_TRID_NOOP(id) id
#endif


/*
   Some classes do not permit copies to be made of an object. These
   classes contains a private copy constructor and assignment
   operator to disable copying (the compiler gives an error message).
*/
#define Q_DISABLE_COPY(Class) \
    Class(const Class &) Q_DECL_EQ_DELETE;\
    Class &operator=(const Class &) Q_DECL_EQ_DELETE;

class QByteArray;
Q_CORE_EXPORT QByteArray qgetenv(const char *varName);
Q_CORE_EXPORT bool qputenv(const char *varName, const QByteArray& value);

inline int qIntCast(double f) { return int(f); }
inline int qIntCast(float f) { return int(f); }

Q_CORE_EXPORT void qsrand(uint seed);
Q_CORE_EXPORT int qrand();


#if defined (__ELF__)
#  if defined (Q_OS_LINUX) || defined (Q_OS_SOLARIS) || defined (Q_OS_FREEBSD) || defined (Q_OS_OPENBSD)
#    define Q_OF_ELF
#  endif
#endif

#if ! defined(Q_WS_WIN) \
    && !(defined(Q_WS_MAC) && defined(QT_MAC_USE_COCOA)) \
    && !(defined(Q_WS_X11) && !defined(QT_NO_FREETYPE))  \
    && !(defined(Q_WS_QPA))
#  define QT_NO_RAWFONT
#endif

namespace QtPrivate {
//like std::enable_if
template <bool B, typename T = void> struct QEnableIf;
template <typename T> struct QEnableIf<true, T> { typedef T Type; };
}

QT_END_NAMESPACE

#endif /* __cplusplus */

#endif /* QGLOBAL_H */
