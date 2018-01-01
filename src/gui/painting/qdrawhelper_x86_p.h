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

#ifndef QDRAWHELPER_X86_P_H
#define QDRAWHELPER_X86_P_H

#include <qdrawhelper_p.h>

QT_BEGIN_NAMESPACE

// 1
#ifdef QT_HAVE_MMX
extern CompositionFunction qt_functionForMode_MMX[];
extern CompositionFunctionSolid qt_functionForModeSolid_MMX[];
void qt_blend_color_argb_mmx(int count, const QSpan *spans, void *userData);
#endif

#ifdef QT_HAVE_MMXEXT
void qt_memfill32_mmxext(quint32 *dest, quint32 value, int count);
void qt_bitmapblit16_mmxext(QRasterBuffer *rasterBuffer, int x, int y, quint32 color, const uchar *src,
                            int width, int height, int stride);
#endif

// 2
#ifdef QT_HAVE_3DNOW

#if defined(QT_HAVE_MMX) || !defined(QT_HAVE_SSE)
extern CompositionFunction qt_functionForMode_MMX3DNOW[];
extern CompositionFunctionSolid qt_functionForModeSolid_MMX3DNOW[];

void qt_blend_color_argb_mmx3dnow(int count, const QSpan *spans, void *userData);
#endif

#ifdef QT_HAVE_SSE
extern CompositionFunction qt_functionForMode_SSE3DNOW[];
extern CompositionFunctionSolid qt_functionForModeSolid_SSE3DNOW[];

void qt_memfill32_sse3dnow(quint32 *dest, quint32 value, int count);
void qt_bitmapblit16_sse3dnow(QRasterBuffer *rasterBuffer, int x, int y, quint32 color, const uchar *src, 
                              int width, int height, int stride);
void qt_blend_color_argb_sse3dnow(int count, const QSpan *spans, void *userData);
#endif

#endif

// 3
#ifdef QT_HAVE_SSE
void qt_memfill32_sse(quint32 *dest, quint32 value, int count);
void qt_bitmapblit16_sse(QRasterBuffer *rasterBuffer, int x, int y, quint32 color,
                         const uchar *src, int width, int height, int stride);

void qt_blend_color_argb_sse(int count, const QSpan *spans, void *userData);

extern CompositionFunction qt_functionForMode_SSE[];
extern CompositionFunctionSolid qt_functionForModeSolid_SSE[];
#endif

// 4
#ifdef QT_HAVE_SSE2
void qt_memfill32_sse2(quint32 *dest, quint32 value, int count);
void qt_memfill16_sse2(quint16 *dest, quint16 value, int count);
void qt_bitmapblit32_sse2(QRasterBuffer *rasterBuffer, int x, int y, quint32 color,
                          const uchar *src, int width, int height, int stride);

void qt_bitmapblit16_sse2(QRasterBuffer *rasterBuffer, int x, int y, quint32 color,
                          const uchar *src, int width, int height, int stride);

void qt_blend_argb32_on_argb32_sse2(uchar *destPixels, int dbpl, const uchar *srcPixels, int sbpl,
                                    int w, int h, int const_alpha);

void qt_blend_rgb32_on_rgb32_sse2(uchar *destPixels, int dbpl, const uchar *srcPixels, int sbpl,
                                  int w, int h, int const_alpha);

extern CompositionFunction qt_functionForMode_onlySSE2[];
extern CompositionFunctionSolid qt_functionForModeSolid_onlySSE2[];
#endif

#ifdef QT_HAVE_IWMMXT
void qt_blend_color_argb_iwmmxt(int count, const QSpan *spans, void *userData);

extern CompositionFunction qt_functionForMode_IWMMXT[];
extern CompositionFunctionSolid qt_functionForModeSolid_IWMMXT[];
#endif

static const int numCompositionFunctions = 33;

QT_END_NAMESPACE

#endif // QDRAWHELPER_X86_P_H
