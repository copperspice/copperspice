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

#ifndef QRGBA64_P_H
#define QRGBA64_P_H

#include <qrgba64.h>
#include <qdrawhelper_p.h>
#include <qsimd_p.h>

inline QRgba64 combineAlpha256(QRgba64 rgba64, uint alpha256)
{
   return QRgba64::fromRgba64(rgba64.red(), rgba64.green(), rgba64.blue(), (rgba64.alpha() * alpha256) >> 8);
}

inline QRgba64 multiplyAlpha256(QRgba64 rgba64, uint alpha256)
{
   return QRgba64::fromRgba64((rgba64.red()   * alpha256) >> 8,
         (rgba64.green() * alpha256) >> 8,
         (rgba64.blue()  * alpha256) >> 8,
         (rgba64.alpha() * alpha256) >> 8);
}

inline QRgba64 multiplyAlpha65535(QRgba64 rgba64, uint alpha65535)
{
#ifdef __SSE2__
   const __m128i va = _mm_shufflelo_epi16(_mm_cvtsi32_si128(alpha65535), _MM_SHUFFLE(0, 0, 0, 0));
   __m128i vs = _mm_loadl_epi64((__m128i *)&rgba64);
   vs = _mm_unpacklo_epi16(_mm_mullo_epi16(vs, va), _mm_mulhi_epu16(vs, va));
   vs = _mm_add_epi32(vs, _mm_srli_epi32(vs, 16));
   vs = _mm_add_epi32(vs, _mm_set1_epi32(0x8000));
   vs = _mm_srai_epi32(vs, 16);
   vs = _mm_packs_epi32(vs, _mm_setzero_si128());
   _mm_storel_epi64((__m128i *)&rgba64, vs);
   return rgba64;
#else
   return QRgba64::fromRgba64(qt_div_65535(rgba64.red()   * alpha65535),
         qt_div_65535(rgba64.green() * alpha65535),
         qt_div_65535(rgba64.blue()  * alpha65535),
         qt_div_65535(rgba64.alpha() * alpha65535));
#endif
}

inline QRgba64 multiplyAlpha255(QRgba64 rgba64, uint alpha255)
{
#ifdef __SSE2__
   return multiplyAlpha65535(rgba64, alpha255 * 257);
#else
   return QRgba64::fromRgba64(qt_div_255(rgba64.red()   * alpha255),
         qt_div_255(rgba64.green() * alpha255),
         qt_div_255(rgba64.blue()  * alpha255),
         qt_div_255(rgba64.alpha() * alpha255));
#endif
}

inline QRgba64 interpolate256(QRgba64 x, uint alpha1, QRgba64 y, uint alpha2)
{
   return QRgba64::fromRgba64(multiplyAlpha256(x, alpha1) + multiplyAlpha256(y, alpha2));
}

inline QRgba64 interpolate255(QRgba64 x, uint alpha1, QRgba64 y, uint alpha2)
{
   return QRgba64::fromRgba64(multiplyAlpha255(x, alpha1) + multiplyAlpha255(y, alpha2));
}

inline QRgba64 interpolate65535(QRgba64 x, uint alpha1, QRgba64 y, uint alpha2)
{
   return QRgba64::fromRgba64(multiplyAlpha65535(x, alpha1) + multiplyAlpha65535(y, alpha2));
}

inline QRgba64 addWithSaturation(QRgba64 a, QRgba64 b)
{
#if defined(__SSE2__) && defined(Q_PROCESSOR_X86_64)
   __m128i va = _mm_cvtsi64_si128((quint64)a);
   __m128i vb = _mm_cvtsi64_si128((quint64)b);
   va = _mm_adds_epu16(va, vb);
   return QRgba64::fromRgba64(_mm_cvtsi128_si64(va));
#else
   return QRgba64::fromRgba64(qMin(a.red() + b.red(), 65535),
         qMin(a.green() + b.green(), 65535),
         qMin(a.blue() + b.blue(), 65535),
         qMin(a.alpha() + b.alpha(), 65535));
#endif
}

#endif
