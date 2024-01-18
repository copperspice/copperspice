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

#ifndef QVIDEOFRAMECONVERSIONHELPER_P_H
#define QVIDEOFRAMECONVERSIONHELPER_P_H

#include <qvideoframe.h>
#include <qsimd_p.h>


using VideoFrameConvertFunc = void (*)(const QVideoFrame &frame, uchar *output);

inline quint32 qConvertBGRA32ToARGB32(quint32 bgra)
{
   return (((bgra & 0xFF000000) >> 24)
         | ((bgra & 0x00FF0000) >> 8)
         | ((bgra & 0x0000FF00) << 8)
         | ((bgra & 0x000000FF) << 24));
}

inline quint32 qConvertBGR24ToARGB32(const uchar *bgr)
{
   return 0xFF000000 | bgr[0] | bgr[1] << 8 | bgr[2] << 16;
}

inline quint32 qConvertBGR565ToARGB32(quint16 bgr)
{
   return 0xff000000
      | ((((bgr) >> 8) & 0xf8) | (((bgr) >> 13) & 0x7))
      | ((((bgr) << 5) & 0xfc00) | (((bgr) >> 1) & 0x300))
      | ((((bgr) << 19) & 0xf80000) | (((bgr) << 14) & 0x70000));
}

inline quint32 qConvertBGR555ToARGB32(quint16 bgr)
{
   return 0xff000000
      | ((((bgr) >> 7) & 0xf8) | (((bgr) >> 12) & 0x7))
      | ((((bgr) << 6) & 0xf800) | (((bgr) << 1) & 0x700))
      | ((((bgr) << 19) & 0xf80000) | (((bgr) << 11) & 0x70000));
}

#define FETCH_INFO_PACKED(frame) \
    const uchar *src = frame.bits(); \
    int stride = frame.bytesPerLine(); \
    int width = frame.width(); \
    int height = frame.height();

#define FETCH_INFO_BIPLANAR(frame) \
    const uchar *plane1 = frame.bits(0); \
    const uchar *plane2 = frame.bits(1); \
    int plane1Stride = frame.bytesPerLine(0); \
    int plane2Stride = frame.bytesPerLine(1); \
    int width = frame.width(); \
    int height = frame.height();

#define FETCH_INFO_TRIPLANAR(frame) \
    const uchar *plane1 = frame.bits(0); \
    const uchar *plane2 = frame.bits(1); \
    const uchar *plane3 = frame.bits(2); \
    int plane1Stride = frame.bytesPerLine(0); \
    int plane2Stride = frame.bytesPerLine(1); \
    int plane3Stride = frame.bytesPerLine(2); \
    int width = frame.width(); \
    int height = frame.height(); \

#define MERGE_LOOPS(width, height, stride, bpp) \
    if (stride == width * bpp) { \
        width *= height; \
        height = 1; \
        stride = 0; \
    }

#define ALIGN(boundary, ptr, x, length) \
    for (; ((reinterpret_cast<qintptr>(ptr) & (boundary - 1)) != 0) && x < length; ++x)

#endif // QVIDEOFRAMECONVERSIONHELPER_P_H

