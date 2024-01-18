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

#include <qpixelformat.h>

static_assert(sizeof(QPixelFormat) == sizeof(quint64), "Type mismatch");

namespace QtPrivate {

QPixelFormat QPixelFormat_createYUV(QPixelFormat::YUVLayout yuvLayout, uchar alphaSize,
   QPixelFormat::AlphaUsage alphaUsage, QPixelFormat::AlphaPosition alphaPosition,
   QPixelFormat::AlphaPremultiplied premultiplied, QPixelFormat::TypeInterpretation typeInterpretation,
   QPixelFormat::ByteOrder byteOrder)
{
   uchar bits_per_pixel = 0;

   switch (yuvLayout) {
      case QPixelFormat::YUV444:
         bits_per_pixel = 24;
         break;

      case QPixelFormat::YUV422:
         bits_per_pixel = 16;
         break;

      case QPixelFormat::YUV411:
      case QPixelFormat::YUV420P:
      case QPixelFormat::YUV420SP:
      case QPixelFormat::YV12:
         bits_per_pixel = 12;
         break;

      case QPixelFormat::UYVY:
      case QPixelFormat::YUYV:
         bits_per_pixel = 16;
         break;

      case QPixelFormat::NV12:
      case QPixelFormat::NV21:
         bits_per_pixel = 12;
         break;

      case QPixelFormat::IMC1:
      case QPixelFormat::IMC2:
      case QPixelFormat::IMC3:
      case QPixelFormat::IMC4:
         bits_per_pixel = 12;
         break;

      case QPixelFormat::Y8:
         bits_per_pixel = 8;
         break;

      case QPixelFormat::Y16:
         bits_per_pixel = 16;
         break;
   }

   return QPixelFormat(QPixelFormat::YUV, 0, 0, 0, 0, bits_per_pixel, alphaSize, alphaUsage,
         alphaPosition, premultiplied, typeInterpretation, byteOrder, yuvLayout);
}

} // namespace

