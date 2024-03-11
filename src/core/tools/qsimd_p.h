/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#ifndef QSIMD_P_H
#define QSIMD_P_H

#include <qglobal.h>

#include <atomic>

#if defined(__MINGW64_VERSION_MAJOR) || defined(Q_CC_MSVC)
#include <intrin.h>
#endif

#define QT_COMPILER_SUPPORTS(x)     (QT_COMPILER_SUPPORTS_ ## x - 0)

#if defined(Q_CC_INTEL) || defined(Q_CC_MSVC) || (defined(Q_CC_GNU) && ! defined(Q_CC_CLANG))

#  define QT_COMPILER_SUPPORTS_SIMD_ALWAYS
#  define QT_COMPILER_SUPPORTS_HERE(x)    QT_COMPILER_SUPPORTS(x)

#  if defined(Q_CC_GNU) && ! defined(Q_CC_INTEL)
// GCC requires attributes for a function
#    define QT_FUNCTION_TARGET(x)  __attribute__((__target__(QT_FUNCTION_TARGET_STRING_ ## x)))
#  else
#    define QT_FUNCTION_TARGET(x)
#  endif

#else
#  define QT_COMPILER_SUPPORTS_HERE(x)    (__ ## x ## __)
#  define QT_FUNCTION_TARGET(x)

#endif

// SSE intrinsics
#define QT_FUNCTION_TARGET_STRING_SSE2      "sse2"

#if defined(__SSE2__) || (defined(QT_COMPILER_SUPPORTS_SSE2) && defined(QT_COMPILER_SUPPORTS_SIMD_ALWAYS))

#if defined(QT_LINUXBASE) || defined(Q_OS_ANDROID_NO_SDK)
/// posix_memalign declaration in LSB is incorrect
//  http://bugs.linuxbase.org/show_bug.cgi?id=2431
#  define posix_memalign _lsb_hack_posix_memalign
#  include <emmintrin.h>
#  undef posix_memalign

#else
#  include <emmintrin.h>
#endif

#if defined(Q_CC_MSVC) && (defined(_M_X64) || _M_IX86_FP >= 2)
#  define __SSE__  1
#  define __SSE2__ 1
#endif

#endif

// SSE3 intrinsics
#define QT_FUNCTION_TARGET_STRING_SSE3      "sse3"
#if defined(__SSE3__) || (defined(QT_COMPILER_SUPPORTS_SSE3) && defined(QT_COMPILER_SUPPORTS_SIMD_ALWAYS))
#include <pmmintrin.h>
#endif

// SSSE3 intrinsics
#define QT_FUNCTION_TARGET_STRING_SSSE3     "ssse3"
#if defined(__SSSE3__) || (defined(QT_COMPILER_SUPPORTS_SSSE3) && defined(QT_COMPILER_SUPPORTS_SIMD_ALWAYS))
#include <tmmintrin.h>
#endif

// SSE4.1 intrinsics
#define QT_FUNCTION_TARGET_STRING_SSE4_1    "sse4.1"
#if defined(__SSE4_1__) || (defined(QT_COMPILER_SUPPORTS_SSE4_1) && defined(QT_COMPILER_SUPPORTS_SIMD_ALWAYS))
#include <smmintrin.h>
#endif

// SSE4.2 intrinsics
#define QT_FUNCTION_TARGET_STRING_SSE4_2    "sse4.2"
#if defined(__SSE4_2__) || (defined(QT_COMPILER_SUPPORTS_SSE4_2) && defined(QT_COMPILER_SUPPORTS_SIMD_ALWAYS))
#include <nmmintrin.h>
#endif

// AVX intrinsics
#define QT_FUNCTION_TARGET_STRING_AVX       "avx"
#define QT_FUNCTION_TARGET_STRING_AVX2      "avx2"

#if defined(__AVX__) || (defined(QT_COMPILER_SUPPORTS_AVX) && defined(QT_COMPILER_SUPPORTS_SIMD_ALWAYS))
// immintrin.h is the ultimate header, we do not need anything else after this
#include <immintrin.h>

#  if defined(Q_CC_MSVC) && (defined(_M_AVX) || defined(__AVX__))

// MS Visual Studio 2010 had no macro to identify the use of /arch:AVX
// MS Visual Studio 2013 adds: __AVX__

#    define __SSE3__   1
#    define __SSSE3__  1

// no Intel CPU supports SSE4a, so do not define it
#    define __SSE4_1__ 1
#    define __SSE4_2__ 1

#    ifndef __AVX__
#      define __AVX__ 1
#    endif
#  endif
#endif

#define QT_FUNCTION_TARGET_STRING_AVX512F       "avx512f"
#define QT_FUNCTION_TARGET_STRING_AVX512CD      "avx512cd"
#define QT_FUNCTION_TARGET_STRING_AVX512ER      "avx512er"
#define QT_FUNCTION_TARGET_STRING_AVX512PF      "avx512pf"
#define QT_FUNCTION_TARGET_STRING_AVX512BW      "avx512bw"
#define QT_FUNCTION_TARGET_STRING_AVX512DQ      "avx512dq"
#define QT_FUNCTION_TARGET_STRING_AVX512VL      "avx512vl"
#define QT_FUNCTION_TARGET_STRING_AVX512IFMA    "avx512ifma"
#define QT_FUNCTION_TARGET_STRING_AVX512VBMI    "avx512vbmi"

#define QT_FUNCTION_TARGET_STRING_F16C          "f16c"
#define QT_FUNCTION_TARGET_STRING_RDRAND        "rdrnd"
#define QT_FUNCTION_TARGET_STRING_BMI           "bmi"
#define QT_FUNCTION_TARGET_STRING_BMI2          "bmi2"
#define QT_FUNCTION_TARGET_STRING_RDSEED        "rdseed"
#define QT_FUNCTION_TARGET_STRING_SHA           "sha"

#if defined(Q_PROCESSOR_X86) && (defined(Q_CC_GNU) || defined(Q_CC_CLANG) || defined(Q_CC_INTEL))

#  define QT_COMPILER_SUPPORTS_X86INTRIN

#  ifdef Q_CC_INTEL
#    include <immintrin.h>
#  else
#    include <x86intrin.h>
#  endif

#endif

// NEON intrinsics
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#define QT_FUNCTION_TARGET_STRING_ARM_NEON      "neon"

#ifndef __ARM_NEON__
// __ARM_NEON__ is not defined on AArch64, required in our NEON detection
#define __ARM_NEON__
#endif

#endif

#undef QT_COMPILER_SUPPORTS_SIMD_ALWAYS

enum CPUFeatures {
#if defined(Q_PROCESSOR_ARM)
   CpuFeatureNEON          = 0,
   CpuFeatureARM_NEON      = CpuFeatureNEON,

#elif defined(Q_PROCESSOR_MIPS)
   CpuFeatureDSP           = 0,
   CpuFeatureDSPR2         = 1,

#elif defined(Q_PROCESSOR_X86)
   // The order of the flags is jumbled so it matches most closely the bits in CPUID

   // Out of order:
   CpuFeatureSSE2          = 1,                       // uses the bit for PCLMULQDQ

   // in level 1, ECX
   CpuFeatureSSE3          = (0 + 0),
   CpuFeatureSSSE3         = (0 + 9),
   CpuFeatureSSE4_1        = (0 + 19),
   CpuFeatureSSE4_2        = (0 + 20),
   CpuFeatureMOVBE         = (0 + 22),
   CpuFeaturePOPCNT        = (0 + 23),
   CpuFeatureAES           = (0 + 25),
   CpuFeatureAVX           = (0 + 28),
   CpuFeatureF16C          = (0 + 29),
   CpuFeatureRDRAND        = (0 + 30),
   // 31 is always zero and we've used it for the QSimdInitialized

   // in level 7, leaf 0, EBX
   CpuFeatureBMI           = (32 + 3),
   CpuFeatureHLE           = (32 + 4),
   CpuFeatureAVX2          = (32 + 5),
   CpuFeatureBMI2          = (32 + 8),
   CpuFeatureRTM           = (32 + 11),
   CpuFeatureAVX512F       = (32 + 16),
   CpuFeatureAVX512DQ      = (32 + 17),
   CpuFeatureRDSEED        = (32 + 18),
   CpuFeatureAVX512IFMA    = (32 + 21),
   CpuFeatureAVX512PF      = (32 + 26),
   CpuFeatureAVX512ER      = (32 + 27),
   CpuFeatureAVX512CD      = (32 + 28),
   CpuFeatureSHA           = (32 + 29),
   CpuFeatureAVX512BW      = (32 + 30),
   CpuFeatureAVX512VL      = (32 + 31),

   // in level 7, leaf 0, ECX (out of order, for now)
   CpuFeatureAVX512VBMI    = 2,                       // uses the bit for DTES64
#endif

   // used only to indicate that the CPU detection was initialised
   QSimdInitialized = 0x80000000
};

static constexpr const quint64 qCompilerCpuFeatures = 0

#if defined __SHA__
      | (Q_UINT64_C(1) << CpuFeatureSHA)
#endif

#if defined __AES__
      | (Q_UINT64_C(1) << CpuFeatureAES)
#endif

#if defined __RTM__
      | (Q_UINT64_C(1) << CpuFeatureRTM)
#endif

#ifdef __RDRND__
      | (Q_UINT64_C(1) << CpuFeatureRDRAND)
#endif

#ifdef __RDSEED__
      | (Q_UINT64_C(1) << CpuFeatureRDSEED)
#endif

#if defined __BMI__
      | (Q_UINT64_C(1) << CpuFeatureBMI)
#endif

#if defined __BMI2__
      | (Q_UINT64_C(1) << CpuFeatureBMI2)
#endif

#if defined __F16C__
      | (Q_UINT64_C(1) << CpuFeatureF16C)
#endif

#if defined __POPCNT__
      | (Q_UINT64_C(1) << CpuFeaturePOPCNT)
#endif

#if defined __MOVBE__           // GCC and Clang do not seem to define this
      | (Q_UINT64_C(1) << CpuFeatureMOVBE)
#endif

#if defined __AVX512F__
      | (Q_UINT64_C(1) << CpuFeatureAVX512F)
#endif

#if defined __AVX512CD__
      | (Q_UINT64_C(1) << CpuFeatureAVX512CD)
#endif

#if defined __AVX512ER__
      | (Q_UINT64_C(1) << CpuFeatureAVX512ER)
#endif

#if defined __AVX512PF__
      | (Q_UINT64_C(1) << CpuFeatureAVX512PF)
#endif

#if defined __AVX512BW__
      | (Q_UINT64_C(1) << CpuFeatureAVX512BW)
#endif

#if defined __AVX512DQ__
      | (Q_UINT64_C(1) << CpuFeatureAVX512DQ)
#endif
#if defined __AVX512VL__
      | (Q_UINT64_C(1) << CpuFeatureAVX512VL)
#endif

#if defined __AVX512IFMA__
      | (Q_UINT64_C(1) << CpuFeatureAVX512IFMA)
#endif
#if defined __AVX512VBMI__
      | (Q_UINT64_C(1) << CpuFeatureAVX512VBMI)
#endif

#if defined __AVX2__
      | (Q_UINT64_C(1) << CpuFeatureAVX2)
#endif

#if defined __AVX__
      | (Q_UINT64_C(1) << CpuFeatureAVX)
#endif

#if defined __SSE4_2__
      | (Q_UINT64_C(1) << CpuFeatureSSE4_2)
#endif

#if defined __SSE4_1__
      | (Q_UINT64_C(1) << CpuFeatureSSE4_1)
#endif

#if defined __SSSE3__
      | (Q_UINT64_C(1) << CpuFeatureSSSE3)
#endif

#if defined __SSE3__
      | (Q_UINT64_C(1) << CpuFeatureSSE3)
#endif

#if defined __SSE2__
      | (Q_UINT64_C(1) << CpuFeatureSSE2)
#endif

#if defined __ARM_NEON__
      | (Q_UINT64_C(1) << CpuFeatureNEON)
#endif

#if defined __mips_dsp
      | (Q_UINT64_C(1) << CpuFeatureDSP)
#endif

#if defined __mips_dspr2
      | (Q_UINT64_C(1) << CpuFeatureDSPR2)
#endif
      ;

extern Q_CORE_EXPORT std::atomic<quint64> cs_cpu_features;

Q_CORE_EXPORT void qDetectCpuFeatures();
static inline quint64 qCpuFeatures()
{
   quint64 features = cs_cpu_features.load();

   if (features == 0) {
      qDetectCpuFeatures();
      features = cs_cpu_features.load();
   }

   return features;
}
#define qCpuHasFeature(feature)     ((qCompilerCpuFeatures & (Q_UINT64_C(1) << CpuFeature ## feature)) \
      || (qCpuFeatures() & (Q_UINT64_C(1) << CpuFeature ## feature)))

#define ALIGNMENT_PROLOGUE_16BYTES(ptr, i, length) \
   for (; i < static_cast<int>(qMin(static_cast<quintptr>(length), ((4 - ((reinterpret_cast<quintptr>(ptr) >> 2) & 0x3)) & 0x3))); ++i)

#endif
