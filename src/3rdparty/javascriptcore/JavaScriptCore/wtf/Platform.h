/*
 * Copyright (C) 2006, 2007, 2008, 2009 Apple Inc.  All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WTF_Platform_h
#define WTF_Platform_h

/* ==== PLATFORM handles OS, operating environment, graphics API, and
   CPU. This macro will be phased out in favor of platform adaptation
   macros, policy decision macros, and top-level port definitions. ==== */
#define PLATFORM(WTF_FEATURE) (defined WTF_PLATFORM_##WTF_FEATURE  && WTF_PLATFORM_##WTF_FEATURE)


/* ==== Platform adaptation macros: these describe properties of the target environment. ==== */

/* COMPILER() - the compiler being used to build the project */
#define COMPILER(WTF_FEATURE) (defined WTF_COMPILER_##WTF_FEATURE  && WTF_COMPILER_##WTF_FEATURE)
/* CPU() - the target CPU architecture */
#define CPU(WTF_FEATURE) (defined WTF_CPU_##WTF_FEATURE  && WTF_CPU_##WTF_FEATURE)
/* HAVE() - specific system features (headers, functions or similar) that are present or not */
#define HAVE(WTF_FEATURE) (defined HAVE_##WTF_FEATURE  && HAVE_##WTF_FEATURE)
/* OS() - underlying operating system; only to be used for mandated low-level services like
   virtual memory, not to choose a GUI toolkit */
#define OS(WTF_FEATURE) (defined WTF_OS_##WTF_FEATURE  && WTF_OS_##WTF_FEATURE)


/* ==== Policy decision macros: these define policy choices for a particular port. ==== */

/* USE() - use a particular third-party library or optional OS service */
#define USE(WTF_FEATURE) (defined WTF_USE_##WTF_FEATURE  && WTF_USE_##WTF_FEATURE)
/* ENABLE() - turn on a specific feature of WebKit */
#define ENABLE(WTF_FEATURE) (defined ENABLE_##WTF_FEATURE  && ENABLE_##WTF_FEATURE)



/* ==== COMPILER() - the compiler being used to build the project ==== */

/* COMPILER(MSVC) Microsoft Visual C++ */
/* COMPILER(MSVC7) Microsoft Visual C++ v7 or lower*/
#if defined(_MSC_VER)
#define WTF_COMPILER_MSVC 1
#if _MSC_VER < 1400
#define WTF_COMPILER_MSVC7 1
#endif
#endif

/* COMPILER(RVCT)  - ARM RealView Compilation Tools */
#if defined(__CC_ARM) || defined(__ARMCC__)
#define WTF_COMPILER_RVCT 1
#endif

/* COMPILER(GCC) - GNU Compiler Collection */
/* --gnu option of the RVCT compiler also defines __GNUC__ */
#if defined(__GNUC__) && !COMPILER(RVCT)
#define WTF_COMPILER_GCC 1
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

/* COMPILER(MINGW) - MinGW GCC */
/* COMPILER(MINGW64) - mingw-w64 GCC - only used as additional check to exclude mingw.org specific functions */
#if defined(__MINGW32__)
#define WTF_COMPILER_MINGW 1
#include <_mingw.h> /* private MinGW header */
    #if defined(__MINGW64_VERSION_MAJOR) /* best way to check for mingw-w64 vs mingw.org */
        #define WTF_COMPILER_MINGW64 1
    #endif /* __MINGW64_VERSION_MAJOR */
#endif /* __MINGW32__ */

/* COMPILER(SUNCC) - Sun CC compiler, also known as Sun Studio or Sun Pro */
#if defined(__SUNPRO_CC) || defined(__SUNPRO_C)
#define WTF_COMPILER_SUNCC 1
#endif

/* COMPILER(INTEL) - Intel C++ Compiler */
#if defined(__INTEL_COMPILER)
#define WTF_COMPILER_INTEL 1
#endif


/* ==== CPU() - the target CPU architecture ==== */

/* This also defines CPU(BIG_ENDIAN) or CPU(MIDDLE_ENDIAN) or neither, as appropriate. */

/* CPU(ALPHA) - DEC Alpha */
#if defined(__alpha__)
#define WTF_CPU_ALPHA 1
#endif

/* CPU(IA64) - Itanium / IA-64 */
#if defined(__ia64__) || defined(__ia64) || defined(_M_IA64)
#define WTF_CPU_IA64 1

/* 32-bit mode on Itanium */
#if !defined(__LP64__)
#define WTF_CPU_IA64_32 1
#endif

/* Itanium can be both big- and little-endian;
   we need to determine at compile time which one it is.
    - HP's aCC compiler only compiles big-endian (so HP-UXi is always big-endian)
    - GCC defines __BIG_ENDIAN__ for us (default on HP-UX)
    - Linux is usually little-endian
    - I've never seen AIX or Windows on IA-64, but they should be little-endian too
*/
#if defined(__BIG_ENDIAN__) || defined(__HP_aCC)
# define WTF_CPU_BIG_ENDIAN 1
#endif
#endif

/* CPU(HPPA) - a.k.a. PA-RISC */
#if defined(__hppa) || defined(__hppa__)
#define WTF_CPU_HPPA 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(PPC) - PowerPC 32-bit */
#if   defined(__ppc__)     \
    || defined(__PPC__)     \
    || defined(__powerpc__) \
    || defined(__powerpc)   \
    || defined(__POWERPC__) \
    || defined(_M_PPC)      \
    || defined(__PPC)
#define WTF_CPU_PPC 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(PPC64) - PowerPC 64-bit */
#if   defined(__ppc64__) \
    || defined(__PPC64__)
#define WTF_CPU_PPC64 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(SH4) - SuperH SH-4 */
#if defined(__SH4__)
#define WTF_CPU_SH4 1
#endif

/* CPU(SPARC32) - SPARC 32-bit */
#if defined(__sparc) && !defined(__arch64__) || defined(__sparcv8)
#define WTF_CPU_SPARC32 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(SPARC64) - SPARC 64-bit */
#if defined(__sparc__) && defined(__arch64__) || defined (__sparcv9)
#define WTF_CPU_SPARC64 1
#define WTF_CPU_BIG_ENDIAN 1
#endif

/* CPU(SPARC) - any SPARC, true for CPU(SPARC32) and CPU(SPARC64) */
#if CPU(SPARC32) || CPU(SPARC64)
#define WTF_CPU_SPARC 1
#endif

/* CPU(X86) - i386 / x86 32-bit */
#if   defined(__i386__) \
    || defined(i386)     \
    || defined(_M_IX86)  \
    || defined(_X86_)    \
    || defined(__THW_INTEL)
#define WTF_CPU_X86 1
#endif

/* CPU(X86_64) - AMD64 / Intel64 / x86_64 64-bit */
#if   defined(__x86_64__) \
    || defined(_M_X64)
#define WTF_CPU_X86_64 1
#endif

/* 64-bit mode on AIX */
#ifdef __64BIT__
#define WTF_CPU_AIX64 1
#endif

/* CPU(AARCH64) - AArch64 */
#if defined(__aarch64__)
#define WTF_CPU_AARCH64 1
#if defined(__AARCH64EB__)
#define WTF_CPU_BIG_ENDIAN 1
#endif
#endif

/* CPU(ARM) - ARM, any version*/
#if   defined(arm) \
    || defined(__arm__) \
    || defined(__MARM__)
#define WTF_CPU_ARM 1

#if defined(__ARMEB__)
#define WTF_CPU_BIG_ENDIAN 1

#elif !defined(__ARM_EABI__) \
    && !defined(__EABI__) \
    && !defined(__VFP_FP__) \
    && !defined(ANDROID)
#define WTF_CPU_MIDDLE_ENDIAN 1

#endif

#define WTF_ARM_ARCH_AT_LEAST(N) (CPU(ARM) && WTF_ARM_ARCH_VERSION >= N)

/* Set WTF_ARM_ARCH_VERSION */
#if   defined(__ARM_ARCH_4__) \
    || defined(__ARM_ARCH_4T__) \
    || defined(__MARM_ARMV4__) \
    || defined(_ARMV4I_)
#define WTF_ARM_ARCH_VERSION 4

#elif defined(__ARM_ARCH_5__) \
    || defined(__ARM_ARCH_5T__) \
    || defined(__ARM_ARCH_5E__) \
    || defined(__ARM_ARCH_5TE__) \
    || defined(__ARM_ARCH_5TEJ__) \
    || defined(__MARM_ARMV5__)
#define WTF_ARM_ARCH_VERSION 5

#elif defined(__ARM_ARCH_6__) \
    || defined(__ARM_ARCH_6J__) \
    || defined(__ARM_ARCH_6K__) \
    || defined(__ARM_ARCH_6Z__) \
    || defined(__ARM_ARCH_6ZK__) \
    || defined(__ARM_ARCH_6T2__) \
    || defined(__ARMV6__)
#define WTF_ARM_ARCH_VERSION 6

#elif defined(__ARM_ARCH_7A__) \
    || defined(__ARM_ARCH_7R__)
#define WTF_ARM_ARCH_VERSION 7

/* RVCT sets _TARGET_ARCH_ARM */
#elif defined(__TARGET_ARCH_ARM)
#define WTF_ARM_ARCH_VERSION __TARGET_ARCH_ARM

#else
#define WTF_ARM_ARCH_VERSION 0

#endif

/* Set WTF_THUMB_ARCH_VERSION */
#if   defined(__ARM_ARCH_4T__)
#define WTF_THUMB_ARCH_VERSION 1

#elif defined(__ARM_ARCH_5T__) \
    || defined(__ARM_ARCH_5TE__) \
    || defined(__ARM_ARCH_5TEJ__)
#define WTF_THUMB_ARCH_VERSION 2

#elif defined(__ARM_ARCH_6J__) \
    || defined(__ARM_ARCH_6K__) \
    || defined(__ARM_ARCH_6Z__) \
    || defined(__ARM_ARCH_6ZK__) \
    || defined(__ARM_ARCH_6M__)
#define WTF_THUMB_ARCH_VERSION 3

#elif defined(__ARM_ARCH_6T2__) \
    || defined(__ARM_ARCH_7__) \
    || defined(__ARM_ARCH_7A__) \
    || defined(__ARM_ARCH_7R__) \
    || defined(__ARM_ARCH_7M__)
#define WTF_THUMB_ARCH_VERSION 4

/* RVCT sets __TARGET_ARCH_THUMB */
#elif defined(__TARGET_ARCH_THUMB)
#define WTF_THUMB_ARCH_VERSION __TARGET_ARCH_THUMB

#else
#define WTF_THUMB_ARCH_VERSION 0
#endif


/* CPU(ARMV5_OR_LOWER) - ARM instruction set v5 or earlier */
/* On ARMv5 and below the natural alignment is required.
   And there are some other differences for v5 or earlier. */
#if !defined(ARMV5_OR_LOWER) && !WTF_ARM_ARCH_AT_LEAST(6)
#define WTF_CPU_ARMV5_OR_LOWER 1
#endif


/* CPU(ARM_TRADITIONAL) - Thumb2 is not available, only traditional ARM (v4 or greater) */
/* CPU(ARM_THUMB2) - Thumb2 instruction set is available */
/* Only one of these will be defined. */
#if !defined(WTF_CPU_ARM_TRADITIONAL) && !defined(WTF_CPU_ARM_THUMB2)
#  if defined(thumb2) || defined(__thumb2__) \
    || ((defined(__thumb) || defined(__thumb__)) && WTF_THUMB_ARCH_VERSION == 4)
#    define WTF_CPU_ARM_TRADITIONAL 0
#    define WTF_CPU_ARM_THUMB2 1
#  elif WTF_ARM_ARCH_AT_LEAST(4)
#    define WTF_CPU_ARM_TRADITIONAL 1
#    define WTF_CPU_ARM_THUMB2 0
#  else
#    error "Not supported ARM architecture"
#  endif
#elif CPU(ARM_TRADITIONAL) && CPU(ARM_THUMB2) /* Sanity Check */
#  error "Cannot use both of WTF_CPU_ARM_TRADITIONAL and WTF_CPU_ARM_THUMB2 platforms"
#endif /* !defined(WTF_CPU_ARM_TRADITIONAL) && !defined(WTF_CPU_ARM_THUMB2) */

#endif /* ARM */

/* CPU(MIPS) - MIPS, any version */
#if (defined(mips) || defined(__mips__) || defined(MIPS) || defined(_MIPS_))
#define WTF_CPU_MIPS 1
#include <sgidefs.h>
#if defined(__MIPSEB__)
#define WTF_CPU_BIG_ENDIAN 1
#endif
/* CPU(MIPS64) - MIPS 64-bit both BIG and LITTLE endian */
#if defined(_MIPS_SIM_ABI64) && (_MIPS_SIM == _MIPS_SIM_ABI64)
#define WTF_CPU_MIPS64 1
#endif

/* CPU(MIPSN32) - MIPS N32 ABI both BIG and LITTLE endian */
#if defined(_MIPS_SIM_ABIN32) && (_MIPS_SIM == _MIPS_SIM_ABIN32)
#define WTF_CPU_MIPSN32 1
#endif

/* CPU(MIPS32) - MIPS O32 ABI both BIG and LITTLE endian */
#if defined(_MIPS_SIM_ABI32) && (_MIPS_SIM == _MIPS_SIM_ABI32)
#define WTF_CPU_MIPS32 1
#endif
#endif /* __mips__ */


/* ==== OS() - underlying operating system; only to be used for mandated low-level services like
   virtual memory, not to choose a GUI toolkit ==== */

/* OS(ANDROID) - Android */
#ifdef ANDROID
#define WTF_OS_ANDROID 1
#endif

/* OS(AIX) - AIX */
#ifdef _AIX
#define WTF_OS_AIX 1
#endif

/* OS(DARWIN) - Any Darwin-based OS, including Mac OS X and iPhone OS */
#ifdef __APPLE__
#define WTF_OS_DARWIN 1

/* FIXME: BUILDING_ON_.., and TARGETING... macros should be folded into the OS() system */
#include <AvailabilityMacros.h>
#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
#define BUILDING_ON_TIGER 1
#elif !defined(MAC_OS_X_VERSION_10_6) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_6
#define BUILDING_ON_LEOPARD 1
#elif !defined(MAC_OS_X_VERSION_10_7) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7
#define BUILDING_ON_SNOW_LEOPARD 1
#endif
#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
#define TARGETING_TIGER 1
#elif !defined(MAC_OS_X_VERSION_10_6) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
#define TARGETING_LEOPARD 1
#elif !defined(MAC_OS_X_VERSION_10_7) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_7
#define TARGETING_SNOW_LEOPARD 1
#endif
#include <TargetConditionals.h>

#endif

/* OS(IPHONE_OS) - iPhone OS */
/* OS(MAC_OS_X) - Mac OS X (not including iPhone OS) */
#if OS(DARWIN) && ((defined(TARGET_OS_EMBEDDED) && TARGET_OS_EMBEDDED)  \
    || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)                   \
    || (defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR))
#define WTF_OS_IPHONE_OS 1
#elif OS(DARWIN) && defined(TARGET_OS_MAC) && TARGET_OS_MAC
#define WTF_OS_MAC_OS_X 1
#endif

/* OS(FREEBSD) - FreeBSD */
#if defined (__FreeBSD__) || defined (__DragonFly__)
#define WTF_OS_FREEBSD 1
#endif

/* OS(HAIKU) - Haiku */
#ifdef __HAIKU__
#define WTF_OS_HAIKU 1
#endif

/* OS(HPUX) - HP-UX */
#if defined(hpux) || defined(__hpux)
#define WTF_OS_HPUX 1
#ifndef MAP_ANON
#define MAP_ANON MAP_ANONYMOUS
#endif
#endif

/* OS(LINUX) - Linux */
#ifdef __linux__
#define WTF_OS_LINUX 1
#endif

/* OS(NETBSD) - NetBSD */
#if defined(__NetBSD__)
#define WTF_PLATFORM_NETBSD 1
#endif

/* OS(OPENBSD) - OpenBSD */
#ifdef __OpenBSD__
#define WTF_OS_OPENBSD 1
#endif

/* OS(QNX) - QNX */
#if defined(__QNXNTO__)
#define WTF_OS_QNX 1
#endif

/* OS(SOLARIS) - Solaris */
#if defined(sun) || defined(__sun)
#define WTF_OS_SOLARIS 1
#endif

/* OS(WINCE) - Windows CE; note that for this platform OS(WINDOWS) is also defined */
#if defined(_WIN32_WCE)
#define WTF_OS_WINCE 1
#endif

/* OS(WINDOWS) - Any version of Windows */
#if defined(WIN32) || defined(_WIN32)
#define WTF_OS_WINDOWS 1
#endif

/* OS(SYMBIAN) - Symbian */
#if defined (__SYMBIAN32__)
/* we are cross-compiling, it is not really windows */
#undef WTF_OS_WINDOWS
#undef WTF_PLATFORM_WIN
#define WTF_OS_SYMBIAN 1
#endif

/* OS(UNIX) - Any Unix-like system */
#if   OS(AIX)              \
    || OS(ANDROID)          \
    || OS(DARWIN)           \
    || OS(FREEBSD)          \
    || OS(HAIKU)            \
    || OS(HPUX)             \
    || OS(LINUX)            \
    || OS(NETBSD)           \
    || OS(OPENBSD)          \
    || OS(QNX)              \
    || OS(SOLARIS)          \
    || OS(SYMBIAN)          \
    || defined(unix)        \
    || defined(__unix)      \
    || defined(__unix__)
#define WTF_OS_UNIX 1
#endif

/* Operating environments */

/* FIXME: these are all mixes of OS, operating environment and policy choices. */
/* PLATFORM(CHROMIUM) */
/* PLATFORM(QT) */
/* PLATFORM(WX) */
/* PLATFORM(GTK) */
/* PLATFORM(HAIKU) */
/* PLATFORM(MAC) */
/* PLATFORM(WIN) */

#if defined(BUILDING_CHROMIUM__)
#define WTF_PLATFORM_CHROMIUM 1

#elif defined(BUILDING_CS)
#define WTF_PLATFORM_QT 1

#elif defined(BUILDING_GTK__)
#define WTF_PLATFORM_GTK 1

#elif OS(DARWIN)
#define WTF_PLATFORM_MAC 1

#elif OS(WINDOWS)
#define WTF_PLATFORM_WIN 1
#endif

/* PLATFORM(IPHONE) */
/* FIXME: this is sometimes used as an OS switch and sometimes for higher-level things */
#if (defined(TARGET_OS_EMBEDDED) && TARGET_OS_EMBEDDED) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
#define WTF_PLATFORM_IPHONE 1
#endif

/* PLATFORM(IPHONE_SIMULATOR) */
#if defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR
#define WTF_PLATFORM_IPHONE 1
#define WTF_PLATFORM_IPHONE_SIMULATOR 1
#else
#define WTF_PLATFORM_IPHONE_SIMULATOR 0
#endif

#if !defined(WTF_PLATFORM_IPHONE)
#define WTF_PLATFORM_IPHONE 0
#endif

/* PLATFORM(ANDROID) */
/* FIXME: this is sometimes used as an OS() switch, and other times to drive
   policy choices */
#if defined(ANDROID)
#define WTF_PLATFORM_ANDROID 1
#endif

/* Graphics engines */

/* PLATFORM(CG) and PLATFORM(CI) */
#if PLATFORM(MAC) || PLATFORM(IPHONE)
#define WTF_PLATFORM_CG 1
#endif
#if PLATFORM(MAC) && !PLATFORM(IPHONE)
#define WTF_PLATFORM_CI 1
#endif

/* PLATFORM(SKIA) for Win/Linux, CG/CI for Mac */
#if PLATFORM(CHROMIUM)
#define ENABLE_HISTORY_ALWAYS_ASYNC 1
#if OS(DARWIN)
#define WTF_PLATFORM_CG 1
#define WTF_PLATFORM_CI 1
#define WTF_USE_ATSUI 1
#define WTF_USE_CORE_TEXT 1
#else
#define WTF_PLATFORM_SKIA 1
#endif
#endif

#if PLATFORM(GTK)
#define WTF_PLATFORM_CAIRO 1
#endif


/* OS(WINCE) && PLATFORM(QT)
   We can not determine the endianess at compile time. For
   Qt for Windows CE the endianess is specified in the
   device specific makespec
*/
#if OS(WINCE) && PLATFORM(QT)
#   include <QtGlobal>
#   undef WTF_CPU_BIG_ENDIAN
#   undef WTF_CPU_MIDDLE_ENDIAN
#   if Q_BYTE_ORDER == Q_BIG_ENDIAN
#       define WTF_CPU_BIG_ENDIAN 1
#   endif

#if (_WIN32_WCE >= 0x700)
	// Windows Embedded Compact 7 is missing std::ptrdiff_t type and std::min and std::max.
	// They are defined here to minimize the changes to JSCore
	namespace std {
		typedef ::ptrdiff_t ptrdiff_t;
		template <class T> inline T max(const T& a, const T& b) { return (a > b) ? a : b; }
		template <class T> inline T min(const T& a, const T& b) { return (a < b) ? a : b; }
	}
#endif

// For localtime in Windows CE
#   include <ce_time.h>
#endif

#if (PLATFORM(IPHONE) || PLATFORM(MAC) || PLATFORM(WIN) || (PLATFORM(QT) && OS(DARWIN) && !ENABLE(SINGLE_THREADED))) && !defined(ENABLE_JSC_MULTIPLE_THREADS)
#define ENABLE_JSC_MULTIPLE_THREADS 1
#endif

/* On Windows, use QueryPerformanceCounter by default */
#if OS(WINDOWS)
#define WTF_USE_QUERY_PERFORMANCE_COUNTER  1
#endif

#if OS(WINCE) && !PLATFORM(QT)
#undef ENABLE_JSC_MULTIPLE_THREADS
#define ENABLE_JSC_MULTIPLE_THREADS        0
#define USE_SYSTEM_MALLOC                  0
#define ENABLE_ICONDATABASE                0
#define ENABLE_JAVASCRIPT_DEBUGGER         0
#define ENABLE_FTPDIR                      0
#define ENABLE_PAN_SCROLLING               0
#define ENABLE_WML                         1
#define HAVE_ACCESSIBILITY                 0

#define NOMINMAX       /* Windows min and max conflict with standard macros */
#define NOSHLWAPI      /* shlwapi.h not available on WinCe */

/* MSDN documentation says these functions are provided with uspce.lib.  But we cannot find this file. */
#define __usp10__      /* disable "usp10.h" */

#define _INC_ASSERT    /* disable "assert.h" */
#define assert(x)

/* _countof is only included in CE6; for CE5 we need to define it ourself */
#ifndef _countof
#define _countof(x) (sizeof(x) / sizeof((x)[0]))
#endif

#endif  /* OS(WINCE) && !PLATFORM(QT) */

#if PLATFORM(QT)
#define WTF_USE_QT4_UNICODE 1
#elif OS(WINCE)
#define WTF_USE_WINCE_UNICODE 1
#elif PLATFORM(GTK)
/* The GTK+ Unicode backend is configurable */
#else
#define WTF_USE_ICU_UNICODE 1
#endif

#if PLATFORM(MAC) && !PLATFORM(IPHONE)
#define WTF_PLATFORM_CF 1
#define WTF_USE_PTHREADS 1
#define HAVE_PTHREAD_RWLOCK 1
#if !defined(BUILDING_ON_LEOPARD) && !defined(BUILDING_ON_TIGER) && CPU(X86_64)
#define WTF_USE_PLUGIN_HOST_PROCESS 1
#endif
#if !defined(ENABLE_MAC_JAVA_BRIDGE)
#define ENABLE_MAC_JAVA_BRIDGE 1
#endif
#if !defined(ENABLE_DASHBOARD_SUPPORT)
#define ENABLE_DASHBOARD_SUPPORT 1
#endif
#define HAVE_READLINE 1
#define HAVE_RUNLOOP_TIMER 1
#endif /* PLATFORM(MAC) && !PLATFORM(IPHONE) */

#if PLATFORM(CHROMIUM) && OS(DARWIN)
#define WTF_PLATFORM_CF 1
#define WTF_USE_PTHREADS 1
#define HAVE_PTHREAD_RWLOCK 1
#endif

#if PLATFORM(QT) && OS(DARWIN)
#define WTF_PLATFORM_CF 1
#endif

#if PLATFORM(IPHONE)
#define ENABLE_CONTEXT_MENUS 0
#define ENABLE_DRAG_SUPPORT 0
#define ENABLE_FTPDIR 1
#define ENABLE_GEOLOCATION 1
#define ENABLE_ICONDATABASE 0
#define ENABLE_INSPECTOR 0
#define ENABLE_MAC_JAVA_BRIDGE 0
#define ENABLE_NETSCAPE_PLUGIN_API 0
#define ENABLE_ORIENTATION_EVENTS 1
#define ENABLE_REPAINT_THROTTLING 1
#define HAVE_READLINE 1
#define WTF_PLATFORM_CF 1
#endif

#if OS(IPHONE_OS) && !PLATFORM(QT)
#define WTF_USE_PTHREADS 1
#define HAVE_PTHREAD_RWLOCK 1
#endif

#if PLATFORM(ANDROID)
#define WTF_USE_PTHREADS 1
#define WTF_PLATFORM_SGL 1
#define USE_SYSTEM_MALLOC 1
#define ENABLE_MAC_JAVA_BRIDGE 1
#define LOG_DISABLED 1
/* Prevents Webkit from drawing the caret in textfields and textareas
   This prevents unnecessary invals. */
#define ENABLE_TEXT_CARET 1
#define ENABLE_JAVASCRIPT_DEBUGGER 0
#endif

#if PLATFORM(WIN)
#define WTF_USE_WININET 1
#endif

#if PLATFORM(WX)
#define ENABLE_ASSEMBLER 1
#if OS(DARWIN)
#define WTF_PLATFORM_CF 1
#endif
#endif

#if PLATFORM(GTK)
#if HAVE(PTHREAD_H)
#define WTF_USE_PTHREADS 1
#define HAVE_PTHREAD_RWLOCK 1
#endif
#endif

#if PLATFORM(HAIKU)
#define HAVE_POSIX_MEMALIGN 1
#define WTF_USE_CURL 1
#define WTF_USE_PTHREADS 1
#define HAVE_PTHREAD_RWLOCK 1
#define USE_SYSTEM_MALLOC 1
#define ENABLE_NETSCAPE_PLUGIN_API 0
#endif

#if !defined(HAVE_ACCESSIBILITY)
#if PLATFORM(IPHONE) || PLATFORM(MAC) || PLATFORM(WIN) || PLATFORM(GTK) || PLATFORM(CHROMIUM)
#define HAVE_ACCESSIBILITY 1
#endif
#endif /* !defined(HAVE_ACCESSIBILITY) */

#if OS(UNIX) && !OS(SYMBIAN)
#define HAVE_SIGNAL_H 1
#endif

#if !OS(WINDOWS) && !OS(SOLARIS) && !OS(QNX) \
    && !OS(SYMBIAN) && !OS(HAIKU) && !OS(RVCT) \
    && !OS(ANDROID) && !OS(AIX) && !OS(HPUX)
#define HAVE_TM_GMTOFF 1
#define HAVE_TM_ZONE 1
#define HAVE_TIMEGM 1
#endif

#if OS(DARWIN)

#define HAVE_ERRNO_H 1
#define HAVE_LANGINFO_H 1
#define HAVE_MMAP 1
#define HAVE_MERGESORT 1
#define HAVE_SBRK 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TIMEB_H 1

#if !defined(TARGETING_TIGER) && !defined(TARGETING_LEOPARD)

#define HAVE_DISPATCH_H 1

#if !PLATFORM(IPHONE)
#define HAVE_MADV_FREE_REUSE 1
#define HAVE_MADV_FREE 1
#define HAVE_PTHREAD_SETNAME_NP 1
#endif

#endif

#if PLATFORM(IPHONE)
#define HAVE_MADV_FREE 1
#endif

#elif OS(WINDOWS)

#if OS(WINCE)
#define HAVE_ERRNO_H 0
#else
#define HAVE_SYS_TIMEB_H 1
#endif
#define HAVE_VIRTUALALLOC 1

#elif OS(SYMBIAN)

#define HAVE_ERRNO_H 1
#define HAVE_MMAP 0
#define HAVE_SBRK 1

#define HAVE_SYS_TIME_H 1
#define HAVE_STRINGS_H 1

#if !COMPILER(RVCT)
#define HAVE_SYS_PARAM_H 1
#endif

#elif OS(QNX)

#define HAVE_ERRNO_H 1
#define HAVE_MMAP 1
#define HAVE_SBRK 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_TIME_H 1

#elif OS(ANDROID)

#define HAVE_ERRNO_H 1
#define HAVE_LANGINFO_H 0
#define HAVE_MMAP 1
#define HAVE_SBRK 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_TIME_H 1

#else

/* FIXME: is this actually used or do other platforms generate their own config.h? */

#define HAVE_ERRNO_H 1
/* As long as Haiku doesn't have a complete support of locale this will be disabled. */
#if !OS(HAIKU)
#define HAVE_LANGINFO_H 1
#endif
#define HAVE_MMAP 1
#define HAVE_SBRK 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_TIME_H 1

#endif

/* ENABLE macro defaults */

/* fastMalloc match validation allows for runtime verification that
   new is matched by delete, fastMalloc is matched by fastFree, etc. */
#if !defined(ENABLE_FAST_MALLOC_MATCH_VALIDATION)
#define ENABLE_FAST_MALLOC_MATCH_VALIDATION 0
#endif

#if !defined(ENABLE_ICONDATABASE)
#define ENABLE_ICONDATABASE 1
#endif

#if !defined(ENABLE_DATABASE)
#define ENABLE_DATABASE 1
#endif

#if !defined(ENABLE_JAVASCRIPT_DEBUGGER)
#define ENABLE_JAVASCRIPT_DEBUGGER 1
#endif

#if !defined(ENABLE_FTPDIR)
#define ENABLE_FTPDIR 1
#endif

#if !defined(ENABLE_CONTEXT_MENUS)
#define ENABLE_CONTEXT_MENUS 1
#endif

#if !defined(ENABLE_DRAG_SUPPORT)
#define ENABLE_DRAG_SUPPORT 1
#endif

#if !defined(ENABLE_DASHBOARD_SUPPORT)
#define ENABLE_DASHBOARD_SUPPORT 0
#endif

#if !defined(ENABLE_INSPECTOR)
#define ENABLE_INSPECTOR 1
#endif

#if !defined(ENABLE_MAC_JAVA_BRIDGE)
#define ENABLE_MAC_JAVA_BRIDGE 0
#endif

#if !defined(ENABLE_NETSCAPE_PLUGIN_API)
#define ENABLE_NETSCAPE_PLUGIN_API 1
#endif

#if !defined(WTF_USE_PLUGIN_HOST_PROCESS)
#define WTF_USE_PLUGIN_HOST_PROCESS 0
#endif

#if !defined(ENABLE_ORIENTATION_EVENTS)
#define ENABLE_ORIENTATION_EVENTS 0
#endif

#if !defined(ENABLE_OPCODE_STATS)
#define ENABLE_OPCODE_STATS 0
#endif

#define ENABLE_SAMPLING_COUNTERS 0
#define ENABLE_SAMPLING_FLAGS 0
#define ENABLE_OPCODE_SAMPLING 0
#define ENABLE_CODEBLOCK_SAMPLING 0
#if ENABLE(CODEBLOCK_SAMPLING) && !ENABLE(OPCODE_SAMPLING)
#error "CODEBLOCK_SAMPLING requires OPCODE_SAMPLING"
#endif
#if ENABLE(OPCODE_SAMPLING) || ENABLE(SAMPLING_FLAGS)
#define ENABLE_SAMPLING_THREAD 1
#endif

#if !defined(ENABLE_GEOLOCATION)
#define ENABLE_GEOLOCATION 0
#endif

#if !defined(ENABLE_NOTIFICATIONS)
#define ENABLE_NOTIFICATIONS 0
#endif

#if !defined(ENABLE_TEXT_CARET)
#define ENABLE_TEXT_CARET 1
#endif

#if !defined(ENABLE_ON_FIRST_TEXTAREA_FOCUS_SELECT_ALL)
#define ENABLE_ON_FIRST_TEXTAREA_FOCUS_SELECT_ALL 0
#endif

#if !defined(WTF_USE_JSVALUE64) && !defined(WTF_USE_JSVALUE32) && !defined(WTF_USE_JSVALUE32_64)
#if (CPU(X86_64) && (OS(UNIX) || OS(WINDOWS) || OS(SOLARIS) || OS(HPUX))) || (CPU(IA64) && !CPU(IA64_32)) || CPU(ALPHA) || CPU(AIX64) || CPU(SPARC64) || CPU(MIPS64) || CPU(AARCH64)
#define WTF_USE_JSVALUE64 1
#elif CPU(ARM) || CPU(PPC64)
#define WTF_USE_JSVALUE32 1
#elif OS(WINDOWS) && COMPILER(MINGW)
/* Using JSVALUE32_64 causes padding/alignement issues for JITStubArg
on MinGW. See https://bugs.webkit.org/show_bug.cgi?id=29268 */
#define WTF_USE_JSVALUE32 1
#else
#define WTF_USE_JSVALUE32_64 1
#endif
#endif /* !defined(WTF_USE_JSVALUE64) && !defined(WTF_USE_JSVALUE32) && !defined(WTF_USE_JSVALUE32_64) */

#if !defined(ENABLE_REPAINT_THROTTLING)
#define ENABLE_REPAINT_THROTTLING 0
#endif

#if !defined(ENABLE_JIT)

/* The JIT is tested & working on x86_64 Mac */
#if CPU(X86_64) && PLATFORM(MAC)
    #define ENABLE_JIT 1
/* The JIT is tested & working on x86 Mac */
#elif CPU(X86) && PLATFORM(MAC)
    #define ENABLE_JIT 1
    #define WTF_USE_JIT_STUB_ARGUMENT_VA_LIST 1
#elif CPU(ARM_THUMB2) && PLATFORM(IPHONE)
    #define ENABLE_JIT 1
/* The JIT is tested & working on x86 Windows */
#elif CPU(X86) && PLATFORM(WIN)
    #define ENABLE_JIT 1
#endif

#if PLATFORM(QT)
#if CPU(X86_64) && OS(DARWIN)
    #define ENABLE_JIT 1
#elif CPU(X86) && OS(DARWIN)
    #define ENABLE_JIT 1
    #define WTF_USE_JIT_STUB_ARGUMENT_VA_LIST 1
#elif CPU(X86) && OS(WINDOWS) && COMPILER(MINGW) && GCC_VERSION >= 40100
    #define ENABLE_JIT 1
    #define WTF_USE_JIT_STUB_ARGUMENT_VA_LIST 1
#elif CPU(X86) && OS(WINDOWS) && COMPILER(MSVC)
    #define ENABLE_JIT 1
    #define WTF_USE_JIT_STUB_ARGUMENT_REGISTER 1
#elif CPU(X86) && OS(LINUX) && GCC_VERSION >= 40100
    #define ENABLE_JIT 1
    #define WTF_USE_JIT_STUB_ARGUMENT_VA_LIST 1
#elif CPU(X86_64) && OS(LINUX) && GCC_VERSION >= 40100
    #define ENABLE_JIT 1
#elif CPU(ARM_TRADITIONAL) && OS(LINUX)
    #define ENABLE_JIT 1
#endif
#endif /* PLATFORM(QT) */

#endif /* !defined(ENABLE_JIT) */

#if ENABLE(JIT)
#ifndef ENABLE_JIT_OPTIMIZE_CALL
#define ENABLE_JIT_OPTIMIZE_CALL 1
#endif
#ifndef ENABLE_JIT_OPTIMIZE_NATIVE_CALL
#define ENABLE_JIT_OPTIMIZE_NATIVE_CALL 1
#endif
#ifndef ENABLE_JIT_OPTIMIZE_PROPERTY_ACCESS
#define ENABLE_JIT_OPTIMIZE_PROPERTY_ACCESS 1
#endif
#ifndef ENABLE_JIT_OPTIMIZE_METHOD_CALLS
#define ENABLE_JIT_OPTIMIZE_METHOD_CALLS 1
#endif
#endif

#if CPU(X86) && COMPILER(MSVC)
#define JSC_HOST_CALL __fastcall
#elif CPU(X86) && COMPILER(GCC)
#define JSC_HOST_CALL __attribute__ ((fastcall))
#else
#define JSC_HOST_CALL
#endif

#if COMPILER(GCC) && !ENABLE(JIT)
#define HAVE_COMPUTED_GOTO 1
#endif

#if ENABLE(JIT) && defined(COVERAGE)
    #define WTF_USE_INTERPRETER 0
#else
    #define WTF_USE_INTERPRETER 1
#endif

/* Yet Another Regex Runtime. */
#if !defined(ENABLE_YARR_JIT)

/* YARR supports x86 & x86-64, and has been tested on Mac and Windows. */
#if (CPU(X86) && PLATFORM(MAC)) \
    || (CPU(X86_64) && PLATFORM(MAC)) \
    || (CPU(ARM_THUMB2) && PLATFORM(IPHONE)) \
    || (CPU(X86) && PLATFORM(WIN))
#define ENABLE_YARR 1
#define ENABLE_YARR_JIT 1
#endif

#if PLATFORM(QT)
#if (CPU(X86) && OS(WINDOWS) && COMPILER(MINGW) && GCC_VERSION >= 40100) \
    || (CPU(X86_64) && OS(WINDOWS) && COMPILER(MINGW64) && GCC_VERSION >= 40100) \
    || (CPU(X86) && OS(WINDOWS) && COMPILER(MSVC)) \
    || (CPU(X86) && OS(LINUX) && GCC_VERSION >= 40100) \
    || (CPU(X86_64) && OS(LINUX) && GCC_VERSION >= 40100) \
    || (CPU(ARM_TRADITIONAL) && OS(LINUX))
#define ENABLE_YARR 1
#define ENABLE_YARR_JIT 1
#endif
#endif

#endif /* !defined(ENABLE_YARR_JIT) */

/* Sanity Check */
#if ENABLE(YARR_JIT) && !ENABLE(YARR)
#error "YARR_JIT requires YARR"
#endif

#if ENABLE(JIT) || ENABLE(YARR_JIT)
#define ENABLE_ASSEMBLER 1
#endif
/* Setting this flag prevents the assembler from using RWX memory; this may improve
   security but currectly comes at a significant performance cost. */
#if PLATFORM(IPHONE)
#define ENABLE_ASSEMBLER_WX_EXCLUSIVE 1
#else
#define ENABLE_ASSEMBLER_WX_EXCLUSIVE 0
#endif

/* Pick which allocator to use; we only need an executable allocator if the assembler is compiled in.
   On x86-64 we use a single fixed mmap, on other platforms we mmap on demand. */
#if ENABLE(ASSEMBLER)
#if CPU(X86_64) && !COMPILER(MINGW64)
#define ENABLE_EXECUTABLE_ALLOCATOR_FIXED 1
#else
#define ENABLE_EXECUTABLE_ALLOCATOR_DEMAND 1
#endif
#endif

#if !defined(ENABLE_PAN_SCROLLING) && OS(WINDOWS)
#define ENABLE_PAN_SCROLLING 1
#endif

/* Use the QXmlStreamReader implementation for XMLTokenizer */
/* Use the QXmlQuery implementation for XSLTProcessor */
#if PLATFORM(QT)
#define WTF_USE_QXMLSTREAM 1
#define WTF_USE_QXMLQUERY 1
#endif

#if !PLATFORM(QT)
#define WTF_USE_FONT_FAST_PATH 1
#endif

/* Accelerated compositing */
#if PLATFORM(MAC)
#if !defined(BUILDING_ON_TIGER)
#define WTF_USE_ACCELERATED_COMPOSITING 1
#endif
#endif

#if PLATFORM(IPHONE)
#define WTF_USE_ACCELERATED_COMPOSITING 1
#endif

/* FIXME: Defining ENABLE_3D_RENDERING here isn't really right, but it's always used with
   with WTF_USE_ACCELERATED_COMPOSITING, and it allows the feature to be turned on and
   off in one place. */
#if PLATFORM(WIN)
#include "QuartzCorePresent.h"
#if QUARTZCORE_PRESENT
#define WTF_USE_ACCELERATED_COMPOSITING 1
#define ENABLE_3D_RENDERING 1
#endif
#endif

#if COMPILER(GCC)
#define WARN_UNUSED_RETURN __attribute__ ((warn_unused_result))
#else
#define WARN_UNUSED_RETURN
#endif

#if !ENABLE(NETSCAPE_PLUGIN_API) || (ENABLE(NETSCAPE_PLUGIN_API) && ((OS(UNIX) && (PLATFORM(QT) || PLATFORM(WX))) || PLATFORM(GTK)))
#define ENABLE_PLUGIN_PACKAGE_SIMPLE_HASH 1
#endif

/* Set up a define for a common error that is intended to cause a build error -- thus the space after Error. */
#define WTF_PLATFORM_CFNETWORK Error USE_macro_should_be_used_with_CFNETWORK

#define ENABLE_JSC_ZOMBIES 0

#endif /* WTF_Platform_h */
