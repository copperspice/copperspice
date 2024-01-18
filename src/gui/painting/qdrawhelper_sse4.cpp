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

#include <qdrawhelper_p.h>
#include <qdrawingprimitive_sse2_p.h>

#if defined(QT_COMPILER_SUPPORTS_SSE4_1)

const uint * convertARGB32ToARGB32PM_sse4(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   return qt_convertARGB32ToARGB32PM(buffer, src, count);
}

const uint * convertRGBA8888ToARGB32PM_sse4(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   return qt_convertRGBA8888ToARGB32PM(buffer, src, count);
}

const uint * convertARGB32FromARGB32PM_sse4(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qUnpremultiply_sse4(src[i]);
   }
   return buffer;
}

const uint * convertRGBA8888FromARGB32PM_sse4(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = ARGB2RGBA(qUnpremultiply_sse4(src[i]));
   }
   return buffer;
}

const uint * convertRGBXFromARGB32PM_sse4(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = ARGB2RGBA(0xff000000 | qUnpremultiply_sse4(src[i]));
   }
   return buffer;
}

template <QtPixelOrder PixelOrder>
const uint * convertA2RGB30PMFromARGB32PM_sse4(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qConvertArgb32ToA2rgb30_sse4<PixelOrder>(src[i]);
   }
   return buffer;
}

template <>
const uint * convertA2RGB30PMFromARGB32PM_sse4<PixelOrderBGR>(uint *buffer, const uint *src,
      int count, const QPixelLayout *, const QRgb *);

template <>
const uint * convertA2RGB30PMFromARGB32PM_sse4<PixelOrderRGB>(uint *buffer, const uint *src,
      int count, const QPixelLayout *, const QRgb *);

#endif
