/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QGLOBAL_H
#define QGLOBAL_H

#if ! defined (CS_DOXYPRESS)
#include <cs_build_info.h>
#endif

#include <qexport.h>
#include <qfeatures.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// usage: #if (CS_VERSION >= CS_VERSION_CHECK(1, 1, 0))
#define CS_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

#if defined(__cplusplus)

#include <qstringfwd.h>

#include <algorithm>
#include <type_traits>

#define QT_PREPEND_NAMESPACE(name)       ::name
#define QT_FORWARD_DECLARE_CLASS(name)   class name;
#define QT_MANGLE_NAMESPACE(name)        name

#endif

#define QT_BEGIN_NAMESPACE
#define QT_END_NAMESPACE

// ** detect target architecture
#if defined(__x86_64__) || defined(_M_AMD64)
// 64-bit x86
#  define QT_ARCH_X86_64
#  define Q_PROCESSOR_X86_64

#  define Q_PROCESSOR_X86      6

#elif defined(__i386__) || defined(_M_IX86)
// 32-bit x86
#  define QT_ARCH_I386
#  define Q_PROCESSOR_X86_32

#  if defined(_M_IX86)
#    define Q_PROCESSOR_X86     (_M_IX86/100)

#  elif defined(__i686__) || defined(__athlon__) || defined(__SSE__)
#    define Q_PROCESSOR_X86     6

#  elif defined(__i586__) || defined(__k6__)
#    define Q_PROCESSOR_X86     5

#  elif defined(__i486__)
#    define Q_PROCESSOR_X86     4

#  else
#    define Q_PROCESSOR_X86     3

#  endif

#else
#error Unable to detect system architecture, contact CopperSpice development

#endif

// ** detect target endianness
#if defined (__BYTE_ORDER__) && (__BYTE_ORDER__ - 0 == __ORDER_BIG_ENDIAN__ - 0 || __BYTE_ORDER__ - 0 == __ORDER_LITTLE_ENDIAN__ - 0)

#define Q_BYTE_ORDER       __BYTE_ORDER__
#define Q_BIG_ENDIAN       __ORDER_BIG_ENDIAN__
#define Q_LITTLE_ENDIAN    __ORDER_LITTLE_ENDIAN__

#elif defined (__LITTLE_ENDIAN__) || defined (QT_ARCH_X86_64) || defined (QT_ARCH_I386)

#define Q_BIG_ENDIAN 1234
#define Q_LITTLE_ENDIAN 4321
#define Q_BYTE_ORDER Q_LITTLE_ENDIAN

#elif defined (__BIG_ENDIAN__)

#define Q_BIG_ENDIAN 1234
#define Q_LITTLE_ENDIAN 4321
#define Q_BYTE_ORDER Q_BIG_ENDIAN

#else
#error Unable to detect target endianness, contact CopperSpice development

#endif


// **
#if defined(__APPLE__) && defined(__GNUC__)
#  define Q_OS_DARWIN
#  define Q_OS_BSD4

#elif defined(__ANDROID__) || defined(ANDROID)
#  define Q_OS_ANDROID
#  define Q_OS_LINUX

#elif defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#  define Q_OS_WIN32
#  define Q_OS_WIN64

#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
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
#  error "CopperSpice has not been ported to this Operating System"

#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#  define Q_OS_WIN
#endif

#if defined(Q_OS_DARWIN)

// ensure it is not defined right now
#  undef Q_OS_IOS

#endif

#if defined(Q_OS_WIN)
#  undef Q_OS_UNIX

#elif ! defined(Q_OS_UNIX)
#  define Q_OS_UNIX

#endif

// **
#if defined(Q_OS_DARWIN) && ! defined(Q_CC_INTEL)
#define QT_BEGIN_INCLUDE_HEADER     }
#define QT_END_INCLUDE_HEADER     extern "C++" {

#else
#define QT_BEGIN_INCLUDE_HEADER
#define QT_END_INCLUDE_HEADER     extern "C++"

#endif


#if defined(Q_OS_DARWIN) && ! defined(QT_LARGEFILE_SUPPORT)
#  define QT_LARGEFILE_SUPPORT 64
#endif

#ifdef Q_OS_DARWIN

#if ! defined (CS_DOXYPRESS)
#  include <AvailabilityMacros.h>
#endif

#  if ! defined(MAC_OS_X_VERSION_10_9)
#     define MAC_OS_X_VERSION_10_9  MAC_OS_X_VERSION_10_8 + 10
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

#  if ! defined(MAC_OS_X_VERSION_10_13)
#     define MAC_OS_X_VERSION_10_13 101300
#  endif

#  if ! defined(MAC_OS_X_VERSION_10_14)
#     define MAC_OS_X_VERSION_10_14 101400
#  endif

#  if ! defined(MAC_OS_X_VERSION_10_15)
#     define MAC_OS_X_VERSION_10_15 101500
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

#  if ( __clang_major__ < 6)
#    error "CopperSpice requires Clang 6 or newer"
#  endif

#  define Q_CC_CLANG
#  define Q_CC_GNU
#  define Q_C_CALLBACKS

#  define Q_FUNC_INFO       __FILE__ ":" QT_STRINGIFY1(__LINE__)

#  define Q_ALIGNOF(type)   __alignof__(type)
#  define Q_LIKELY(expr)    __builtin_expect(!!(expr), true)
#  define Q_UNLIKELY(expr)  __builtin_expect(!!(expr), false)

#  ifndef __ARM_EABI__
#     define QT_NO_ARM_EABI
#  endif


#elif defined(__GNUC__)
//  ****

#  if (__GNUC__ < 7) || (__GNUC__ == 7 && __GNUC_MINOR__ < 3)
#    error "CopperSpice requires GCC 7.3 or newer"
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

#  define Q_FUNC_INFO       __PRETTY_FUNCTION__

#  define Q_ALIGNOF(type)   __alignof__(type)
#  define Q_LIKELY(expr)    __builtin_expect(!!(expr), true)
#  define Q_UNLIKELY(expr)  __builtin_expect(!!(expr), false)

#  if (defined(Q_CC_GNU) || defined(Q_CC_INTEL))

#    ifndef __ARM_EABI__
#      define QT_NO_ARM_EABI
#    endif
#  endif


#elif defined(_MSC_VER)
//  ****

#  if _MSC_VER < 1926
#    error "CopperSpice requires Visual Studio 2019 Version 15.5 or newer"
#  endif

#  define Q_CC_MSVC         (_MSC_VER)
#  define Q_FUNC_INFO       __FUNCSIG__

#  define Q_ALIGNOF(type)   __alignof(type)
#  define Q_LIKELY(expr)    (expr)
#  define Q_UNLIKELY(expr)  (expr)


#elif defined(__PGI)
//  ****

#  define Q_CC_PGI

#  if defined(__EDG__)
#    define Q_CC_EDG
#  endif

#  define Q_FUNC_INFO      __FILE__ ":" QT_STRINGIFY1(__LINE__)


#elif defined(__EDG) || defined(__EDG__)
//  **

#  if defined(__COMO__)
#    define Q_C_CALLBACKS

#  elif defined(__INTEL_COMPILER)
#    define Q_CC_INTEL

#  elif defined(__sgi)
#    define Q_CC_MIPS

#  endif

#  define Q_FUNC_INFO      __FILE__ ":" QT_STRINGIFY1(__LINE__)

#else
//  **

#  error "CopperSpice has not been tested with this Compiler"

#endif

#ifndef Q_LIKELY
#  define Q_LIKELY(x)   (x)
#endif

#ifndef Q_UNLIKELY
#  define Q_UNLIKELY(x) (x)
#endif

#ifndef QT_STRINGIFY1
#  define QT_STRINGIFY2(x)  #x
#  define QT_STRINGIFY1(x)  QT_STRINGIFY2(x)
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
#if defined(_WIN32_X11_)
#  define Q_WS_X11

#elif defined(Q_OS_UNIX)

#  if ! defined(Q_OS_DARWIN)
#    define Q_WS_X11
#  endif

#endif

#define Q_INIT_RESOURCE_EXTERN(name) \
   extern int QT_MANGLE_NAMESPACE(qInitResources_ ## name) ();

#define Q_INIT_RESOURCE(name) \
   do { extern int QT_MANGLE_NAMESPACE(qInitResources_ ## name) ();       \
   QT_MANGLE_NAMESPACE(qInitResources_ ## name) (); } while (0)

#define Q_CLEANUP_RESOURCE(name) \
   do { extern int QT_MANGLE_NAMESPACE(qCleanupResources_ ## name) ();    \
   QT_MANGLE_NAMESPACE(qCleanupResources_ ## name) (); } while (0)

// make sure to update QVariant when changing the following

typedef int8_t               qint8;
typedef uint8_t              quint8;

typedef int16_t              qint16;
typedef uint16_t             quint16;

typedef int32_t              qint32;
typedef uint32_t             quint32;

typedef long long            qint64;
typedef unsigned long long   quint64;

#define Q_INT64_C(c)         static_cast<int64_t>(c ## LL)
#define Q_UINT64_C(c)        static_cast<uint64_t>(c ## ULL)

#ifndef QT_POINTER_SIZE
#define QT_POINTER_SIZE      sizeof(void *)
#endif

#if defined(__cplusplus)      // block c

using qintptr   = std::conditional<sizeof(void *) == 4, qint32, qint64>::type;
using qptrdiff  = qintptr;
using quintptr  = std::conditional<sizeof(void *) == 4, quint32, quint64>::type;

using uchar     = unsigned char;
using ushort    = unsigned short;
using uint      = unsigned int;
using ulong     = unsigned long;

// ****
#ifndef TRUE
#  define TRUE  true
#  define FALSE false
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

//
using qreal = double;

// utility macros and inline functions
template <typename T>
constexpr inline T qAbs(const T &t)
{
   return t >= 0 ? t : -t;
}

constexpr inline int qRound(double d)
{
   return d >= 0.0 ? int(d + 0.5) : int(d - double(int(d - 1)) + 0.5) + int(d - 1);
}

constexpr inline int qRound(float d)
{
   return d >= 0.0f ? int(d + 0.5f) : int(d - float(int(d - 1)) + 0.5f) + int(d - 1);
}

constexpr inline qint64 qRound64(double d)
{
   return d >= 0.0 ? qint64(d + 0.5) : qint64(d - double(qint64(d - 1)) + 0.5) + qint64(d - 1);
}

constexpr inline qint64 qRound64(float d)
{
   return d >= 0.0f ? qint64(d + 0.5f) : qint64(d - float(qint64(d - 1)) + 0.5f) + qint64(d - 1);
}

// enhanced to support size_type which can be 32 bit or 64 bit
// the larger data type size will be returned
template <typename T1, typename T2>
constexpr inline auto qMin(const T1 &a, const T2 &b)
{
   return (a < b) ? a : b;
}

template <typename T1, typename T2>
constexpr inline auto qMax(const T1 &a, const T2 &b)
{
   return (a < b) ? b : a;
}

template <typename T1, typename T2, typename T3>
constexpr inline auto qBound(const T1 &min, const T2 &val, const T3 &max)
{
   return qMax(min, qMin(max, val));
}

#if defined(Q_OS_DARWIN)

#  ifndef QMAC_QMENUBAR_NO_EVENT
#    define QMAC_QMENUBAR_NO_EVENT
#  endif

// implemented in qcore_mac_objc.mm
class Q_CORE_EXPORT QMacAutoReleasePool
{
   public:
      QMacAutoReleasePool();
      ~QMacAutoReleasePool();

   private:
      QMacAutoReleasePool(const QMacAutoReleasePool &) = delete;
      void *pool;
};

#endif

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

#ifdef Q_OS_DARWIN
   enum MacVersion {
      MV_Unknown = 0x0000,

      MV_10_11 = 0x000D,
      MV_10_12 = 0x000E,
      MV_10_13 = 0x000F,
      MV_10_14 = 0x0010,
      MV_10_15 = 0x0011,

      MV_EL_CAPITAN   = MV_10_11,                // supported from here
      MV_SIERRA       = MV_10_12,
      MV_HIGH_SIERRA  = MV_10_13,
      MV_MOJAVE       = MV_10_14,
      MV_CATALINA     = MV_10_15,

      MV_IOS       = 1 << 8,                     // unknown version
      MV_IOS_9_0   = MV_IOS | 9  << 4 | 0,       // 9.0
      MV_IOS_9_1   = MV_IOS | 9  << 4 | 1,
      MV_IOS_9_2   = MV_IOS | 9  << 4 | 2,
      MV_IOS_9_3   = MV_IOS | 9  << 4 | 3,
      MV_IOS_10_0  = MV_IOS | 10 << 4 | 0,
      MV_IOS_11_0  = MV_IOS | 11 << 4 | 0
   };

   static const MacVersion MacintoshVersion;
#endif

   static QString buildCpuArchitecture();
   static QString machineHostName();
};

Q_CORE_EXPORT const char *qVersion();

// avoid "unused parameter" warnings
#define Q_UNUSED(x) (void)x;

// Debugging and error handling
#if ! defined(QT_NO_DEBUG) && ! defined(QT_DEBUG)
#  define QT_DEBUG
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
   QGlobalStaticDeleter(QGlobalStatic<T> &_globalStatic)
      : globalStatic(_globalStatic) {
   }

   inline ~QGlobalStaticDeleter() {
      delete globalStatic.pointer.load();
      globalStatic.pointer.store(nullptr);
      globalStatic.destroyed = true;
   }

   QGlobalStatic<T> &globalStatic;
};

#define Q_GLOBAL_STATIC(TYPE, NAME)                                              \
   static TYPE *NAME()                                                           \
   {                                                                             \
      static QGlobalStatic<TYPE> staticVar = { QAtomicPointer<TYPE>(0), false }; \
      if (! staticVar.pointer.load() && ! staticVar.destroyed) {                 \
         TYPE *x = new TYPE;                                                     \
         if (! staticVar.pointer.testAndSetOrdered(nullptr, x)) {                \
            delete x;                                                            \
         } else {                                                                \
            static QGlobalStaticDeleter<TYPE > cleanup(staticVar);               \
         }                                                                       \
      }                                                                          \
      return staticVar.pointer.load();                                           \
   }                                                                             \

#define Q_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                              \
   static TYPE *NAME()                                                           \
   {                                                                             \
      static QGlobalStatic<TYPE> staticVar = { QAtomicPointer<TYPE>(0), false }; \
      if (! staticVar.pointer.load() && ! staticVar.destroyed) {                 \
         TYPE *x = new TYPE ARGS;                                                \
         if (! staticVar.pointer.testAndSetOrdered(nullptr, x))                  \
            delete x;                                                            \
         else                                                                    \
            static QGlobalStaticDeleter<TYPE > cleanup(staticVar);               \
      }                                                                          \
      return staticVar.pointer.load();                                           \
   }

#define Q_GLOBAL_STATIC_WITH_INITIALIZER(TYPE, NAME, INITIALIZER)                \
   static TYPE *NAME()                                                           \
   {                                                                             \
      static QGlobalStatic<TYPE> staticVar = { QAtomicPointer<TYPE>(0), false }; \
      if (! staticVar.pointer.load() && ! staticVar.destroyed) {                 \
         QScopedPointer<TYPE > x(new TYPE);                                      \
         INITIALIZER;                                                            \
         if (staticVar.pointer.testAndSetOrdered(nullptr, x.data())) {           \
            static QGlobalStaticDeleter<TYPE > cleanup(staticVar);               \
            x.take();                                                            \
         }                                                                       \
      }                                                                          \
      return staticVar.pointer.load();                                           \
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

// test a double actual value
static inline bool qIsNull(double value)
{
   return value == 0.0;
}

// tests a float to see if all the bits are zero
static inline bool qIsNull(float value)
{
   return value == 0.0f;
}

// used everywhere
template <typename T>
inline void qSwap(T &value1, T &value2)
{
   using std::swap;
   swap(value1, value2);
}

// used in QBitArray, QByteArray, QUrl, QUrlQuery, QVariant
#define Q_DECLARE_SHARED(TYPE)                        \
template <>                                           \
inline void qSwap<TYPE>(TYPE &value1, TYPE &value2)   \
{                                                     \
   qSwap(value1.data_ptr(), value2.data_ptr());       \
}                                                     \
                                                      \
inline void swap(TYPE &value1, TYPE &value2)          \
{                                                     \
   using std::swap;                                   \
   swap(value1.data_ptr(), value2.data_ptr());        \
}

Q_CORE_EXPORT void *qMalloc(size_t size);
Q_CORE_EXPORT void *qRealloc(void *ptr, size_t size);
Q_CORE_EXPORT void *qMallocAligned(size_t size, size_t alignment);
Q_CORE_EXPORT void *qReallocAligned(void *ptr, size_t size, size_t oldsize, size_t alignment);
Q_CORE_EXPORT void qFree(void *ptr);
Q_CORE_EXPORT void qFreeAligned(void *ptr);

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

template<typename E>
class QFlags
{
   public:
      using enum_type = E;
      using int_type  = std::underlying_type_t<E>;
      using uint_type = std::make_unsigned_t<int_type>;
      using sint_type = std::make_signed_t<int_type>;

      constexpr inline QFlags(const QFlags &other)
         : i(other.i)
      {}

      constexpr inline QFlags(E value)
         : i(static_cast<int_type>(value))
      {}

      constexpr inline QFlags()
         : i(0)
      {}

      constexpr inline QFlags(std::nullptr_t)
         : i(0)
      {}

      inline QFlags(QFlag flag)
         : i(flag)
      {}

      inline QFlags &operator=(const QFlags &other) {
         i = other.i;
         return *this;
      }

      inline QFlags &operator&=(sint_type mask)  {
         i &= mask;
         return *this;
      }

      inline QFlags &operator&=(uint_type mask)  {
         i &= mask;
         return *this;
      }

      inline QFlags &operator|=(QFlags other)  {
         i |= other.i;
         return *this;
      }

      inline QFlags &operator|=(E value)    {
         i |= static_cast<int_type>(value);
         return *this;
      }

      inline QFlags &operator^=(QFlags other)  {
         i ^= other.i;
         return *this;
      }

      inline QFlags &operator^=(E value)    {
         i ^= static_cast<int_type>(value);
         return *this;
      }

      constexpr inline operator int_type() const {
         return i;
      }

      constexpr inline QFlags operator|(QFlags other) const {
         return QFlags(E(i | other.i));
      }

      constexpr inline QFlags operator|(E value) const {
         return QFlags(E(i | static_cast<int_type>(value)));
      }

      constexpr inline QFlags operator^(QFlags other) const {
         return QFlags(E(i ^ other.i));
      }

      constexpr inline QFlags operator^(E value) const {
         return QFlags(E(i ^ static_cast<int_type>(value)));
      }

      constexpr inline QFlags operator&(sint_type mask) const {
         return QFlags(E(i & mask));
      }

      constexpr inline QFlags operator&(uint_type mask) const {
         return QFlags(E(i & mask));
      }

      constexpr inline QFlags operator&(E value) const {
         return QFlags(E(i & static_cast<int_type>(value)));
      }

      constexpr inline QFlags operator~() const {
         return QFlags(E(~i));
      }

      constexpr inline bool operator!() const {
         return !i;
      }

      inline bool testFlag(E value) const {
         int_type tmp = static_cast<int_type>(value);
         return (i & tmp) == tmp && (tmp != 0 || i == tmp);
      }

   private:
      int_type i;
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

// raw pointer ( QEasingCurvePrivate, maybe a few other classes 12/28/2013 )
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

#define Q_DECLARE_PUBLIC(Class)                                     \
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
// better name for qTrId() is reserved for an upcoming function which would return a more
// more powerful QStringFormatter instead of a QString

Q_CORE_EXPORT QString qtTrId(const char *id, int n = -1);

#define QT_TRID_NOOP(id) id
#endif

// copy & move constructor and copy & move assignment operator = delete
#define Q_DISABLE_COPY(ClassName)           \
    ClassName(const ClassName &) = delete;  \
    ClassName &operator=(const ClassName &) = delete;

Q_CORE_EXPORT QByteArray qgetenv(const char *varName);
Q_CORE_EXPORT bool       qputenv(const char *varName, const QByteArray &value);
Q_CORE_EXPORT bool       qunsetenv(const char *varName);

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
#  if defined (Q_OS_LINUX) || defined (Q_OS_FREEBSD) || defined (Q_OS_OPENBSD) || defined (Q_OS_DRAGONFLY)
#    define Q_OF_ELF
#  endif
#endif


#endif      // block c

#endif
