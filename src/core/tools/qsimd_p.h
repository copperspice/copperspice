/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#ifndef QSIMD_P_H
#define QSIMD_P_H

#include <qglobal.h>

#if defined(QT_NO_MAC_XARCH) || (defined(Q_OS_DARWIN) && (defined(__ppc__) || defined(__ppc64__)))
// Disable MMX and SSE on Mac/PPC builds, or if the compiler
// does not support -Xarch argument passing
#undef QT_HAVE_SSE
#undef QT_HAVE_SSE2
#undef QT_HAVE_SSE3
#undef QT_HAVE_SSSE3
#undef QT_HAVE_SSE4_1
#undef QT_HAVE_SSE4_2
#undef QT_HAVE_AVX
#undef QT_HAVE_3DNOW
#undef QT_HAVE_MMX
#endif

// SSE intrinsics
#if defined(QT_HAVE_SSE2) && (defined(__SSE2__) )
#if defined(QT_LINUXBASE)
/// this is an evil hack - the posix_memalign declaration in LSB
/// is wrong - see http://bugs.linuxbase.org/show_bug.cgi?id=2431
#  define posix_memalign _lsb_hack_posix_memalign
#  include <emmintrin.h>
#  undef posix_memalign
#else
#  ifdef Q_CC_MINGW
#    include <windows.h>
#  endif
#  include <emmintrin.h>
#endif

// SSE3 intrinsics
#if defined(QT_HAVE_SSE3) && (defined(__SSE3__))
#include <pmmintrin.h>
#endif

// SSSE3 intrinsics
#if defined(QT_HAVE_SSSE3) && (defined(__SSSE3__))
#include <tmmintrin.h>
#endif

// SSE4.1 intrinsics
#if defined(QT_HAVE_SSE4_1) && (defined(__SSE4_1__))
#include <smmintrin.h>
#endif

// SSE4.2 intrinsics
#if defined(QT_HAVE_SSE4_2) && (defined(__SSE4_2__))
#include <nmmintrin.h>

// Add missing intrisics in some compilers (e.g. llvm-gcc)
#ifndef _SIDD_UBYTE_OPS
#define _SIDD_UBYTE_OPS                 0x00
#endif

#ifndef _SIDD_UWORD_OPS
#define _SIDD_UWORD_OPS                 0x01
#endif

#ifndef _SIDD_SBYTE_OPS
#define _SIDD_SBYTE_OPS                 0x02
#endif

#ifndef _SIDD_SWORD_OPS
#define _SIDD_SWORD_OPS                 0x03
#endif

#ifndef _SIDD_CMP_EQUAL_ANY
#define _SIDD_CMP_EQUAL_ANY             0x00
#endif

#ifndef _SIDD_CMP_RANGES
#define _SIDD_CMP_RANGES                0x04
#endif

#ifndef _SIDD_CMP_EQUAL_EACH
#define _SIDD_CMP_EQUAL_EACH            0x08
#endif

#ifndef _SIDD_CMP_EQUAL_ORDERED
#define _SIDD_CMP_EQUAL_ORDERED         0x0c
#endif

#ifndef _SIDD_POSITIVE_POLARITY
#define _SIDD_POSITIVE_POLARITY         0x00
#endif

#ifndef _SIDD_NEGATIVE_POLARITY
#define _SIDD_NEGATIVE_POLARITY         0x10
#endif

#ifndef _SIDD_MASKED_POSITIVE_POLARITY
#define _SIDD_MASKED_POSITIVE_POLARITY  0x20
#endif

#ifndef _SIDD_MASKED_NEGATIVE_POLARITY
#define _SIDD_MASKED_NEGATIVE_POLARITY  0x30
#endif

#ifndef _SIDD_LEAST_SIGNIFICANT
#define _SIDD_LEAST_SIGNIFICANT         0x00
#endif

#ifndef _SIDD_MOST_SIGNIFICANT
#define _SIDD_MOST_SIGNIFICANT          0x40
#endif

#ifndef _SIDD_BIT_MASK
#define _SIDD_BIT_MASK                  0x00
#endif

#ifndef _SIDD_UNIT_MASK
#define _SIDD_UNIT_MASK                 0x40
#endif

#endif

// AVX intrinsics
#if defined(QT_HAVE_AVX) && (defined(__AVX__))
#include <immintrin.h>
#endif

#define QT_ALWAYS_HAVE_SSE2

#endif // defined(QT_HAVE_SSE2) && (defined(__SSE2__))

// NEON intrinsics
#if defined __ARM_NEON__
#define QT_ALWAYS_HAVE_NEON
#include <arm_neon.h>
#endif


// IWMMXT intrinsics
#if defined(QT_HAVE_IWMMXT)
#include <mmintrin.h>
#endif

#if defined(QT_HAVE_IWMMXT)
#if !defined(__IWMMXT__)
#  include <xmmintrin.h>
#endif
#endif

// 3D now intrinsics
#if defined(QT_HAVE_3DNOW) && (defined(__3dNOW__) || defined(Q_CC_MSVC))
#include <mm3dnow.h>
#endif

QT_BEGIN_NAMESPACE

enum CPUFeatures {
   None        = 0,
   MMX         = 0x1,
   MMXEXT      = 0x2,
   MMX3DNOW    = 0x4,
   MMX3DNOWEXT = 0x8,
   SSE         = 0x10,
   SSE2        = 0x20,
   CMOV        = 0x40,
   IWMMXT      = 0x80,
   NEON        = 0x100,
   SSE3        = 0x200,
   SSSE3       = 0x400,
   SSE4_1      = 0x800,
   SSE4_2      = 0x1000,
   AVX         = 0x2000
};

Q_CORE_EXPORT uint qDetectCPUFeatures();

#define ALIGNMENT_PROLOGUE_16BYTES(ptr, i, length) \
    for (; i < static_cast<int>(qMin(static_cast<quintptr>(length), ((4 - ((reinterpret_cast<quintptr>(ptr) >> 2) & 0x3)) & 0x3))); ++i)

QT_END_NAMESPACE

#endif // QSIMD_P_H
