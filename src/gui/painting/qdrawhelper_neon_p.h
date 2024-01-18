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

#ifndef QDRAWHELPER_NEON_P_H
#define QDRAWHELPER_NEON_P_H

#include <qdrawhelper_p.h>

#ifdef __ARM_NEON__

void qt_blend_argb32_on_argb32_neon(uchar *destPixels, int dbpl, const uchar *srcPixels, int sbpl,
   int w, int h, int const_alpha);

void qt_blend_rgb32_on_rgb32_neon(uchar *destPixels, int dbpl, const uchar *srcPixels, int sbpl,
   int w, int h, int const_alpha);

void qt_blend_argb32_on_rgb16_neon(uchar *destPixels, int dbpl, const uchar *srcPixels, int sbpl,
   int w, int h, int const_alpha);

void qt_blend_argb32_on_argb32_scanline_neon(uint *dest, const uint *src, int length, uint const_alpha);

void qt_blend_rgb16_on_argb32_neon(uchar *destPixels, int dbpl, const uchar *srcPixels, int sbpl,
   int w, int h, int const_alpha);

void qt_blend_rgb16_on_rgb16_neon(uchar *destPixels, int dbpl, const uchar *srcPixels, int sbpl,
   int w, int h, int const_alpha);

void qt_alphamapblit_quint16_neon(QRasterBuffer *rasterBuffer, int x, int y, quint32 color, const uchar *bitmap,
   int mapWidth, int mapHeight, int mapStride, const QClipData *clip);

void qt_scale_image_argb32_on_rgb16_neon(uchar *destPixels, int dbpl, const uchar *srcPixels,
   int sbpl, int sh, const QRectF &targetRect, const QRectF &sourceRect,
   const QRect &clip, int const_alpha);

void qt_scale_image_rgb16_on_rgb16_neon(uchar *destPixels, int dbpl, const uchar *srcPixels, int sbpl, int sh,
   const QRectF &targetRect, const QRectF &sourceRect, const QRect &clip, int const_alpha);

void qt_transform_image_argb32_on_rgb16_neon(uchar *destPixels, int dbpl, const uchar *srcPixels, int sbpl,
   const QRectF &targetRect, const QRectF &sourceRect, const QRect &clip,
   const QTransform &targetRectTransform, int const_alpha);

void qt_transform_image_rgb16_on_rgb16_neon(uchar *destPixels, int dbpl, const uchar *srcPixels, int sbpl,
   const QRectF &targetRect, const QRectF &sourceRect, const QRect &clip,
   const QTransform &targetRectTransform, int const_alpha);

void qt_memfill32_neon(quint32 *dest, quint32 value, int count);
void qt_memrotate90_16_neon(const uchar *srcPixels, int w, int h, int sbpl, uchar *destPixels, int dbpl);
void qt_memrotate270_16_neon(const uchar *srcPixels, int w, int h, int sbpl, uchar *destPixels, int dbpl);

uint *QT_FASTCALL qt_destFetchRGB16_neon(uint *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length);

void QT_FASTCALL qt_destStoreRGB16_neon(QRasterBuffer *rasterBuffer, int x, int y, const uint *buffer, int length);

void QT_FASTCALL comp_func_solid_SourceOver_neon(uint *destPixels, int length, uint color, uint const_alpha);
void QT_FASTCALL comp_func_Plus_neon(uint *dst, const uint *src, int length, uint const_alpha);

#endif


#endif
