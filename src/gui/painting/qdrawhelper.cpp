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

#include <qglobal.h>

#include <qstylehints.h>
#include <qapplication.h>
#include <qatomic.h>
#include <qmath.h>

#include <qdrawhelper_p.h>
#include <qpaintengine_raster_p.h>
#include <qpainter_p.h>
#include <qdrawhelper_x86_p.h>
#include <qdrawingprimitive_sse2_p.h>
#include <qdrawhelper_neon_p.h>
#include <qapplication_p.h>
#include <qrgba64_p.h>

#if defined(QT_COMPILER_SUPPORTS_MIPS_DSP) || defined(QT_COMPILER_SUPPORTS_MIPS_DSPR2)
#include <qdrawhelper_mips_dsp_p.h>
#endif

#define MASK(src, a) src = BYTE_MUL(src, a)

enum {
   fixed_scale = 1 << 16,
   half_point = 1 << 15
};

// must be multiple of 4 for easier SIMD implementations
static const int buffer_size = 2048;

template<QImage::Format> constexpr uint redWidth();
template<QImage::Format> constexpr uint redShift();
template<QImage::Format> constexpr uint greenWidth();
template<QImage::Format> constexpr uint greenShift();
template<QImage::Format> constexpr uint blueWidth();
template<QImage::Format> constexpr uint blueShift();
template<QImage::Format> constexpr uint alphaWidth();
template<QImage::Format> constexpr uint alphaShift();

template<> constexpr uint redWidth<QImage::Format_RGB16>()
{
   return 5;
}
template<> constexpr uint redWidth<QImage::Format_RGB444>()
{
   return 4;
}
template<> constexpr uint redWidth<QImage::Format_RGB555>()
{
   return 5;
}
template<> constexpr uint redWidth<QImage::Format_RGB666>()
{
   return 6;
}
template<> constexpr uint redWidth<QImage::Format_RGB888>()
{
   return 8;
}
template<> constexpr uint redWidth<QImage::Format_ARGB4444_Premultiplied>()
{
   return 4;
}
template<> constexpr uint redWidth<QImage::Format_ARGB8555_Premultiplied>()
{
   return 5;
}
template<> constexpr uint redWidth<QImage::Format_ARGB8565_Premultiplied>()
{
   return 5;
}
template<> constexpr uint redWidth<QImage::Format_ARGB6666_Premultiplied>()
{
   return 6;
}
template<> constexpr uint redShift<QImage::Format_RGB16>()
{
   return  11;
}
template<> constexpr uint redShift<QImage::Format_RGB444>()
{
   return  8;
}
template<> constexpr uint redShift<QImage::Format_RGB555>()
{
   return 10;
}
template<> constexpr uint redShift<QImage::Format_RGB666>()
{
   return 12;
}
template<> constexpr uint redShift<QImage::Format_RGB888>()
{
   return 16;
}
template<> constexpr uint redShift<QImage::Format_ARGB4444_Premultiplied>()
{
   return  8;
}
template<> constexpr uint redShift<QImage::Format_ARGB8555_Premultiplied>()
{
   return 18;
}
template<> constexpr uint redShift<QImage::Format_ARGB8565_Premultiplied>()
{
   return 19;
}
template<> constexpr uint redShift<QImage::Format_ARGB6666_Premultiplied>()
{
   return 12;
}
template<> constexpr uint greenWidth<QImage::Format_RGB16>()
{
   return 6;
}
template<> constexpr uint greenWidth<QImage::Format_RGB444>()
{
   return 4;
}
template<> constexpr uint greenWidth<QImage::Format_RGB555>()
{
   return 5;
}
template<> constexpr uint greenWidth<QImage::Format_RGB666>()
{
   return 6;
}
template<> constexpr uint greenWidth<QImage::Format_RGB888>()
{
   return 8;
}
template<> constexpr uint greenWidth<QImage::Format_ARGB4444_Premultiplied>()
{
   return 4;
}
template<> constexpr uint greenWidth<QImage::Format_ARGB8555_Premultiplied>()
{
   return 5;
}
template<> constexpr uint greenWidth<QImage::Format_ARGB8565_Premultiplied>()
{
   return 6;
}
template<> constexpr uint greenWidth<QImage::Format_ARGB6666_Premultiplied>()
{
   return 6;
}
template<> constexpr uint greenShift<QImage::Format_RGB16>()
{
   return  5;
}
template<> constexpr uint greenShift<QImage::Format_RGB444>()
{
   return 4;
}
template<> constexpr uint greenShift<QImage::Format_RGB555>()
{
   return 5;
}
template<> constexpr uint greenShift<QImage::Format_RGB666>()
{
   return 6;
}
template<> constexpr uint greenShift<QImage::Format_RGB888>()
{
   return 8;
}
template<> constexpr uint greenShift<QImage::Format_ARGB4444_Premultiplied>()
{
   return  4;
}
template<> constexpr uint greenShift<QImage::Format_ARGB8555_Premultiplied>()
{
   return 13;
}
template<> constexpr uint greenShift<QImage::Format_ARGB8565_Premultiplied>()
{
   return 13;
}
template<> constexpr uint greenShift<QImage::Format_ARGB6666_Premultiplied>()
{
   return  6;
}
template<> constexpr uint blueWidth<QImage::Format_RGB16>()
{
   return 5;
}
template<> constexpr uint blueWidth<QImage::Format_RGB444>()
{
   return 4;
}
template<> constexpr uint blueWidth<QImage::Format_RGB555>()
{
   return 5;
}
template<> constexpr uint blueWidth<QImage::Format_RGB666>()
{
   return 6;
}
template<> constexpr uint blueWidth<QImage::Format_RGB888>()
{
   return 8;
}
template<> constexpr uint blueWidth<QImage::Format_ARGB4444_Premultiplied>()
{
   return 4;
}
template<> constexpr uint blueWidth<QImage::Format_ARGB8555_Premultiplied>()
{
   return 5;
}
template<> constexpr uint blueWidth<QImage::Format_ARGB8565_Premultiplied>()
{
   return 5;
}
template<> constexpr uint blueWidth<QImage::Format_ARGB6666_Premultiplied>()
{
   return 6;
}
template<> constexpr uint blueShift<QImage::Format_RGB16>()
{
   return 0;
}
template<> constexpr uint blueShift<QImage::Format_RGB444>()
{
   return 0;
}
template<> constexpr uint blueShift<QImage::Format_RGB555>()
{
   return 0;
}
template<> constexpr uint blueShift<QImage::Format_RGB666>()
{
   return 0;
}
template<> constexpr uint blueShift<QImage::Format_RGB888>()
{
   return 0;
}
template<> constexpr uint blueShift<QImage::Format_ARGB4444_Premultiplied>()
{
   return 0;
}
template<> constexpr uint blueShift<QImage::Format_ARGB8555_Premultiplied>()
{
   return 8;
}
template<> constexpr uint blueShift<QImage::Format_ARGB8565_Premultiplied>()
{
   return 8;
}
template<> constexpr uint blueShift<QImage::Format_ARGB6666_Premultiplied>()
{
   return 0;
}
template<> constexpr uint alphaWidth<QImage::Format_ARGB4444_Premultiplied>()
{
   return  4;
}
template<> constexpr uint alphaWidth<QImage::Format_ARGB8555_Premultiplied>()
{
   return  8;
}
template<> constexpr uint alphaWidth<QImage::Format_ARGB8565_Premultiplied>()
{
   return  8;
}
template<> constexpr uint alphaWidth<QImage::Format_ARGB6666_Premultiplied>()
{
   return  6;
}
template<> constexpr uint alphaShift<QImage::Format_ARGB4444_Premultiplied>()
{
   return 12;
}
template<> constexpr uint alphaShift<QImage::Format_ARGB8555_Premultiplied>()
{
   return  0;
}
template<> constexpr uint alphaShift<QImage::Format_ARGB8565_Premultiplied>()
{
   return  0;
}
template<> constexpr uint alphaShift<QImage::Format_ARGB6666_Premultiplied>()
{
   return 18;
}

template<QImage::Format> constexpr QPixelLayout::BPP bitsPerPixel();

template<>
constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGB16>()
{
   return QPixelLayout::BPP16;
}

template<>
constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGB444>()
{
   return QPixelLayout::BPP16;
}

template<>
constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGB555>()
{
   return QPixelLayout::BPP16;
}

template<>
constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGB666>()
{
   return QPixelLayout::BPP24;
}

template<>
constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGB888>()
{
   return QPixelLayout::BPP24;
}

template<>
constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_ARGB4444_Premultiplied>()
{
   return QPixelLayout::BPP16;
}

template<>
constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_ARGB8555_Premultiplied>()
{
   return QPixelLayout::BPP24;
}

template<>
constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_ARGB8565_Premultiplied>()
{
   return QPixelLayout::BPP24;
}

template<>
constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_ARGB6666_Premultiplied>()
{
   return QPixelLayout::BPP24;
}


template<QImage::Format Format>
static const uint *convertToRGB32(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   constexpr uint redMask = ((1 << redWidth<Format>()) - 1);
   constexpr uint greenMask = ((1 << greenWidth<Format>()) - 1);
   constexpr uint blueMask = ((1 << blueWidth<Format>()) - 1);

   constexpr uchar redLeftShift = 8 - redWidth<Format>();
   constexpr uchar greenLeftShift = 8 - greenWidth<Format>();
   constexpr uchar blueLeftShift = 8 - blueWidth<Format>();

   constexpr uchar redRightShift = 2 * redWidth<Format>() - 8;
   constexpr uchar greenRightShift = 2 * greenWidth<Format>() - 8;
   constexpr uchar blueRightShift = 2 * blueWidth<Format>() - 8;

   for (int i = 0; i < count; ++i) {
      uint red = (src[i] >> redShift<Format>()) & redMask;
      uint green = (src[i] >> greenShift<Format>()) & greenMask;
      uint blue = (src[i] >> blueShift<Format>()) & blueMask;

      red = ((red << redLeftShift) | (red >> redRightShift)) << 16;
      green = ((green << greenLeftShift) | (green >> greenRightShift)) << 8;
      blue = (blue << blueLeftShift) | (blue >> blueRightShift);
      buffer[i] = 0xff000000 | red | green | blue;
   }

   return buffer;
}

template<QImage::Format Format>
static const QRgba64 *convertToRGB64(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   constexpr uint redMask = ((1 << redWidth<Format>()) - 1);
   constexpr uint greenMask = ((1 << greenWidth<Format>()) - 1);
   constexpr uint blueMask = ((1 << blueWidth<Format>()) - 1);

   constexpr uchar redLeftShift = 8 - redWidth<Format>();
   constexpr uchar greenLeftShift = 8 - greenWidth<Format>();
   constexpr uchar blueLeftShift = 8 - blueWidth<Format>();

   constexpr uchar redRightShift = 2 * redWidth<Format>() - 8;
   constexpr uchar greenRightShift = 2 * greenWidth<Format>() - 8;
   constexpr uchar blueRightShift = 2 * blueWidth<Format>() - 8;

   for (int i = 0; i < count; ++i) {
      uint red = (src[i] >> redShift<Format>()) & redMask;
      uint green = (src[i] >> greenShift<Format>()) & greenMask;
      uint blue = (src[i] >> blueShift<Format>()) & blueMask;

      red = ((red << redLeftShift) | (red >> redRightShift));
      green = ((green << greenLeftShift) | (green >> greenRightShift));
      blue = (blue << blueLeftShift) | (blue >> blueRightShift);
      buffer[i] = QRgba64::fromRgba(red, green, blue, 255);
   }

   return buffer;
}

template<QImage::Format Format>
static const uint *convertARGBPMToARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   constexpr uint alphaMask = ((1 << alphaWidth<Format>()) - 1);
   constexpr uint redMask = ((1 << redWidth<Format>()) - 1);
   constexpr uint greenMask = ((1 << greenWidth<Format>()) - 1);
   constexpr uint blueMask = ((1 << blueWidth<Format>()) - 1);

   constexpr uchar alphaLeftShift = 8 - alphaWidth<Format>();
   constexpr uchar redLeftShift = 8 - redWidth<Format>();
   constexpr uchar greenLeftShift = 8 - greenWidth<Format>();
   constexpr uchar blueLeftShift = 8 - blueWidth<Format>();

   constexpr uchar alphaRightShift = 2 * alphaWidth<Format>() - 8;
   constexpr uchar redRightShift = 2 * redWidth<Format>() - 8;
   constexpr uchar greenRightShift = 2 * greenWidth<Format>() - 8;
   constexpr uchar blueRightShift = 2 * blueWidth<Format>() - 8;

   for (int i = 0; i < count; ++i) {
      uint alpha = (src[i] >> alphaShift<Format>()) & alphaMask;
      uint red = (src[i] >> redShift<Format>()) & redMask;
      uint green = (src[i] >> greenShift<Format>()) & greenMask;
      uint blue = (src[i] >> blueShift<Format>()) & blueMask;

      alpha = (alpha << alphaLeftShift) | (alpha >> alphaRightShift);
      red = qMin(alpha, (red << redLeftShift) | (red >> redRightShift));
      green = qMin(alpha, (green << greenLeftShift) | (green >> greenRightShift));
      blue = qMin(alpha, (blue << blueLeftShift) | (blue >> blueRightShift));
      buffer[i] = (alpha << 24) | (red << 16) | (green << 8) | blue;
   }

   return buffer;
}

template<QImage::Format Format>
static const QRgba64 *convertARGBPMToARGB64PM(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   constexpr uint alphaMask = ((1 << alphaWidth<Format>()) - 1);
   constexpr uint redMask = ((1 << redWidth<Format>()) - 1);
   constexpr uint greenMask = ((1 << greenWidth<Format>()) - 1);
   constexpr uint blueMask = ((1 << blueWidth<Format>()) - 1);

   constexpr uchar alphaLeftShift = 8 - alphaWidth<Format>();
   constexpr uchar redLeftShift = 8 - redWidth<Format>();
   constexpr uchar greenLeftShift = 8 - greenWidth<Format>();
   constexpr uchar blueLeftShift = 8 - blueWidth<Format>();

   constexpr uchar alphaRightShift = 2 * alphaWidth<Format>() - 8;
   constexpr uchar redRightShift = 2 * redWidth<Format>() - 8;
   constexpr uchar greenRightShift = 2 * greenWidth<Format>() - 8;
   constexpr uchar blueRightShift = 2 * blueWidth<Format>() - 8;

   for (int i = 0; i < count; ++i) {
      uint alpha = (src[i] >> alphaShift<Format>()) & alphaMask;
      uint red = (src[i] >> redShift<Format>()) & redMask;
      uint green = (src[i] >> greenShift<Format>()) & greenMask;
      uint blue = (src[i] >> blueShift<Format>()) & blueMask;

      alpha = (alpha << alphaLeftShift) | (alpha >> alphaRightShift);
      red = qMin(alpha, (red << redLeftShift) | (red >> redRightShift));
      green = qMin(alpha, (green << greenLeftShift) | (green >> greenRightShift));
      blue = qMin(alpha, (blue << blueLeftShift) | (blue >> blueRightShift));
      buffer[i] = QRgba64::fromRgba(red, green, blue, alpha);
   }

   return buffer;
}

template<QImage::Format Format>
static const uint *convertRGBFromARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   constexpr uint redMask = ((1 << redWidth<Format>()) - 1);
   constexpr uint greenMask = ((1 << greenWidth<Format>()) - 1);
   constexpr uint blueMask = ((1 << blueWidth<Format>()) - 1);

   constexpr uchar redRightShift = 24 - redWidth<Format>();
   constexpr uchar greenRightShift = 16 - greenWidth<Format>();
   constexpr uchar blueRightShift = 8 - blueWidth<Format>();

   for (int i = 0; i < count; ++i) {
      const uint color = qUnpremultiply(src[i]);
      const uint red = ((color >> redRightShift) & redMask) << redShift<Format>();
      const uint green = ((color >> greenRightShift) & greenMask) << greenShift<Format>();
      const uint blue = ((color >> blueRightShift) & blueMask) << blueShift<Format>();
      buffer[i] = red | green | blue;
   }
   return buffer;
}

template<QImage::Format Format>
static const uint *convertRGBFromRGB32(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   constexpr uint redMask = ((1 << redWidth<Format>()) - 1);
   constexpr uint greenMask = ((1 << greenWidth<Format>()) - 1);
   constexpr uint blueMask = ((1 << blueWidth<Format>()) - 1);

   constexpr uchar redRightShift = 24 - redWidth<Format>();
   constexpr uchar greenRightShift = 16 - greenWidth<Format>();
   constexpr uchar blueRightShift = 8 - blueWidth<Format>();

   for (int i = 0; i < count; ++i) {
      const uint red = ((src[i] >> redRightShift) & redMask) << redShift<Format>();
      const uint green = ((src[i] >> greenRightShift) & greenMask) << greenShift<Format>();
      const uint blue = ((src[i] >> blueRightShift) & blueMask) << blueShift<Format>();
      buffer[i] = red | green | blue;
   }
   return buffer;
}

template<QImage::Format Format>
static const uint *convertARGBPMFromRGB32(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   constexpr uint alphaMask = ((1 << alphaWidth<Format>()) - 1);
   constexpr uint redMask = ((1 << redWidth<Format>()) - 1);
   constexpr uint greenMask = ((1 << greenWidth<Format>()) - 1);
   constexpr uint blueMask = ((1 << blueWidth<Format>()) - 1);

   constexpr uchar redRightShift = 24 - redWidth<Format>();
   constexpr uchar greenRightShift = 16 - greenWidth<Format>();
   constexpr uchar blueRightShift = 8 - blueWidth<Format>();

   for (int i = 0; i < count; ++i) {
      constexpr uint alpha = (0xff & alphaMask) << alphaShift<Format>();
      const uint red = ((src[i] >> redRightShift) & redMask) << redShift<Format>();
      const uint green = ((src[i] >> greenRightShift) & greenMask) << greenShift<Format>();
      const uint blue = ((src[i] >> blueRightShift) & blueMask) << blueShift<Format>();
      buffer[i] = alpha | red | green | blue;
   }
   return buffer;
}

template<QImage::Format Format>
static const uint *convertARGBPMFromARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   constexpr uint alphaMask = ((1 << alphaWidth<Format>()) - 1);
   constexpr uint redMask = ((1 << redWidth<Format>()) - 1);
   constexpr uint greenMask = ((1 << greenWidth<Format>()) - 1);
   constexpr uint blueMask = ((1 << blueWidth<Format>()) - 1);

   constexpr uchar alphaRightShift = 32 - alphaWidth<Format>();
   constexpr uchar redRightShift = 24 - redWidth<Format>();
   constexpr uchar greenRightShift = 16 - greenWidth<Format>();
   constexpr uchar blueRightShift = 8 - blueWidth<Format>();

   for (int i = 0; i < count; ++i) {
      const uint alpha = ((src[i] >> alphaRightShift) & alphaMask) << alphaShift<Format>();
      const uint red = ((src[i] >> redRightShift) & redMask) << redShift<Format>();
      const uint green = ((src[i] >> greenRightShift) & greenMask) << greenShift<Format>();
      const uint blue = ((src[i] >> blueRightShift) & blueMask) << blueShift<Format>();
      buffer[i] = alpha | red | green | blue;
   }
   return buffer;
}

template<QImage::Format Format> constexpr static inline QPixelLayout pixelLayoutRGB()
{
   return QPixelLayout{
      uchar(redWidth<Format>()), uchar(redShift<Format>()),
      uchar(greenWidth<Format>()), uchar(greenShift<Format>()),
      uchar(blueWidth<Format>()), uchar(blueShift<Format>()),
      0, 0,
      false, bitsPerPixel<Format>(),
      convertToRGB32<Format>,
      convertRGBFromARGB32PM<Format>,
      convertRGBFromRGB32<Format>,
      convertToRGB64<Format>
   };
}

template<QImage::Format Format> constexpr static inline QPixelLayout pixelLayoutARGBPM()
{
   return QPixelLayout{
      uchar(redWidth<Format>()), uchar(redShift<Format>()),
      uchar(greenWidth<Format>()), uchar(greenShift<Format>()),
      uchar(blueWidth<Format>()), uchar(blueShift<Format>()),
      uchar(alphaWidth<Format>()), uchar(alphaShift<Format>()),
      true, bitsPerPixel<Format>(),
      convertARGBPMToARGB32PM<Format>,
      convertARGBPMFromARGB32PM<Format>,
      convertARGBPMFromRGB32<Format>,
      convertARGBPMToARGB64PM<Format>
   };
}


// To convert in place, let 'dest' and 'src' be the same.
static const uint *convertIndexedToARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *clut)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qPremultiply(clut[src[i]]);
   }
   return buffer;
}

static const QRgba64 *convertIndexedToARGB64PM(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *clut)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = QRgba64::fromArgb32(clut[src[i]]).premultiplied();
   }
   return buffer;
}

static const uint *convertPassThrough(uint *, const uint *src, int,
   const QPixelLayout *, const QRgb *)
{
   return src;
}

static const uint *convertARGB32ToARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   return qt_convertARGB32ToARGB32PM(buffer, src, count);
}

static const uint *convertRGBA8888PMToARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = RGBA2ARGB(src[i]);
   }
   return buffer;
}

static const uint *convertRGBA8888ToARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   return qt_convertRGBA8888ToARGB32PM(buffer, src, count);
}

static const uint *convertAlpha8ToRGB32(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qRgba(0, 0, 0, src[i]);
   }
   return buffer;
}

static const uint *convertGrayscale8ToRGB32(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qRgb(src[i], src[i], src[i]);
   }
   return buffer;
}

static const QRgba64 *convertAlpha8ToRGB64(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = QRgba64::fromRgba(0, 0, 0, src[i]);
   }
   return buffer;
}

static const QRgba64 *convertGrayscale8ToRGB64(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = QRgba64::fromRgba(src[i], src[i], src[i], 255);
   }
   return buffer;
}

static const uint *convertARGB32FromARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qUnpremultiply(src[i]);
   }
   return buffer;
}

static const uint *convertRGBA8888PMFromARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = ARGB2RGBA(src[i]);
   }
   return buffer;
}

#ifdef __SSE2__
template<bool RGBA, bool maskAlpha>
static inline void qConvertARGB32PMToARGB64PM_sse2(QRgba64 *buffer, const uint *src, int count)
{
   if (count <= 0) {
      return;
   }

   const __m128i amask = _mm_set1_epi32(0xff000000);
   int i = 0;
   for (; ((uintptr_t)buffer & 0xf) && i < count; ++i) {
      uint s = *src++;
      if (RGBA) {
         s = RGBA2ARGB(s);
      }
      *buffer++ = QRgba64::fromArgb32(s);
   }
   for (; i < count - 3; i += 4) {
      __m128i vs = _mm_loadu_si128((const __m128i *)src);
      if (maskAlpha) {
         vs = _mm_or_si128(vs, amask);
      }
      src += 4;
      __m128i v1 = _mm_unpacklo_epi8(vs, vs);
      __m128i v2 = _mm_unpackhi_epi8(vs, vs);
      if (!RGBA) {
         v1 = _mm_shufflelo_epi16(v1, _MM_SHUFFLE(3, 0, 1, 2));
         v2 = _mm_shufflelo_epi16(v2, _MM_SHUFFLE(3, 0, 1, 2));
         v1 = _mm_shufflehi_epi16(v1, _MM_SHUFFLE(3, 0, 1, 2));
         v2 = _mm_shufflehi_epi16(v2, _MM_SHUFFLE(3, 0, 1, 2));
      }
      _mm_store_si128((__m128i *)(buffer), v1);
      buffer += 2;
      _mm_store_si128((__m128i *)(buffer), v2);
      buffer += 2;
   }

   for (; i < count; ++i) {
      uint s = *src++;
      if (RGBA) {
         s = RGBA2ARGB(s);
      }
      *buffer++ = QRgba64::fromArgb32(s);
   }
}
#endif

static const QRgba64 *convertRGB32ToRGB64(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
#ifdef __SSE2__
   qConvertARGB32PMToARGB64PM_sse2<false, true>(buffer, src, count);
#else
   for (int i = 0; i < count; ++i) {
      buffer[i] = QRgba64::fromArgb32(0xff000000 | src[i]);
   }
#endif
   return buffer;
}

static const QRgba64 *convertARGB32ToARGB64PM(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
#ifdef __SSE2__
   qConvertARGB32PMToARGB64PM_sse2<false, false>(buffer, src, count);
   for (int i = 0; i < count; ++i) {
      buffer[i] = buffer[i].premultiplied();
   }
#else
   for (int i = 0; i < count; ++i) {
      buffer[i] = QRgba64::fromArgb32(src[i]).premultiplied();
   }
#endif
   return buffer;
}

static const QRgba64 *convertARGB32PMToARGB64PM(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
#ifdef __SSE2__
   qConvertARGB32PMToARGB64PM_sse2<false, false>(buffer, src, count);
#else
   for (int i = 0; i < count; ++i) {
      buffer[i] = QRgba64::fromArgb32(src[i]);
   }
#endif
   return buffer;
}

static const QRgba64 *convertRGBA8888ToARGB64PM(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
#ifdef __SSE2__
   qConvertARGB32PMToARGB64PM_sse2<true, false>(buffer, src, count);
   for (int i = 0; i < count; ++i) {
      buffer[i] = buffer[i].premultiplied();
   }
#else
   for (int i = 0; i < count; ++i) {
      buffer[i] = QRgba64::fromArgb32(RGBA2ARGB(src[i])).premultiplied();
   }
#endif
   return buffer;
}

static const QRgba64 *convertRGBA8888PMToARGB64PM(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
#ifdef __SSE2__
   qConvertARGB32PMToARGB64PM_sse2<true, false>(buffer, src, count);
#else
   for (int i = 0; i < count; ++i) {
      buffer[i] = QRgba64::fromArgb32(RGBA2ARGB(src[i]));
   }
#endif
   return buffer;
}

static const uint *convertRGBA8888FromARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = ARGB2RGBA(qUnpremultiply(src[i]));
   }
   return buffer;
}

static const uint *convertRGBXFromRGB32(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = ARGB2RGBA(0xff000000 | src[i]);
   }
   return buffer;
}

static const uint *convertRGBXFromARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = ARGB2RGBA(0xff000000 | qUnpremultiply(src[i]));
   }
   return buffer;
}

template<QtPixelOrder PixelOrder>
static const uint *convertA2RGB30PMToARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qConvertA2rgb30ToArgb32<PixelOrder>(src[i]);
   }
   return buffer;
}

#ifdef __SSE2__
template<QtPixelOrder PixelOrder>
static inline void qConvertA2RGB30PMToARGB64PM_sse2(QRgba64 *buffer, const uint *src, int count)
{
   if (count <= 0) {
      return;
   }

   const __m128i rmask = _mm_set1_epi32(0x3ff00000);
   const __m128i gmask = _mm_set1_epi32(0x000ffc00);
   const __m128i bmask = _mm_set1_epi32(0x000003ff);
   const __m128i afactor = _mm_set1_epi16(0x5555);
   int i = 0;

   for (; ((uintptr_t)buffer & 0xf) && i < count; ++i) {
      *buffer++ = qConvertA2rgb30ToRgb64<PixelOrder>(*src++);
   }

   for (; i < count - 3; i += 4) {
      __m128i vs = _mm_loadu_si128((const __m128i *)src);
      src += 4;
      __m128i va = _mm_srli_epi32(vs, 30);
      __m128i vr = _mm_and_si128(vs, rmask);
      __m128i vb = _mm_and_si128(vs, bmask);
      __m128i vg = _mm_and_si128(vs, gmask);
      va = _mm_mullo_epi16(va, afactor);
      vr = _mm_or_si128(_mm_srli_epi32(vr, 14), _mm_srli_epi32(vr, 24));
      vg = _mm_or_si128(_mm_srli_epi32(vg, 4), _mm_srli_epi32(vg, 14));
      vb = _mm_or_si128(_mm_slli_epi32(vb, 6), _mm_srli_epi32(vb, 4));
      __m128i vrb;
      if (PixelOrder == PixelOrderRGB) {
         vrb = _mm_or_si128(vr, _mm_slli_si128(vb, 2));
      } else {
         vrb = _mm_or_si128(vb, _mm_slli_si128(vr, 2));
      }
      __m128i vga = _mm_or_si128(vg, _mm_slli_si128(va, 2));
      _mm_store_si128((__m128i *)(buffer), _mm_unpacklo_epi16(vrb, vga));
      buffer += 2;
      _mm_store_si128((__m128i *)(buffer), _mm_unpackhi_epi16(vrb, vga));
      buffer += 2;
   }

   for (; i < count; ++i) {
      *buffer++ = qConvertA2rgb30ToRgb64<PixelOrder>(*src++);
   }
}
#endif

template<QtPixelOrder PixelOrder>
static const QRgba64 *convertA2RGB30PMToARGB64PM(QRgba64 *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
#ifdef __SSE2__
   qConvertA2RGB30PMToARGB64PM_sse2<PixelOrder>(buffer, src, count);
#else
   for (int i = 0; i < count; ++i) {
      buffer[i] = qConvertA2rgb30ToRgb64<PixelOrder>(src[i]);
   }
#endif
   return buffer;
}

template<QtPixelOrder PixelOrder>
static const uint *convertA2RGB30PMFromARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qConvertArgb32ToA2rgb30<PixelOrder>(src[i]);
   }
   return buffer;
}

template<QtPixelOrder PixelOrder>
static const uint *convertRGB30FromRGB32(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qConvertRgb32ToRgb30<PixelOrder>(src[i]);
   }
   return buffer;
}

template<QtPixelOrder PixelOrder>
static const uint *convertRGB30FromARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qConvertRgb32ToRgb30<PixelOrder>(qUnpremultiply(src[i]));
   }
   return buffer;
}

static const uint *convertAlpha8FromARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qAlpha(src[i]);
   }
   return buffer;
}

static const uint *convertGrayscale8FromRGB32(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qGray(src[i]);
   }
   return buffer;
}

static const uint *convertGrayscale8FromARGB32PM(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = qGray(qUnpremultiply(src[i]));
   }
   return buffer;
}

template <QPixelLayout::BPP bpp> static
uint fetchPixel(const uchar *src, int index);

template <>
inline uint fetchPixel<QPixelLayout::BPP1LSB>(const uchar *src, int index)
{
   return (src[index >> 3] >> (index & 7)) & 1;
}

template <>
inline uint fetchPixel<QPixelLayout::BPP1MSB>(const uchar *src, int index)
{
   return (src[index >> 3] >> (~index & 7)) & 1;
}

template <>
inline uint fetchPixel<QPixelLayout::BPP8>(const uchar *src, int index)
{
   return src[index];
}

template <>
inline uint fetchPixel<QPixelLayout::BPP16>(const uchar *src, int index)
{
   return reinterpret_cast<const quint16 *>(src)[index];
}

template <>
inline uint fetchPixel<QPixelLayout::BPP24>(const uchar *src, int index)
{
   return reinterpret_cast<const quint24 *>(src)[index];
}

template <>
inline uint fetchPixel<QPixelLayout::BPP32>(const uchar *src, int index)
{
   return reinterpret_cast<const uint *>(src)[index];
}

template <QPixelLayout::BPP bpp>
inline const uint *fetchPixels(uint *buffer, const uchar *src, int index, int count)
{
   for (int i = 0; i < count; ++i) {
      buffer[i] = fetchPixel<bpp>(src, index + i);
   }
   return buffer;
}

template <>
inline const uint *fetchPixels<QPixelLayout::BPP32>(uint *, const uchar *src, int index, int)
{
   return reinterpret_cast<const uint *>(src) + index;
}

template <QPixelLayout::BPP width>
static void storePixel(uchar *dest, int index, uint pixel);

template <>
inline void storePixel<QPixelLayout::BPP1LSB>(uchar *dest, int index, uint pixel)
{
   if (pixel) {
      dest[index >> 3] |= 1 << (index & 7);
   } else {
      dest[index >> 3] &= ~(1 << (index & 7));
   }
}

template <>
inline void storePixel<QPixelLayout::BPP1MSB>(uchar *dest, int index, uint pixel)
{
   if (pixel) {
      dest[index >> 3] |= 1 << (~index & 7);
   } else {
      dest[index >> 3] &= ~(1 << (~index & 7));
   }
}

template <>
inline void storePixel<QPixelLayout::BPP8>(uchar *dest, int index, uint pixel)
{
   dest[index] = uchar(pixel);
}

template <>
inline void storePixel<QPixelLayout::BPP16>(uchar *dest, int index, uint pixel)
{
   reinterpret_cast<quint16 *>(dest)[index] = quint16(pixel);
}

template <>
inline void storePixel<QPixelLayout::BPP24>(uchar *dest, int index, uint pixel)
{
   reinterpret_cast<quint24 *>(dest)[index] = quint24(pixel);
}

template <QPixelLayout::BPP width>
inline void storePixels(uchar *dest, const uint *src, int index, int count)
{
   for (int i = 0; i < count; ++i) {
      storePixel<width>(dest, index + i, src[i]);
   }
}

template <>
inline void storePixels<QPixelLayout::BPP32>(uchar *dest, const uint *src, int index, int count)
{
   memcpy(reinterpret_cast<uint *>(dest) + index, src, count * sizeof(uint));
}

// Note:
// convertToArgb32() assumes that no color channel is less than 4 bits.
// convertFromArgb32() assumes that no color channel is more than 8 bits.
// QImage::rgbSwapped() assumes that the red and blue color channels have the same number of bits.

QPixelLayout qPixelLayouts[QImage::NImageFormats] = {
   { 0,  0, 0,  0, 0,  0, 0,  0, false, QPixelLayout::BPPNone, 0, 0, 0, 0 }, // Format_Invalid
   { 0,  0, 0,  0, 0,  0, 0,  0, false, QPixelLayout::BPP1MSB, convertIndexedToARGB32PM, 0, 0, convertIndexedToARGB64PM }, // Format_Mono
   { 0,  0, 0,  0, 0,  0, 0,  0, false, QPixelLayout::BPP1LSB, convertIndexedToARGB32PM, 0, 0, convertIndexedToARGB64PM  }, // Format_MonoLSB
   { 0,  0, 0,  0, 0,  0, 0,  0, false, QPixelLayout::BPP8, convertIndexedToARGB32PM, 0, 0, convertIndexedToARGB64PM  }, // Format_Indexed8
   // Technically using convertPassThrough to convert from ARGB32PM to RGB32 is wrong,
   // but everywhere this generic conversion would be wrong is currently overloaded.
   { 8, 16, 8,  8, 8,  0, 0,  0, false, QPixelLayout::BPP32, convertPassThrough, convertPassThrough, convertPassThrough, convertRGB32ToRGB64 }, // Format_RGB32
   { 8, 16, 8,  8, 8,  0, 8, 24, false, QPixelLayout::BPP32, convertARGB32ToARGB32PM, convertARGB32FromARGB32PM, convertPassThrough, convertARGB32ToARGB64PM }, // Format_ARGB32
   { 8, 16, 8,  8, 8,  0, 8, 24,  true, QPixelLayout::BPP32, convertPassThrough, convertPassThrough, convertPassThrough, convertARGB32PMToARGB64PM }, // Format_ARGB32_Premultiplied

   pixelLayoutRGB<QImage::Format_RGB16>(),
   pixelLayoutARGBPM<QImage::Format_ARGB8565_Premultiplied>(),
   pixelLayoutRGB<QImage::Format_RGB666>(),
   pixelLayoutARGBPM<QImage::Format_ARGB6666_Premultiplied>(),
   pixelLayoutRGB<QImage::Format_RGB555>(),
   pixelLayoutARGBPM<QImage::Format_ARGB8555_Premultiplied>(),
   pixelLayoutRGB<QImage::Format_RGB888>(),
   pixelLayoutRGB<QImage::Format_RGB444>(),
   pixelLayoutARGBPM<QImage::Format_ARGB4444_Premultiplied>(),

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
   { 8, 24, 8, 16, 8,  8, 0,  0, false, QPixelLayout::BPP32, convertRGBA8888PMToARGB32PM, convertRGBXFromARGB32PM, convertRGBXFromRGB32, convertRGBA8888PMToARGB64PM }, // Format_RGBX8888
   { 8, 24, 8, 16, 8,  8, 8,  0, false, QPixelLayout::BPP32, convertRGBA8888ToARGB32PM, convertRGBA8888FromARGB32PM, convertRGBXFromRGB32, convertRGBA8888ToARGB64PM }, // Format_RGBA8888
   { 8, 24, 8, 16, 8,  8, 8,  0,  true, QPixelLayout::BPP32, convertRGBA8888PMToARGB32PM, convertRGBA8888PMFromARGB32PM, convertRGBXFromRGB32, convertRGBA8888PMToARGB64PM}, // Format_RGBA8888_Premultiplied
#else
   { 8,  0, 8,  8, 8, 16, 0, 24, false, QPixelLayout::BPP32, convertRGBA8888PMToARGB32PM, convertRGBXFromARGB32PM, convertRGBXFromRGB32, convertRGBA8888PMToARGB64PM }, // Format_RGBX8888
   { 8,  0, 8,  8, 8, 16, 8, 24, false, QPixelLayout::BPP32, convertRGBA8888ToARGB32PM, convertRGBA8888FromARGB32PM, convertRGBXFromRGB32, convertRGBA8888ToARGB64PM }, // Format_RGBA8888 (ABGR32)
   { 8,  0, 8,  8, 8, 16, 8, 24,  true, QPixelLayout::BPP32, convertRGBA8888PMToARGB32PM, convertRGBA8888PMFromARGB32PM, convertRGBXFromRGB32, convertRGBA8888PMToARGB64PM },  // Format_RGBA8888_Premultiplied
#endif
   { 10,  20, 10,  10, 10, 0, 0, 30, false, QPixelLayout::BPP32, convertA2RGB30PMToARGB32PM<PixelOrderBGR>, convertRGB30FromARGB32PM<PixelOrderBGR>, convertRGB30FromRGB32<PixelOrderBGR>, convertA2RGB30PMToARGB64PM<PixelOrderBGR> }, // Format_BGR30
   { 10,  20, 10,  10, 10, 0, 2, 30,  true, QPixelLayout::BPP32, convertA2RGB30PMToARGB32PM<PixelOrderBGR>, convertA2RGB30PMFromARGB32PM<PixelOrderBGR>, convertRGB30FromRGB32<PixelOrderBGR>, convertA2RGB30PMToARGB64PM<PixelOrderBGR> },  // Format_A2BGR30_Premultiplied
   { 10,  0, 10,  10, 10, 20, 0, 30, false, QPixelLayout::BPP32, convertA2RGB30PMToARGB32PM<PixelOrderRGB>, convertRGB30FromARGB32PM<PixelOrderRGB>, convertRGB30FromRGB32<PixelOrderRGB>, convertA2RGB30PMToARGB64PM<PixelOrderRGB> }, // Format_RGB30
   { 10,  0, 10,  10, 10, 20, 2, 30,  true, QPixelLayout::BPP32, convertA2RGB30PMToARGB32PM<PixelOrderRGB>, convertA2RGB30PMFromARGB32PM<PixelOrderRGB>, convertRGB30FromRGB32<PixelOrderRGB>, convertA2RGB30PMToARGB64PM<PixelOrderRGB> },  // Format_A2RGB30_Premultiplied
   { 0, 0,  0, 0,  0, 0,  8, 0, false, QPixelLayout::BPP8, convertAlpha8ToRGB32, convertAlpha8FromARGB32PM, 0, convertAlpha8ToRGB64 }, // Format_Alpha8
   { 0, 0,  0, 0,  0, 0,  0, 0, false, QPixelLayout::BPP8, convertGrayscale8ToRGB32, convertGrayscale8FromARGB32PM, convertGrayscale8FromRGB32, convertGrayscale8ToRGB64 } // Format_Grayscale8
};

const FetchPixelsFunc qFetchPixels[QPixelLayout::BPPCount] = {
   0, // BPPNone
   fetchPixels<QPixelLayout::BPP1MSB>, // BPP1MSB
   fetchPixels<QPixelLayout::BPP1LSB>, // BPP1LSB
   fetchPixels<QPixelLayout::BPP8>, // BPP8
   fetchPixels<QPixelLayout::BPP16>, // BPP16
   fetchPixels<QPixelLayout::BPP24>, // BPP24
   fetchPixels<QPixelLayout::BPP32> // BPP32
};

const StorePixelsFunc qStorePixels[QPixelLayout::BPPCount] = {
   0, // BPPNone
   storePixels<QPixelLayout::BPP1MSB>, // BPP1MSB
   storePixels<QPixelLayout::BPP1LSB>, // BPP1LSB
   storePixels<QPixelLayout::BPP8>, // BPP8
   storePixels<QPixelLayout::BPP16>, // BPP16
   storePixels<QPixelLayout::BPP24>, // BPP24
   storePixels<QPixelLayout::BPP32> // BPP32
};

typedef uint (*FetchPixelFunc)(const uchar *src, int index);

static const FetchPixelFunc qFetchPixel[QPixelLayout::BPPCount] = {
   0, // BPPNone
   fetchPixel<QPixelLayout::BPP1MSB>, // BPP1MSB
   fetchPixel<QPixelLayout::BPP1LSB>, // BPP1LSB
   fetchPixel<QPixelLayout::BPP8>, // BPP8
   fetchPixel<QPixelLayout::BPP16>, // BPP16
   fetchPixel<QPixelLayout::BPP24>, // BPP24
   fetchPixel<QPixelLayout::BPP32> // BPP32
};

/*
  Destination fetch. This is simple as we don't have to do bounds checks or
  transformations
*/

static uint *destFetchMono(uint *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length)
{
   uchar *__restrict data = (uchar *)rasterBuffer->scanLine(y);
   uint *start = buffer;
   const uint *end = buffer + length;
   while (buffer < end) {
      *buffer = data[x >> 3] & (0x80 >> (x & 7)) ? rasterBuffer->destColor1 : rasterBuffer->destColor0;
      ++buffer;
      ++x;
   }
   return start;
}

static uint *destFetchMonoLsb(uint *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length)
{
   uchar *__restrict data = (uchar *)rasterBuffer->scanLine(y);
   uint *start = buffer;
   const uint *end = buffer + length;
   while (buffer < end) {
      *buffer = data[x >> 3] & (0x1 << (x & 7)) ? rasterBuffer->destColor1 : rasterBuffer->destColor0;
      ++buffer;
      ++x;
   }
   return start;
}

static uint *destFetchARGB32P(uint *, QRasterBuffer *rasterBuffer, int x, int y, int)
{
   return (uint *)rasterBuffer->scanLine(y) + x;
}

static uint *destFetchRGB16(uint *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length)
{
   const ushort *__restrict data = (const ushort *)rasterBuffer->scanLine(y) + x;
   for (int i = 0; i < length; ++i) {
      buffer[i] = qConvertRgb16To32(data[i]);
   }
   return buffer;
}

static uint *destFetch(uint *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length)
{
   const QPixelLayout *layout = &qPixelLayouts[rasterBuffer->format];
   const uint *ptr = qFetchPixels[layout->bpp](buffer, rasterBuffer->scanLine(y), x, length);
   return const_cast<uint *>(layout->convertToARGB32PM(buffer, ptr, length, layout, 0));
}

static QRgba64 *destFetch64(QRgba64 *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length)
{
   const QPixelLayout *layout = &qPixelLayouts[rasterBuffer->format];
   uint buffer32[buffer_size];
   const uint *ptr = qFetchPixels[layout->bpp](buffer32, rasterBuffer->scanLine(y), x, length);

   return const_cast<QRgba64 *>(layout->convertToARGB64PM(buffer, ptr, length, layout, 0));
}

static QRgba64 *destFetch64uint32(QRgba64 *buffer, QRasterBuffer *rasterBuffer, int x, int y, int length)
{
   const QPixelLayout *layout = &qPixelLayouts[rasterBuffer->format];
   const uint *src = ((const uint *)rasterBuffer->scanLine(y)) + x;
   return const_cast<QRgba64 *>(layout->convertToARGB64PM(buffer, src, length, layout, 0));
}

static DestFetchProc destFetchProc[QImage::NImageFormats] = {
   0,                  // Format_Invalid
   destFetchMono,      // Format_Mono,
   destFetchMonoLsb,   // Format_MonoLSB
   0,                  // Format_Indexed8
   destFetchARGB32P,   // Format_RGB32
   destFetch,          // Format_ARGB32,
   destFetchARGB32P,   // Format_ARGB32_Premultiplied
   destFetchRGB16,     // Format_RGB16
   destFetch,          // Format_ARGB8565_Premultiplied
   destFetch,          // Format_RGB666
   destFetch,          // Format_ARGB6666_Premultiplied
   destFetch,          // Format_RGB555
   destFetch,          // Format_ARGB8555_Premultiplied
   destFetch,          // Format_RGB888
   destFetch,          // Format_RGB444
   destFetch,          // Format_ARGB4444_Premultiplied
   destFetch,          // Format_RGBX8888
   destFetch,          // Format_RGBA8888
   destFetch,          // Format_RGBA8888_Premultiplied
   destFetch,          // Format_BGR30
   destFetch,          // Format_A2BGR30_Premultiplied
   destFetch,          // Format_RGB30
   destFetch,          // Format_A2RGB30_Premultiplied
   destFetch,          // Format_Alpha8
   destFetch,          // Format_Grayscale8
};

static DestFetchProc64 destFetchProc64[QImage::NImageFormats] = {
   0,                  // Format_Invalid
   destFetch64,        // Format_Mono,
   destFetch64,        // Format_MonoLSB
   0,                  // Format_Indexed8
   destFetch64uint32,  // Format_RGB32
   destFetch64uint32,  // Format_ARGB32,
   destFetch64uint32,  // Format_ARGB32_Premultiplied
   destFetch64,        // Format_RGB16
   destFetch64,        // Format_ARGB8565_Premultiplied
   destFetch64,        // Format_RGB666
   destFetch64,        // Format_ARGB6666_Premultiplied
   destFetch64,        // Format_RGB555
   destFetch64,        // Format_ARGB8555_Premultiplied
   destFetch64,        // Format_RGB888
   destFetch64,        // Format_RGB444
   destFetch64,        // Format_ARGB4444_Premultiplied
   destFetch64uint32,  // Format_RGBX8888
   destFetch64uint32,  // Format_RGBA8888
   destFetch64uint32,  // Format_RGBA8888_Premultiplied
   destFetch64uint32,  // Format_BGR30
   destFetch64uint32,  // Format_A2BGR30_Premultiplied
   destFetch64uint32,  // Format_RGB30
   destFetch64uint32,  // Format_A2RGB30_Premultiplied
   destFetch64,        // Format_Alpha8
   destFetch64,        // Format_Grayscale8
};

/*
   Returns the color in the mono destination color table
   that is the "nearest" to /color/.
*/
static inline QRgb findNearestColor(QRgb color, QRasterBuffer *rbuf)
{
   QRgb color_0 = qPremultiply(rbuf->destColor0);
   QRgb color_1 = qPremultiply(rbuf->destColor1);
   color = qPremultiply(color);

   int r = qRed(color);
   int g = qGreen(color);
   int b = qBlue(color);
   int rx, gx, bx;
   int dist_0, dist_1;

   rx = r - qRed(color_0);
   gx = g - qGreen(color_0);
   bx = b - qBlue(color_0);
   dist_0 = rx * rx + gx * gx + bx * bx;

   rx = r - qRed(color_1);
   gx = g - qGreen(color_1);
   bx = b - qBlue(color_1);
   dist_1 = rx * rx + gx * gx + bx * bx;

   if (dist_0 < dist_1) {
      return color_0;
   }
   return color_1;
}

/*
  Destination store.
*/

static void destStoreMono(QRasterBuffer *rasterBuffer, int x, int y, const uint *buffer, int length)
{
   uchar *__restrict data = (uchar *)rasterBuffer->scanLine(y);
   if (rasterBuffer->monoDestinationWithClut) {
      for (int i = 0; i < length; ++i) {
         if (buffer[i] == rasterBuffer->destColor0) {
            data[x >> 3] &= ~(0x80 >> (x & 7));
         } else if (buffer[i] == rasterBuffer->destColor1) {
            data[x >> 3] |= 0x80 >> (x & 7);
         } else if (findNearestColor(buffer[i], rasterBuffer) == rasterBuffer->destColor0) {
            data[x >> 3] &= ~(0x80 >> (x & 7));
         } else {
            data[x >> 3] |= 0x80 >> (x & 7);
         }
         ++x;
      }
   } else {
      for (int i = 0; i < length; ++i) {
         if (qGray(buffer[i]) < int(qt_bayer_matrix[y & 15][x & 15])) {
            data[x >> 3] |= 0x80 >> (x & 7);
         } else {
            data[x >> 3] &= ~(0x80 >> (x & 7));
         }
         ++x;
      }
   }
}

static void destStoreMonoLsb(QRasterBuffer *rasterBuffer, int x, int y, const uint *buffer, int length)
{
   uchar *__restrict data = (uchar *)rasterBuffer->scanLine(y);
   if (rasterBuffer->monoDestinationWithClut) {
      for (int i = 0; i < length; ++i) {
         if (buffer[i] == rasterBuffer->destColor0) {
            data[x >> 3] &= ~(1 << (x & 7));
         } else if (buffer[i] == rasterBuffer->destColor1) {
            data[x >> 3] |= 1 << (x & 7);
         } else if (findNearestColor(buffer[i], rasterBuffer) == rasterBuffer->destColor0) {
            data[x >> 3] &= ~(1 << (x & 7));
         } else {
            data[x >> 3] |= 1 << (x & 7);
         }
         ++x;
      }
   } else {
      for (int i = 0; i < length; ++i) {
         if (qGray(buffer[i]) < int(qt_bayer_matrix[y & 15][x & 15])) {
            data[x >> 3] |= 1 << (x & 7);
         } else {
            data[x >> 3] &= ~(1 << (x & 7));
         }
         ++x;
      }
   }
}

static void destStoreRGB16(QRasterBuffer *rasterBuffer, int x, int y, const uint *buffer, int length)
{
   quint16 *data = (quint16 *)rasterBuffer->scanLine(y) + x;
   for (int i = 0; i < length; ++i) {
      data[i] = qConvertRgb32To16(buffer[i]);
   }
}

static void destStore(QRasterBuffer *rasterBuffer, int x, int y, const uint *buffer, int length)
{
   uint buf[buffer_size];
   const QPixelLayout *layout = &qPixelLayouts[rasterBuffer->format];
   StorePixelsFunc store = qStorePixels[layout->bpp];
   uchar *dest = rasterBuffer->scanLine(y);
   while (length) {
      int l = qMin(length, buffer_size);
      const uint *ptr = 0;
      if (!layout->premultiplied && !layout->alphaWidth) {
         ptr = layout->convertFromRGB32(buf, buffer, l, layout, 0);
      } else {
         ptr = layout->convertFromARGB32PM(buf, buffer, l, layout, 0);
      }
      store(dest, ptr, x, l);
      length -= l;
      buffer += l;
      x += l;
   }
}

static void convertFromRgb64(uint *dest, const QRgba64 *src, int length)
{
   for (int i = 0; i < length; ++i) {
      dest[i] = src[i].toArgb32();
   }
}

static void destStore64(QRasterBuffer *rasterBuffer, int x, int y, const QRgba64 *buffer, int length)
{
   uint buf[buffer_size];
   const QPixelLayout *layout = &qPixelLayouts[rasterBuffer->format];
   StorePixelsFunc store = qStorePixels[layout->bpp];
   uchar *dest = rasterBuffer->scanLine(y);
   while (length) {
      int l = qMin(length, buffer_size);
      const uint *ptr = 0;
      convertFromRgb64(buf, buffer, l);
      if (!layout->premultiplied && !layout->alphaWidth) {
         ptr = layout->convertFromRGB32(buf, buf, l, layout, 0);
      } else {
         ptr = layout->convertFromARGB32PM(buf, buf, l, layout, 0);
      }
      store(dest, ptr, x, l);
      length -= l;
      buffer += l;
      x += l;
   }
}

#ifdef __SSE2__
template<QtPixelOrder PixelOrder>
static inline void qConvertARGB64PMToA2RGB30PM_sse2(uint *dest, const QRgba64 *buffer, int count)
{
   const __m128i gmask = _mm_set1_epi32(0x000ffc00);
   const __m128i cmask = _mm_set1_epi32(0x000003ff);
   int i = 0;
   __m128i vr, vg, vb, va;
   for (; i < count && (const uintptr_t)buffer & 0xF; ++i) {
      *dest++ = qConvertRgb64ToRgb30<PixelOrder>(*buffer++);
   }

   for (; i < count - 15; i += 16) {
      // Repremultiplying is really expensive and hard to do in SIMD without AVX2,
      // so we try to avoid it by checking if it is needed 16 samples at a time.
      __m128i vOr = _mm_set1_epi32(0);
      __m128i vAnd = _mm_set1_epi32(0xffffffff);
      for (int j = 0; j < 16; j += 2) {
         __m128i vs = _mm_load_si128((const __m128i *)(buffer + j));
         vOr = _mm_or_si128(vOr, vs);
         vAnd = _mm_and_si128(vAnd, vs);
      }
      const quint16 orAlpha = ((uint)_mm_extract_epi16(vOr, 3)) | ((uint)_mm_extract_epi16(vOr, 7));
      const quint16 andAlpha = ((uint)_mm_extract_epi16(vAnd, 3)) & ((uint)_mm_extract_epi16(vAnd, 7));

      if (andAlpha == 0xffff) {
         for (int j = 0; j < 16; j += 2) {
            __m128i vs = _mm_load_si128((const __m128i *)buffer);
            buffer += 2;
            vr = _mm_srli_epi64(vs, 6);
            vg = _mm_srli_epi64(vs, 16 + 6 - 10);
            vb = _mm_srli_epi64(vs, 32 + 6);
            vr = _mm_and_si128(vr, cmask);
            vg = _mm_and_si128(vg, gmask);
            vb = _mm_and_si128(vb, cmask);
            va = _mm_srli_epi64(vs, 48 + 14);
            if (PixelOrder == PixelOrderRGB) {
               vr = _mm_slli_epi32(vr, 20);
            } else {
               vb = _mm_slli_epi32(vb, 20);
            }
            va = _mm_slli_epi32(va, 30);
            __m128i vd = _mm_or_si128(_mm_or_si128(vr, vg), _mm_or_si128(vb, va));
            vd = _mm_shuffle_epi32(vd, _MM_SHUFFLE(3, 1, 2, 0));
            _mm_storel_epi64((__m128i *)dest, vd);
            dest += 2;
         }
      } else if (orAlpha == 0) {
         for (int j = 0; j < 16; ++j) {
            *dest++ = 0;
            buffer++;
         }
      } else {
         for (int j = 0; j < 16; ++j) {
            *dest++ = qConvertRgb64ToRgb30<PixelOrder>(*buffer++);
         }
      }
   }

   for (; i < count; ++i) {
      *dest++ = qConvertRgb64ToRgb30<PixelOrder>(*buffer++);
   }
}
#endif

static void destStore64ARGB32(QRasterBuffer *rasterBuffer, int x, int y, const QRgba64 *buffer, int length)
{
   uint *dest = (uint *)rasterBuffer->scanLine(y) + x;
   for (int i = 0; i < length; ++i) {
      dest[i] = buffer[i].unpremultiplied().toArgb32();
   }
}

static void destStore64RGBA8888(QRasterBuffer *rasterBuffer, int x, int y, const QRgba64 *buffer, int length)
{
   uint *dest = (uint *)rasterBuffer->scanLine(y) + x;
   for (int i = 0; i < length; ++i) {
      dest[i] = ARGB2RGBA(buffer[i].unpremultiplied().toArgb32());
   }
}

template<QtPixelOrder PixelOrder>
static void destStore64RGB30(QRasterBuffer *rasterBuffer, int x, int y, const QRgba64 *buffer, int length)
{
   uint *dest = (uint *)rasterBuffer->scanLine(y) + x;
#ifdef __SSE2__
   qConvertARGB64PMToA2RGB30PM_sse2<PixelOrder>(dest, buffer, length);
#else
   for (int i = 0; i < length; ++i) {
      dest[i] = qConvertRgb64ToRgb30<PixelOrder>(buffer[i]);
   }
#endif
}

static DestStoreProc destStoreProc[QImage::NImageFormats] = {
   0,                  // Format_Invalid
   destStoreMono,      // Format_Mono,
   destStoreMonoLsb,   // Format_MonoLSB
   0,                  // Format_Indexed8
   0,                  // Format_RGB32
   destStore,          // Format_ARGB32,
   0,                  // Format_ARGB32_Premultiplied
   destStoreRGB16,     // Format_RGB16
   destStore,          // Format_ARGB8565_Premultiplied
   destStore,          // Format_RGB666
   destStore,          // Format_ARGB6666_Premultiplied
   destStore,          // Format_RGB555
   destStore,          // Format_ARGB8555_Premultiplied
   destStore,          // Format_RGB888
   destStore,          // Format_RGB444
   destStore,          // Format_ARGB4444_Premultiplied
   destStore,          // Format_RGBX8888
   destStore,          // Format_RGBA8888
   destStore,          // Format_RGBA8888_Premultiplied
   destStore,          // Format_BGR30
   destStore,          // Format_A2BGR30_Premultiplied
   destStore,          // Format_RGB30
   destStore,          // Format_A2RGB30_Premultiplied
   destStore,          // Format_Alpha8
   destStore,          // Format_Grayscale8
};

static DestStoreProc64 destStoreProc64[QImage::NImageFormats] = {
   0,                  // Format_Invalid
   destStore64,        // Format_Mono,
   destStore64,        // Format_MonoLSB
   0,                  // Format_Indexed8
   destStore64,        // Format_RGB32
   destStore64ARGB32,  // Format_ARGB32,
   destStore64,        // Format_ARGB32_Premultiplied
   destStore64,        // Format_RGB16
   destStore64,        // Format_ARGB8565_Premultiplied
   destStore64,        // Format_RGB666
   destStore64,        // Format_ARGB6666_Premultiplied
   destStore64,        // Format_RGB555
   destStore64,        // Format_ARGB8555_Premultiplied
   destStore64,        // Format_RGB888
   destStore64,        // Format_RGB444
   destStore64,        // Format_ARGB4444_Premultiplied
   destStore64,        // Format_RGBX8888
   destStore64RGBA8888,        // Format_RGBA8888
   destStore64,        // Format_RGBA8888_Premultiplied
   destStore64RGB30<PixelOrderBGR>,        // Format_BGR30
   destStore64RGB30<PixelOrderBGR>,        // Format_A2BGR30_Premultiplied
   destStore64RGB30<PixelOrderRGB>,        // Format_RGB30
   destStore64RGB30<PixelOrderRGB>,        // Format_A2RGB30_Premultiplied
   destStore64,        // Format_Alpha8
   destStore64,        // Format_Grayscale8
};

/*
  Source fetches

  This is a bit more complicated, as we need several fetch routines for every surface type

  We need 5 fetch methods per surface type:
  untransformed
  transformed (tiled and not tiled)
  transformed bilinear (tiled and not tiled)

  We do not need bounds checks for untransformed, but we need them for the other ones.

  The generic implementation does pixel by pixel fetches
*/

enum TextureBlendType {
   BlendUntransformed,
   BlendTiled,
   BlendTransformed,
   BlendTransformedTiled,
   BlendTransformedBilinear,
   BlendTransformedBilinearTiled,
   NBlendTypes
};

static const uint *fetchUntransformed(uint *buffer, const Operator *,
   const QSpanData *data, int y, int x, int length)
{
   const QPixelLayout *layout = &qPixelLayouts[data->texture.format];
   const uint *ptr = qFetchPixels[layout->bpp](buffer, data->texture.scanLine(y), x, length);
   const QRgb *clut = data->texture.colorTable ? data->texture.colorTable->constData() : 0;
   return layout->convertToARGB32PM(buffer, ptr, length, layout, clut);
}

static const uint *fetchUntransformedARGB32PM(uint *, const Operator *,
   const QSpanData *data, int y, int x, int)
{
   const uchar *scanLine = data->texture.scanLine(y);
   return ((const uint *)scanLine) + x;
}

static const uint *fetchUntransformedRGB16(uint *buffer, const Operator *,
   const QSpanData *data, int y, int x,
   int length)
{
   const quint16 *scanLine = (const quint16 *)data->texture.scanLine(y) + x;
#ifdef QT_COMPILER_SUPPORTS_MIPS_DSPR2
   qConvertRgb16To32_asm_mips_dspr2(buffer, scanLine, length);
#else
   for (int i = 0; i < length; ++i) {
      buffer[i] = qConvertRgb16To32(scanLine[i]);
   }
#endif
   return buffer;
}

static const QRgba64 *fetchUntransformed64(QRgba64 *buffer, const Operator *,
   const QSpanData *data, int y, int x, int length)
{
   const QPixelLayout *layout = &qPixelLayouts[data->texture.format];
   const QRgb *clut = data->texture.colorTable ? data->texture.colorTable->constData() : 0;
   if (layout->bpp != QPixelLayout::BPP32) {
      uint buffer32[buffer_size];
      const uint *ptr = qFetchPixels[layout->bpp](buffer32, data->texture.scanLine(y), x, length);
      return layout->convertToARGB64PM(buffer, ptr, length, layout, clut);
   } else {
      const uint *src = (const uint *)data->texture.scanLine(y) + x;
      return layout->convertToARGB64PM(buffer, src, length, layout, clut);
   }
}

// blendType is either BlendTransformed or BlendTransformedTiled
template<TextureBlendType blendType>
static const uint *fetchTransformedARGB32PM(uint *buffer, const Operator *, const QSpanData *data,
   int y, int x, int length)
{
   int image_width = data->texture.width;
   int image_height = data->texture.height;

   const qreal cx = x + qreal(0.5);
   const qreal cy = y + qreal(0.5);

   const uint *end = buffer + length;
   uint *b = buffer;
   if (data->fast_matrix) {
      // The increment pr x in the scanline
      int fdx = (int)(data->m11 * fixed_scale);
      int fdy = (int)(data->m12 * fixed_scale);

      int fx = int((data->m21 * cy
               + data->m11 * cx + data->dx) * fixed_scale);
      int fy = int((data->m22 * cy
               + data->m12 * cx + data->dy) * fixed_scale);

      while (b < end) {
         int px = fx >> 16;
         int py = fy >> 16;

         if (blendType == BlendTransformedTiled) {
            px %= image_width;
            py %= image_height;

            if (px < 0) {
               px += image_width;
            }

            if (py < 0) {
               py += image_height;
            }

         } else {
            px = qBound(0, px, image_width - 1);
            py = qBound(0, py, image_height - 1);
         }
         *b = reinterpret_cast<const uint *>(data->texture.scanLine(py))[px];

         fx += fdx;
         fy += fdy;
         ++b;
      }
   } else {
      const qreal fdx = data->m11;
      const qreal fdy = data->m12;
      const qreal fdw = data->m13;

      qreal fx = data->m21 * cy + data->m11 * cx + data->dx;
      qreal fy = data->m22 * cy + data->m12 * cx + data->dy;
      qreal fw = data->m23 * cy + data->m13 * cx + data->m33;

      while (b < end) {
         const qreal iw = fw == 0 ? 1 : 1 / fw;
         const qreal tx = fx * iw;
         const qreal ty = fy * iw;
         int px = int(tx) - (tx < 0);
         int py = int(ty) - (ty < 0);

         if (blendType == BlendTransformedTiled) {
            px %= image_width;
            py %= image_height;

            if (px < 0) {
               px += image_width;
            }

            if (py < 0) {
               py += image_height;
            }

         } else {
            px = qBound(0, px, image_width - 1);
            py = qBound(0, py, image_height - 1);
         }
         *b = reinterpret_cast<const uint *>(data->texture.scanLine(py))[px];

         fx += fdx;
         fy += fdy;
         fw += fdw;
         //force increment to avoid /0
         if (!fw) {
            fw += fdw;
         }
         ++b;
      }
   }
   return buffer;
}

template<TextureBlendType blendType>  /* either BlendTransformed or BlendTransformedTiled */
static const uint *fetchTransformed(uint *buffer, const Operator *, const QSpanData *data,
   int y, int x, int length)
{
   int image_width = data->texture.width;
   int image_height = data->texture.height;

   const qreal cx = x + qreal(0.5);
   const qreal cy = y + qreal(0.5);

   const QPixelLayout *layout = &qPixelLayouts[data->texture.format];
   FetchPixelFunc fetch = qFetchPixel[layout->bpp];

   const uint *end = buffer + length;
   uint *b = buffer;
   if (data->fast_matrix) {
      // The increment pr x in the scanline
      int fdx = (int)(data->m11 * fixed_scale);
      int fdy = (int)(data->m12 * fixed_scale);

      int fx = int((data->m21 * cy
               + data->m11 * cx + data->dx) * fixed_scale);
      int fy = int((data->m22 * cy
               + data->m12 * cx + data->dy) * fixed_scale);

      while (b < end) {
         int px = fx >> 16;
         int py = fy >> 16;

         if (blendType == BlendTransformedTiled) {
            px %= image_width;
            py %= image_height;

            if (px < 0) {
               px += image_width;
            }

            if (py < 0) {
               py += image_height;
            }

         } else {
            px = qBound(0, px, image_width - 1);
            py = qBound(0, py, image_height - 1);
         }
         *b = fetch(data->texture.scanLine(py), px);

         fx += fdx;
         fy += fdy;
         ++b;
      }
   } else {
      const qreal fdx = data->m11;
      const qreal fdy = data->m12;
      const qreal fdw = data->m13;

      qreal fx = data->m21 * cy + data->m11 * cx + data->dx;
      qreal fy = data->m22 * cy + data->m12 * cx + data->dy;
      qreal fw = data->m23 * cy + data->m13 * cx + data->m33;

      while (b < end) {
         const qreal iw = fw == 0 ? 1 : 1 / fw;
         const qreal tx = fx * iw;
         const qreal ty = fy * iw;
         int px = int(tx) - (tx < 0);
         int py = int(ty) - (ty < 0);

         if (blendType == BlendTransformedTiled) {
            px %= image_width;
            py %= image_height;

            if (px < 0) {
               px += image_width;
            }

            if (py < 0) {
               py += image_height;
            }

         } else {
            px = qBound(0, px, image_width - 1);
            py = qBound(0, py, image_height - 1);
         }
         *b = fetch(data->texture.scanLine(py), px);

         fx += fdx;
         fy += fdy;
         fw += fdw;
         //force increment to avoid /0
         if (!fw) {
            fw += fdw;
         }
         ++b;
      }
   }
   const QRgb *clut = data->texture.colorTable ? data->texture.colorTable->constData() : 0;
   return layout->convertToARGB32PM(buffer, buffer, length, layout, clut);
}

template<TextureBlendType blendType>  /* either BlendTransformed or BlendTransformedTiled */
static const QRgba64 *fetchTransformed64(QRgba64 *buffer, const Operator *, const QSpanData *data,
   int y, int x, int length)
{
   int image_width = data->texture.width;
   int image_height = data->texture.height;

   const qreal cx = x + qreal(0.5);
   const qreal cy = y + qreal(0.5);

   const QPixelLayout *layout = &qPixelLayouts[data->texture.format];
   FetchPixelFunc fetch = qFetchPixel[layout->bpp];
   const QRgb *clut = data->texture.colorTable ? data->texture.colorTable->constData() : 0;

   uint buffer32[buffer_size];
   QRgba64 *b = buffer;
   if (data->fast_matrix) {
      // The increment pr x in the scanline
      int fdx = (int)(data->m11 * fixed_scale);
      int fdy = (int)(data->m12 * fixed_scale);

      int fx = int((data->m21 * cy
               + data->m11 * cx + data->dx) * fixed_scale);
      int fy = int((data->m22 * cy
               + data->m12 * cx + data->dy) * fixed_scale);

      int i = 0,  j = 0;
      while (i < length) {
         if (j == buffer_size) {
            layout->convertToARGB64PM(b, buffer32, buffer_size, layout, clut);
            b += buffer_size;
            j = 0;
         }
         int px = fx >> 16;
         int py = fy >> 16;

         if (blendType == BlendTransformedTiled) {
            px %= image_width;
            py %= image_height;
            if (px < 0) {
               px += image_width;
            }
            if (py < 0) {
               py += image_height;
            }
         } else {
            px = qBound(0, px, image_width - 1);
            py = qBound(0, py, image_height - 1);
         }
         buffer32[j] = fetch(data->texture.scanLine(py), px);

         fx += fdx;
         fy += fdy;
         ++i;
         ++j;
      }
      if (j > 0) {
         layout->convertToARGB64PM(b, buffer32, j, layout, clut);
         b += j;
      }
   } else {
      const qreal fdx = data->m11;
      const qreal fdy = data->m12;
      const qreal fdw = data->m13;

      qreal fx = data->m21 * cy + data->m11 * cx + data->dx;
      qreal fy = data->m22 * cy + data->m12 * cx + data->dy;
      qreal fw = data->m23 * cy + data->m13 * cx + data->m33;

      int i = 0,  j = 0;
      while (i < length) {
         if (j == buffer_size) {
            layout->convertToARGB64PM(b, buffer32, buffer_size, layout, clut);
            b += buffer_size;
            j = 0;
         }
         const qreal iw = fw == 0 ? 1 : 1 / fw;
         const qreal tx = fx * iw;
         const qreal ty = fy * iw;
         int px = int(tx) - (tx < 0);
         int py = int(ty) - (ty < 0);

         if (blendType == BlendTransformedTiled) {
            px %= image_width;
            py %= image_height;
            if (px < 0) {
               px += image_width;
            }
            if (py < 0) {
               py += image_height;
            }
         } else {
            px = qBound(0, px, image_width - 1);
            py = qBound(0, py, image_height - 1);
         }
         buffer32[j] = fetch(data->texture.scanLine(py), px);

         fx += fdx;
         fy += fdy;
         fw += fdw;
         //force increment to avoid /0
         if (!fw) {
            fw += fdw;
         }

         ++i;
         ++j;
      }
      if (j > 0) {
         layout->convertToARGB64PM(b, buffer32, j, layout, clut);
         b += j;
      }
   }
   return buffer;
}

/** \internal
  interpolate 4 argb pixels with the distx and disty factor.
  distx and disty bust be between 0 and 16
 */
static inline uint interpolate_4_pixels_16(uint tl, uint tr, uint bl, uint br, int distx, int disty)
{
   uint distxy = distx * disty;
   //idistx * disty = (16-distx) * disty = 16*disty - distxy
   //idistx * idisty = (16-distx) * (16-disty) = 16*16 - 16*distx -16*disty + distxy
   uint tlrb = (tl & 0x00ff00ff)         * (16 * 16 - 16 * distx - 16 * disty + distxy);
   uint tlag = ((tl & 0xff00ff00) >> 8)  * (16 * 16 - 16 * distx - 16 * disty + distxy);
   uint trrb = ((tr & 0x00ff00ff)        * (distx * 16 - distxy));
   uint trag = (((tr & 0xff00ff00) >> 8) * (distx * 16 - distxy));
   uint blrb = ((bl & 0x00ff00ff)        * (disty * 16 - distxy));
   uint blag = (((bl & 0xff00ff00) >> 8) * (disty * 16 - distxy));
   uint brrb = ((br & 0x00ff00ff)        * (distxy));
   uint brag = (((br & 0xff00ff00) >> 8) * (distxy));
   return (((tlrb + trrb + blrb + brrb) >> 8) & 0x00ff00ff) | ((tlag + trag + blag + brag) & 0xff00ff00);
}

#if defined(__SSE2__)
#define interpolate_4_pixels_16_sse2(tl, tr, bl, br, distx, disty, colorMask, v_256, b)  \
{ \
    const __m128i dxdy = _mm_mullo_epi16 (distx, disty); \
    const __m128i distx_ = _mm_slli_epi16(distx, 4); \
    const __m128i disty_ = _mm_slli_epi16(disty, 4); \
    const __m128i idxidy =  _mm_add_epi16(dxdy, _mm_sub_epi16(v_256, _mm_add_epi16(distx_, disty_))); \
    const __m128i dxidy =  _mm_sub_epi16(distx_, dxdy); \
    const __m128i idxdy =  _mm_sub_epi16(disty_, dxdy); \
 \
    __m128i tlAG = _mm_srli_epi16(tl, 8); \
    __m128i tlRB = _mm_and_si128(tl, colorMask); \
    __m128i trAG = _mm_srli_epi16(tr, 8); \
    __m128i trRB = _mm_and_si128(tr, colorMask); \
    __m128i blAG = _mm_srli_epi16(bl, 8); \
    __m128i blRB = _mm_and_si128(bl, colorMask); \
    __m128i brAG = _mm_srli_epi16(br, 8); \
    __m128i brRB = _mm_and_si128(br, colorMask); \
 \
    tlAG = _mm_mullo_epi16(tlAG, idxidy); \
    tlRB = _mm_mullo_epi16(tlRB, idxidy); \
    trAG = _mm_mullo_epi16(trAG, dxidy); \
    trRB = _mm_mullo_epi16(trRB, dxidy); \
    blAG = _mm_mullo_epi16(blAG, idxdy); \
    blRB = _mm_mullo_epi16(blRB, idxdy); \
    brAG = _mm_mullo_epi16(brAG, dxdy); \
    brRB = _mm_mullo_epi16(brRB, dxdy); \
 \
    /* Add the values, and shift to only keep 8 significant bits per colors */ \
    __m128i rAG =_mm_add_epi16(_mm_add_epi16(tlAG, trAG), _mm_add_epi16(blAG, brAG)); \
    __m128i rRB =_mm_add_epi16(_mm_add_epi16(tlRB, trRB), _mm_add_epi16(blRB, brRB)); \
    rAG = _mm_andnot_si128(colorMask, rAG); \
    rRB = _mm_srli_epi16(rRB, 8); \
    _mm_storeu_si128((__m128i*)(b), _mm_or_si128(rAG, rRB)); \
}
#endif

#if defined(__ARM_NEON__)
#define interpolate_4_pixels_16_neon(tl, tr, bl, br, distx, disty, disty_, colorMask, invColorMask, v_256, b)  \
{ \
    const int16x8_t dxdy = vmulq_s16(distx, disty); \
    const int16x8_t distx_ = vshlq_n_s16(distx, 4); \
    const int16x8_t idxidy =  vaddq_s16(dxdy, vsubq_s16(v_256, vaddq_s16(distx_, disty_))); \
    const int16x8_t dxidy =  vsubq_s16(distx_, dxdy); \
    const int16x8_t idxdy =  vsubq_s16(disty_, dxdy); \
 \
    int16x8_t tlAG = vreinterpretq_s16_u16(vshrq_n_u16(vreinterpretq_u16_s16(tl), 8)); \
    int16x8_t tlRB = vandq_s16(tl, colorMask); \
    int16x8_t trAG = vreinterpretq_s16_u16(vshrq_n_u16(vreinterpretq_u16_s16(tr), 8)); \
    int16x8_t trRB = vandq_s16(tr, colorMask); \
    int16x8_t blAG = vreinterpretq_s16_u16(vshrq_n_u16(vreinterpretq_u16_s16(bl), 8)); \
    int16x8_t blRB = vandq_s16(bl, colorMask); \
    int16x8_t brAG = vreinterpretq_s16_u16(vshrq_n_u16(vreinterpretq_u16_s16(br), 8)); \
    int16x8_t brRB = vandq_s16(br, colorMask); \
 \
    int16x8_t rAG = vmulq_s16(tlAG, idxidy); \
    int16x8_t rRB = vmulq_s16(tlRB, idxidy); \
    rAG = vmlaq_s16(rAG, trAG, dxidy); \
    rRB = vmlaq_s16(rRB, trRB, dxidy); \
    rAG = vmlaq_s16(rAG, blAG, idxdy); \
    rRB = vmlaq_s16(rRB, blRB, idxdy); \
    rAG = vmlaq_s16(rAG, brAG, dxdy); \
    rRB = vmlaq_s16(rRB, brRB, dxdy); \
 \
    rAG = vandq_s16(invColorMask, rAG); \
    rRB = vreinterpretq_s16_u16(vshrq_n_u16(vreinterpretq_u16_s16(rRB), 8)); \
    vst1q_s16((int16_t*)(b), vorrq_s16(rAG, rRB)); \
}
#endif

#if defined(__SSE2__)
static inline QRgba64 interpolate_4_pixels_rgb64(QRgba64 t[], QRgba64 b[], uint distx, uint disty)
{
   const __m128i vdistx = _mm_shufflelo_epi16(_mm_cvtsi32_si128(distx), _MM_SHUFFLE(0, 0, 0, 0));
   const __m128i vidistx = _mm_shufflelo_epi16(_mm_cvtsi32_si128(0x10000 - distx), _MM_SHUFFLE(0, 0, 0, 0));

   __m128i vt = _mm_loadu_si128((const __m128i *)t);
   if (disty) {
      __m128i vb = _mm_loadu_si128((const __m128i *)b);
      vt = _mm_mulhi_epu16(vt, _mm_set1_epi16(0x10000 - disty));
      vb = _mm_mulhi_epu16(vb, _mm_set1_epi16(disty));
      vt = _mm_add_epi16(vt, vb);
   }
   vt = _mm_mulhi_epu16(vt, _mm_unpacklo_epi64(vidistx, vdistx));
   vt = _mm_add_epi16(vt, _mm_srli_si128(vt, 8));
#ifdef Q_PROCESSOR_X86_64
   return QRgba64::fromRgba64(_mm_cvtsi128_si64(vt));
#else
   QRgba64 out;
   _mm_storel_epi64((__m128i *)&out, vt);
   return out;
#endif
}
#else
static inline QRgba64 interpolate_4_pixels_rgb64(QRgba64 t[], QRgba64 b[], uint distx, uint disty)
{
   const uint dx = distx >> 8;
   const uint dy = disty >> 8;
   const uint idx = 256 - dx;
   const uint idy = 256 - dy;
   QRgba64 xtop = interpolate256(t[0], idx, t[1], dx);
   QRgba64 xbot = interpolate256(b[0], idx, b[1], dx);
   return interpolate256(xtop, idy, xbot, dy);
}
#endif

template<TextureBlendType blendType>
void fetchTransformedBilinear_pixelBounds(int max, int l1, int l2, int &v1, int &v2);

template<>
inline void fetchTransformedBilinear_pixelBounds<BlendTransformedBilinearTiled>(int max, int, int, int &v1, int &v2)
{
   v1 %= max;
   if (v1 < 0) {
      v1 += max;
   }
   v2 = v1 + 1;
   if (v2 == max) {
      v2 = 0;
   }
   Q_ASSERT(v1 >= 0 && v1 < max);
   Q_ASSERT(v2 >= 0 && v2 < max);
}

template<>
inline void fetchTransformedBilinear_pixelBounds<BlendTransformedBilinear>(int, int l1, int l2, int &v1, int &v2)
{
   if (v1 < l1) {
      v2 = v1 = l1;
   } else if (v1 >= l2) {
      v2 = v1 = l2;
   } else {
      v2 = v1 + 1;
   }
   Q_ASSERT(v1 >= l1 && v1 <= l2);
   Q_ASSERT(v2 >= l1 && v2 <= l2);
}

template<TextureBlendType blendType> /* blendType = BlendTransformedBilinear or BlendTransformedBilinearTiled */
static const uint *fetchTransformedBilinearARGB32PM(uint *buffer, const Operator *,
   const QSpanData *data, int y, int x,
   int length)
{
   int image_width = data->texture.width;
   int image_height = data->texture.height;

   int image_x1 = data->texture.x1;
   int image_y1 = data->texture.y1;
   int image_x2 = data->texture.x2 - 1;
   int image_y2 = data->texture.y2 - 1;

   const qreal cx = x + qreal(0.5);
   const qreal cy = y + qreal(0.5);

   uint *end = buffer + length;
   uint *b = buffer;
   if (data->fast_matrix) {
      // The increment pr x in the scanline
      int fdx = (int)(data->m11 * fixed_scale);
      int fdy = (int)(data->m12 * fixed_scale);

      int fx = int((data->m21 * cy
               + data->m11 * cx + data->dx) * fixed_scale);
      int fy = int((data->m22 * cy
               + data->m12 * cx + data->dy) * fixed_scale);

      fx -= half_point;
      fy -= half_point;

      if (fdy == 0) { //simple scale, no rotation
         int y1 = (fy >> 16);
         int y2;
         fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);
         const uint *s1 = (const uint *)data->texture.scanLine(y1);
         const uint *s2 = (const uint *)data->texture.scanLine(y2);

         if (fdx <= fixed_scale && fdx > 0) { // scale up on X
            int disty = (fy & 0x0000ffff) >> 8;
            int idisty = 256 - disty;
            int x = fx >> 16;

            // The idea is first to do the interpolation between the row s1 and the row s2
            // into an intermediate buffer, then we interpolate between two pixel of this buffer.

            // intermediate_buffer[0] is a buffer of red-blue component of the pixel, in the form 0x00RR00BB
            // intermediate_buffer[1] is the alpha-green component of the pixel, in the form 0x00AA00GG
            // +1 for the last pixel to interpolate with, and +1 for rounding errors.
            quint32 intermediate_buffer[2][buffer_size + 2];
            // count is the size used in the intermediate_buffer.
            int count = (qint64(length) * fdx + fixed_scale - 1) / fixed_scale + 2;
            Q_ASSERT(count <= buffer_size + 2); //length is supposed to be <= buffer_size and data->m11 < 1 in this case
            int f = 0;
            int lim = count;
            if (blendType == BlendTransformedBilinearTiled) {
               x %= image_width;
               if (x < 0) {
                  x += image_width;
               }
            } else {
               lim = qMin(count, image_x2 - x + 1);
               if (x < image_x1) {
                  Q_ASSERT(x <= image_x2);
                  uint t = s1[image_x1];
                  uint b = s2[image_x1];
                  quint32 rb = (((t & 0xff00ff) * idisty + (b & 0xff00ff) * disty) >> 8) & 0xff00ff;
                  quint32 ag = ((((t >> 8) & 0xff00ff) * idisty + ((b >> 8) & 0xff00ff) * disty) >> 8) & 0xff00ff;
                  do {
                     intermediate_buffer[0][f] = rb;
                     intermediate_buffer[1][f] = ag;
                     f++;
                     x++;
                  } while (x < image_x1 && f < lim);
               }
            }

            if (blendType != BlendTransformedBilinearTiled) {
#if defined(__SSE2__)
               const __m128i disty_ = _mm_set1_epi16(disty);
               const __m128i idisty_ = _mm_set1_epi16(idisty);
               const __m128i colorMask = _mm_set1_epi32(0x00ff00ff);

               lim -= 3;
               for (; f < lim; x += 4, f += 4) {
                  // Load 4 pixels from s1, and split the alpha-green and red-blue component
                  __m128i top = _mm_loadu_si128((const __m128i *)((const uint *)(s1) + x));
                  __m128i topAG = _mm_srli_epi16(top, 8);
                  __m128i topRB = _mm_and_si128(top, colorMask);
                  // Multiplies each colour component by idisty
                  topAG = _mm_mullo_epi16 (topAG, idisty_);
                  topRB = _mm_mullo_epi16 (topRB, idisty_);

                  // Same for the s2 vector
                  __m128i bottom = _mm_loadu_si128((const __m128i *)((const uint *)(s2) + x));
                  __m128i bottomAG = _mm_srli_epi16(bottom, 8);
                  __m128i bottomRB = _mm_and_si128(bottom, colorMask);
                  bottomAG = _mm_mullo_epi16 (bottomAG, disty_);
                  bottomRB = _mm_mullo_epi16 (bottomRB, disty_);

                  // Add the values, and shift to only keep 8 significant bits per colors
                  __m128i rAG = _mm_add_epi16(topAG, bottomAG);
                  rAG = _mm_srli_epi16(rAG, 8);
                  _mm_storeu_si128((__m128i *)(&intermediate_buffer[1][f]), rAG);
                  __m128i rRB = _mm_add_epi16(topRB, bottomRB);
                  rRB = _mm_srli_epi16(rRB, 8);
                  _mm_storeu_si128((__m128i *)(&intermediate_buffer[0][f]), rRB);
               }
#elif defined(__ARM_NEON__)
               const int16x8_t disty_ = vdupq_n_s16(disty);
               const int16x8_t idisty_ = vdupq_n_s16(idisty);
               const int16x8_t colorMask = vdupq_n_s16(0x00ff);

               lim -= 3;
               for (; f < lim; x += 4, f += 4) {
                  // Load 4 pixels from s1, and split the alpha-green and red-blue component
                  int16x8_t top = vld1q_s16((int16_t *)((const uint *)(s1) + x));
                  int16x8_t topAG = vreinterpretq_s16_u16(vshrq_n_u16(vreinterpretq_u16_s16(top), 8));
                  int16x8_t topRB = vandq_s16(top, colorMask);
                  // Multiplies each colour component by idisty
                  topAG = vmulq_s16(topAG, idisty_);
                  topRB = vmulq_s16(topRB, idisty_);

                  // Same for the s2 vector
                  int16x8_t bottom = vld1q_s16((int16_t *)((const uint *)(s2) + x));
                  int16x8_t bottomAG = vreinterpretq_s16_u16(vshrq_n_u16(vreinterpretq_u16_s16(bottom), 8));
                  int16x8_t bottomRB = vandq_s16(bottom, colorMask);
                  bottomAG = vmulq_s16(bottomAG, disty_);
                  bottomRB = vmulq_s16(bottomRB, disty_);

                  // Add the values, and shift to only keep 8 significant bits per colors
                  int16x8_t rAG = vaddq_s16(topAG, bottomAG);
                  rAG = vreinterpretq_s16_u16(vshrq_n_u16(vreinterpretq_u16_s16(rAG), 8));
                  vst1q_s16((int16_t *)(&intermediate_buffer[1][f]), rAG);
                  int16x8_t rRB = vaddq_s16(topRB, bottomRB);
                  rRB = vreinterpretq_s16_u16(vshrq_n_u16(vreinterpretq_u16_s16(rRB), 8));
                  vst1q_s16((int16_t *)(&intermediate_buffer[0][f]), rRB);
               }
#endif
            }
            for (; f < count; f++) { // Same as above but without sse2
               if (blendType == BlendTransformedBilinearTiled) {
                  if (x >= image_width) {
                     x -= image_width;
                  }
               } else {
                  x = qMin(x, image_x2);
               }

               uint t = s1[x];
               uint b = s2[x];

               intermediate_buffer[0][f] = (((t & 0xff00ff) * idisty + (b & 0xff00ff) * disty) >> 8) & 0xff00ff;
               intermediate_buffer[1][f] = ((((t >> 8) & 0xff00ff) * idisty + ((b >> 8) & 0xff00ff) * disty) >> 8) & 0xff00ff;
               x++;
            }
            // Now interpolate the values from the intermediate_buffer to get the final result.
            fx &= fixed_scale - 1;
            Q_ASSERT((fx >> 16) == 0);
            while (b < end) {
               int x1 = (fx >> 16);
               int x2 = x1 + 1;
               Q_ASSERT(x1 >= 0);
               Q_ASSERT(x2 < count);

               int distx = (fx & 0x0000ffff) >> 8;
               int idistx = 256 - distx;
               int rb = ((intermediate_buffer[0][x1] * idistx + intermediate_buffer[0][x2] * distx) >> 8) & 0xff00ff;
               int ag = (intermediate_buffer[1][x1] * idistx + intermediate_buffer[1][x2] * distx) & 0xff00ff00;
               *b = rb | ag;
               b++;
               fx += fdx;
            }
         } else if ((fdx < 0 && fdx > -(fixed_scale / 8)) || std::abs(data->m22) < (1. / 8.)) { // scale up more than 8x
            int y1 = (fy >> 16);
            int y2;
            fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);
            const uint *s1 = (const uint *)data->texture.scanLine(y1);
            const uint *s2 = (const uint *)data->texture.scanLine(y2);
            int disty = (fy & 0x0000ffff) >> 8;
            while (b < end) {
               int x1 = (fx >> 16);
               int x2;
               fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);
               uint tl = s1[x1];
               uint tr = s1[x2];
               uint bl = s2[x1];
               uint br = s2[x2];
               int distx = (fx & 0x0000ffff) >> 8;
               *b = interpolate_4_pixels(tl, tr, bl, br, distx, disty);

               fx += fdx;
               ++b;
            }
         } else { //scale down
            int y1 = (fy >> 16);
            int y2;
            fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);
            const uint *s1 = (const uint *)data->texture.scanLine(y1);
            const uint *s2 = (const uint *)data->texture.scanLine(y2);
            int disty = (fy & 0x0000ffff) >> 12;

            if (blendType != BlendTransformedBilinearTiled) {
#define BILINEAR_DOWNSCALE_BOUNDS_PROLOG \
                    while (b < end) { \
                        int x1 = (fx >> 16); \
                        int x2; \
                        fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2); \
                        if (x1 != x2) \
                            break; \
                        uint tl = s1[x1]; \
                        uint tr = s1[x2]; \
                        uint bl = s2[x1]; \
                        uint br = s2[x2]; \
                        int distx = (fx & 0x0000ffff) >> 12; \
                        *b = interpolate_4_pixels_16(tl, tr, bl, br, distx, disty); \
                        fx += fdx; \
                        ++b; \
                    } \
                    uint *boundedEnd; \
                    if (fdx > 0) \
                        boundedEnd = qMin(end, buffer + uint((image_x2 - (fx >> 16)) / data->m11)); \
                    else \
                        boundedEnd = qMin(end, buffer + uint((image_x1 - (fx >> 16)) / data->m11)); \
                    boundedEnd -= 3;

#if defined(__SSE2__)
               BILINEAR_DOWNSCALE_BOUNDS_PROLOG

               const __m128i colorMask = _mm_set1_epi32(0x00ff00ff);
               const __m128i v_256 = _mm_set1_epi16(256);
               const __m128i v_disty = _mm_set1_epi16(disty);
               const __m128i v_fdx = _mm_set1_epi32(fdx * 4);
               __m128i v_fx = _mm_setr_epi32(fx, fx + fdx, fx + fdx + fdx, fx + fdx + fdx + fdx);

               while (b < boundedEnd) {
                  __m128i offset = _mm_srli_epi32(v_fx, 16);
                  const int offset0 = _mm_cvtsi128_si32(offset);
                  offset = _mm_srli_si128(offset, 4);

                  const int offset1 = _mm_cvtsi128_si32(offset);
                  offset = _mm_srli_si128(offset, 4);

                  const int offset2 = _mm_cvtsi128_si32(offset);
                  offset = _mm_srli_si128(offset, 4);
                  const int offset3 = _mm_cvtsi128_si32(offset);
                  const __m128i tl = _mm_setr_epi32(s1[offset0], s1[offset1], s1[offset2], s1[offset3]);
                  const __m128i tr = _mm_setr_epi32(s1[offset0 + 1], s1[offset1 + 1], s1[offset2 + 1], s1[offset3 + 1]);
                  const __m128i bl = _mm_setr_epi32(s2[offset0], s2[offset1], s2[offset2], s2[offset3]);
                  const __m128i br = _mm_setr_epi32(s2[offset0 + 1], s2[offset1 + 1], s2[offset2 + 1], s2[offset3 + 1]);

                  __m128i v_distx = _mm_srli_epi16(v_fx, 12);
                  v_distx = _mm_shufflehi_epi16(v_distx, _MM_SHUFFLE(2, 2, 0, 0));
                  v_distx = _mm_shufflelo_epi16(v_distx, _MM_SHUFFLE(2, 2, 0, 0));

                  interpolate_4_pixels_16_sse2(tl, tr, bl, br, v_distx, v_disty, colorMask, v_256, b);
                  b += 4;
                  v_fx = _mm_add_epi32(v_fx, v_fdx);
               }
               fx = _mm_cvtsi128_si32(v_fx);
#elif defined(__ARM_NEON__)
               BILINEAR_DOWNSCALE_BOUNDS_PROLOG

               const int16x8_t colorMask = vdupq_n_s16(0x00ff);
               const int16x8_t invColorMask = vmvnq_s16(colorMask);
               const int16x8_t v_256 = vdupq_n_s16(256);
               const int16x8_t v_disty = vdupq_n_s16(disty);
               const int16x8_t v_disty_ = vshlq_n_s16(v_disty, 4);
               int32x4_t v_fdx = vdupq_n_s32(fdx * 4);

               ptrdiff_t secondLine = reinterpret_cast<const uint *>(s2) - reinterpret_cast<const uint *>(s1);

               union Vect_buffer {
                  int32x4_t vect;
                  quint32 i[4];
               };
               Vect_buffer v_fx;

               for (int i = 0; i < 4; i++) {
                  v_fx.i[i] = fx;
                  fx += fdx;
               }

               const int32x4_t v_ffff_mask = vdupq_n_s32(0x0000ffff);

               while (b < boundedEnd) {

                  Vect_buffer tl, tr, bl, br;

                  Vect_buffer v_fx_shifted;
                  v_fx_shifted.vect = vshrq_n_s32(v_fx.vect, 16);

                  int32x4_t v_distx = vshrq_n_s32(vandq_s32(v_fx.vect, v_ffff_mask), 12);

                  for (int i = 0; i < 4; i++) {
                     int x1 = v_fx_shifted.i[i];
                     const uint *addr_tl = reinterpret_cast<const uint *>(s1) + x1;
                     const uint *addr_tr = addr_tl + 1;
                     tl.i[i] = *addr_tl;
                     tr.i[i] = *addr_tr;
                     bl.i[i] = *(addr_tl + secondLine);
                     br.i[i] = *(addr_tr + secondLine);
                  }

                  v_distx = vorrq_s32(v_distx, vshlq_n_s32(v_distx, 16));

                  interpolate_4_pixels_16_neon(vreinterpretq_s16_s32(tl.vect), vreinterpretq_s16_s32(tr.vect), vreinterpretq_s16_s32(bl.vect),
                     vreinterpretq_s16_s32(br.vect), vreinterpretq_s16_s32(v_distx), v_disty, v_disty_, colorMask, invColorMask, v_256, b);
                  b += 4;
                  v_fx.vect = vaddq_s32(v_fx.vect, v_fdx);
               }
               fx = v_fx.i[0];
#endif
            }

            while (b < end) {
               int x1 = (fx >> 16);
               int x2;
               fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);
               uint tl = s1[x1];
               uint tr = s1[x2];
               uint bl = s2[x1];
               uint br = s2[x2];
               int distx = (fx & 0x0000ffff) >> 12;
               *b = interpolate_4_pixels_16(tl, tr, bl, br, distx, disty);
               fx += fdx;
               ++b;
            }
         }
      } else { //rotation
         if (std::abs(data->m11) > 8 || std::abs(data->m22) > 8) {
            //if we are zooming more than 8 times, we use 8bit precision for the position.
            while (b < end) {
               int x1 = (fx >> 16);
               int x2;
               int y1 = (fy >> 16);
               int y2;

               fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);
               fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);

               const uint *s1 = (const uint *)data->texture.scanLine(y1);
               const uint *s2 = (const uint *)data->texture.scanLine(y2);

               uint tl = s1[x1];
               uint tr = s1[x2];
               uint bl = s2[x1];
               uint br = s2[x2];

               int distx = (fx & 0x0000ffff) >> 8;
               int disty = (fy & 0x0000ffff) >> 8;

               *b = interpolate_4_pixels(tl, tr, bl, br, distx, disty);

               fx += fdx;
               fy += fdy;
               ++b;
            }
         } else {
            //we are zooming less than 8x, use 4bit precision

            if (blendType != BlendTransformedBilinearTiled) {
#define BILINEAR_ROTATE_BOUNDS_PROLOG \
                    while (b < end) { \
                        int x1 = (fx >> 16); \
                        int x2; \
                        int y1 = (fy >> 16); \
                        int y2; \
                        fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2); \
                        fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2); \
                        if (x1 != x2 && y1 != y2) \
                            break; \
                        const uint *s1 = (const uint *)data->texture.scanLine(y1); \
                        const uint *s2 = (const uint *)data->texture.scanLine(y2); \
                        uint tl = s1[x1]; \
                        uint tr = s1[x2]; \
                        uint bl = s2[x1]; \
                        uint br = s2[x2]; \
                        int distx = (fx & 0x0000ffff) >> 8; \
                        int disty = (fy & 0x0000ffff) >> 8; \
                        *b = interpolate_4_pixels(tl, tr, bl, br, distx, disty); \
                        fx += fdx; \
                        fy += fdy; \
                        ++b; \
                    } \
                    uint *boundedEnd = end - 3; \
                    boundedEnd -= 3;

#if defined(__SSE2__)
               BILINEAR_ROTATE_BOUNDS_PROLOG

               const __m128i colorMask = _mm_set1_epi32(0x00ff00ff);
               const __m128i v_256 = _mm_set1_epi16(256);
               const __m128i v_fdx = _mm_set1_epi32(fdx * 4);
               const __m128i v_fdy = _mm_set1_epi32(fdy * 4);
               __m128i v_fx = _mm_setr_epi32(fx, fx + fdx, fx + fdx + fdx, fx + fdx + fdx + fdx);
               __m128i v_fy = _mm_setr_epi32(fy, fy + fdy, fy + fdy + fdy, fy + fdy + fdy + fdy);

               const uchar *textureData = data->texture.imageData;
               const int bytesPerLine = data->texture.bytesPerLine;
               const __m128i vbpl = _mm_shufflelo_epi16(_mm_cvtsi32_si128(bytesPerLine / 4), _MM_SHUFFLE(0, 0, 0, 0));

               while (b < boundedEnd) {
                  if (fdx > 0 && (short)_mm_extract_epi16(v_fx, 7) >= image_x2) {
                     break;
                  }
                  if (fdx < 0 && (short)_mm_extract_epi16(v_fx, 7) < image_x1) {
                     break;
                  }
                  if (fdy > 0 && (short)_mm_extract_epi16(v_fy, 7) >= image_y2) {
                     break;
                  }
                  if (fdy < 0 && (short)_mm_extract_epi16(v_fy, 7) < image_y1) {
                     break;
                  }

                  const __m128i vy = _mm_packs_epi32(_mm_srli_epi32(v_fy, 16), _mm_setzero_si128());
                  // 4x16bit * 4x16bit -> 4x32bit
                  __m128i offset = _mm_unpacklo_epi16(_mm_mullo_epi16(vy, vbpl), _mm_mulhi_epi16(vy, vbpl));
                  offset = _mm_add_epi32(offset, _mm_srli_epi32(v_fx, 16));
                  const int offset0 = _mm_cvtsi128_si32(offset);
                  offset = _mm_srli_si128(offset, 4);

                  const int offset1 = _mm_cvtsi128_si32(offset);
                  offset = _mm_srli_si128(offset, 4);

                  const int offset2 = _mm_cvtsi128_si32(offset);
                  offset = _mm_srli_si128(offset, 4);
                  const int offset3 = _mm_cvtsi128_si32(offset);
                  const uint *topData = (const uint *)(textureData);
                  const __m128i tl = _mm_setr_epi32(topData[offset0], topData[offset1], topData[offset2], topData[offset3]);
                  const __m128i tr = _mm_setr_epi32(topData[offset0 + 1], topData[offset1 + 1], topData[offset2 + 1], topData[offset3 + 1]);
                  const uint *bottomData = (const uint *)(textureData + bytesPerLine);
                  const __m128i bl = _mm_setr_epi32(bottomData[offset0], bottomData[offset1], bottomData[offset2], bottomData[offset3]);
                  const __m128i br = _mm_setr_epi32(bottomData[offset0 + 1], bottomData[offset1 + 1], bottomData[offset2 + 1], bottomData[offset3 + 1]);

                  __m128i v_distx = _mm_srli_epi16(v_fx, 12);
                  __m128i v_disty = _mm_srli_epi16(v_fy, 12);
                  v_distx = _mm_shufflehi_epi16(v_distx, _MM_SHUFFLE(2, 2, 0, 0));
                  v_distx = _mm_shufflelo_epi16(v_distx, _MM_SHUFFLE(2, 2, 0, 0));
                  v_disty = _mm_shufflehi_epi16(v_disty, _MM_SHUFFLE(2, 2, 0, 0));
                  v_disty = _mm_shufflelo_epi16(v_disty, _MM_SHUFFLE(2, 2, 0, 0));

                  interpolate_4_pixels_16_sse2(tl, tr, bl, br, v_distx, v_disty, colorMask, v_256, b);
                  b += 4;
                  v_fx = _mm_add_epi32(v_fx, v_fdx);
                  v_fy = _mm_add_epi32(v_fy, v_fdy);
               }
               fx = _mm_cvtsi128_si32(v_fx);
               fy = _mm_cvtsi128_si32(v_fy);
#endif
            }

            while (b < end) {
               int x1 = (fx >> 16);
               int x2;
               int y1 = (fy >> 16);
               int y2;

               fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);
               fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);

               const uint *s1 = (const uint *)data->texture.scanLine(y1);
               const uint *s2 = (const uint *)data->texture.scanLine(y2);

               uint tl = s1[x1];
               uint tr = s1[x2];
               uint bl = s2[x1];
               uint br = s2[x2];

#if defined(__SSE2__)
               // The SSE2 optimized interpolate_4_pixels is faster than interpolate_4_pixels_16.
               int distx = (fx & 0x0000ffff) >> 8;
               int disty = (fy & 0x0000ffff) >> 8;
               *b = interpolate_4_pixels(tl, tr, bl, br, distx, disty);
#else
               int distx = (fx & 0x0000ffff) >> 12;
               int disty = (fy & 0x0000ffff) >> 12;
               *b = interpolate_4_pixels_16(tl, tr, bl, br, distx, disty);
#endif

               fx += fdx;
               fy += fdy;
               ++b;
            }
         }
      }
   } else {
      const qreal fdx = data->m11;
      const qreal fdy = data->m12;
      const qreal fdw = data->m13;

      qreal fx = data->m21 * cy + data->m11 * cx + data->dx;
      qreal fy = data->m22 * cy + data->m12 * cx + data->dy;
      qreal fw = data->m23 * cy + data->m13 * cx + data->m33;

      while (b < end) {
         const qreal iw = fw == 0 ? 1 : 1 / fw;
         const qreal px = fx * iw - qreal(0.5);
         const qreal py = fy * iw - qreal(0.5);

         int x1 = int(px) - (px < 0);
         int x2;
         int y1 = int(py) - (py < 0);
         int y2;

         int distx = int((px - x1) * 256);
         int disty = int((py - y1) * 256);

         fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);
         fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);

         const uint *s1 = (const uint *)data->texture.scanLine(y1);
         const uint *s2 = (const uint *)data->texture.scanLine(y2);

         uint tl = s1[x1];
         uint tr = s1[x2];
         uint bl = s2[x1];
         uint br = s2[x2];

         *b = interpolate_4_pixels(tl, tr, bl, br, distx, disty);

         fx += fdx;
         fy += fdy;
         fw += fdw;
         //force increment to avoid /0
         if (!fw) {
            fw += fdw;
         }
         ++b;
      }
   }

   return buffer;
}

// blendType = BlendTransformedBilinear or BlendTransformedBilinearTiled
template<TextureBlendType blendType>
static const uint *fetchTransformedBilinear(uint *buffer, const Operator *,
   const QSpanData *data, int y, int x, int length)
{
   const QPixelLayout *layout = &qPixelLayouts[data->texture.format];
   const QRgb *clut = data->texture.colorTable ? data->texture.colorTable->constData() : 0;

   int image_width = data->texture.width;
   int image_height = data->texture.height;

   int image_x1 = data->texture.x1;
   int image_y1 = data->texture.y1;
   int image_x2 = data->texture.x2 - 1;
   int image_y2 = data->texture.y2 - 1;

   const qreal cx = x + qreal(0.5);
   const qreal cy = y + qreal(0.5);

   if (data->fast_matrix) {
      // The increment pr x in the scanline
      int fdx = (int)(data->m11 * fixed_scale);
      int fdy = (int)(data->m12 * fixed_scale);

      int fx = int((data->m21 * cy + data->m11 * cx + data->dx) * fixed_scale);
      int fy = int((data->m22 * cy + data->m12 * cx + data->dy) * fixed_scale);

      fx -= half_point;
      fy -= half_point;

      if (fdy == 0) { //simple scale, no rotation
         int y1 = (fy >> 16);
         int y2;
         fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);
         const uchar *s1 = data->texture.scanLine(y1);
         const uchar *s2 = data->texture.scanLine(y2);

         if (fdx <= fixed_scale && fdx > 0) { // scale up on X
            int disty = (fy & 0x0000ffff) >> 8;
            int idisty = 256 - disty;
            int x = fx >> 16;

            // The idea is first to do the interpolation between the row s1 and the row s2
            // into an intermediate buffer, then we interpolate between two pixel of this buffer.
            FetchPixelsFunc fetch = qFetchPixels[layout->bpp];
            // +1 for the last pixel to interpolate with, and +1 for rounding errors.
            uint buf1[buffer_size + 2];
            uint buf2[buffer_size + 2];
            const uint *ptr1;
            const uint *ptr2;

            int count = (qint64(length) * fdx + fixed_scale - 1) / fixed_scale + 2;
            Q_ASSERT(count <= buffer_size + 2); //length is supposed to be <= buffer_size and data->m11 < 1 in this case

            if (blendType == BlendTransformedBilinearTiled) {
               x %= image_width;
               if (x < 0) {
                  x += image_width;
               }
               int len1 = qMin(count, image_width - x);
               int len2 = qMin(x, count - len1);

               ptr1 = fetch(buf1, s1, x, len1);
               ptr1 = layout->convertToARGB32PM(buf1, ptr1, len1, layout, clut);
               ptr2 = fetch(buf2, s2, x, len1);
               ptr2 = layout->convertToARGB32PM(buf2, ptr2, len1, layout, clut);
               for (int i = 0; i < len1; ++i) {
                  uint t = ptr1[i];
                  uint b = ptr2[i];
                  buf1[i] = (((t & 0xff00ff) * idisty + (b & 0xff00ff) * disty) >> 8) & 0xff00ff;
                  buf2[i] = ((((t >> 8) & 0xff00ff) * idisty + ((b >> 8) & 0xff00ff) * disty) >> 8) & 0xff00ff;
               }

               if (len2) {
                  ptr1 = fetch(buf1 + len1, s1, 0, len2);
                  ptr1 = layout->convertToARGB32PM(buf1 + len1, ptr1, len2, layout, clut);
                  ptr2 = fetch(buf2 + len1, s2, 0, len2);
                  ptr2 = layout->convertToARGB32PM(buf2 + len1, ptr2, len2, layout, clut);
                  for (int i = 0; i < len2; ++i) {
                     uint t = ptr1[i];
                     uint b = ptr2[i];
                     buf1[i + len1] = (((t & 0xff00ff) * idisty + (b & 0xff00ff) * disty) >> 8) & 0xff00ff;
                     buf2[i + len1] = ((((t >> 8) & 0xff00ff) * idisty + ((b >> 8) & 0xff00ff) * disty) >> 8) & 0xff00ff;
                  }
               }
               for (int i = image_width; i < count; ++i) {
                  buf1[i] = buf1[i - image_width];
                  buf2[i] = buf2[i - image_width];
               }
            } else {
               int start = qMax(x, image_x1);
               int end = qMin(x + count, image_x2 + 1);
               int len = qMax(1, end - start);
               int leading = start - x;

               ptr1 = fetch(buf1 + leading, s1, start, len);
               ptr1 = layout->convertToARGB32PM(buf1 + leading, ptr1, len, layout, clut);
               ptr2 = fetch(buf2 + leading, s2, start, len);
               ptr2 = layout->convertToARGB32PM(buf2 + leading, ptr2, len, layout, clut);

               for (int i = 0; i < len; ++i) {
                  uint t = ptr1[i];
                  uint b = ptr2[i];
                  buf1[i + leading] = (((t & 0xff00ff) * idisty + (b & 0xff00ff) * disty) >> 8) & 0xff00ff;
                  buf2[i + leading] = ((((t >> 8) & 0xff00ff) * idisty + ((b >> 8) & 0xff00ff) * disty) >> 8) & 0xff00ff;
               }

               for (int i = 0; i < leading; ++i) {
                  buf1[i] = buf1[leading];
                  buf2[i] = buf2[leading];
               }
               for (int i = leading + len; i < count; ++i) {
                  buf1[i] = buf1[i - 1];
                  buf2[i] = buf2[i - 1];
               }
            }

            // Now interpolate the values from the intermediate_buffer to get the final result.
            fx &= fixed_scale - 1;
            Q_ASSERT((fx >> 16) == 0);
            for (int i = 0; i < length; ++i) {
               int x1 = (fx >> 16);
               int x2 = x1 + 1;
               Q_ASSERT(x1 >= 0);
               Q_ASSERT(x2 < count);

               int distx = (fx & 0x0000ffff) >> 8;
               int idistx = 256 - distx;
               int rb = ((buf1[x1] * idistx + buf1[x2] * distx) >> 8) & 0xff00ff;
               int ag = (buf2[x1] * idistx + buf2[x2] * distx) & 0xff00ff00;
               buffer[i] = rb | ag;
               fx += fdx;
            }
         } else {
            FetchPixelFunc fetch = qFetchPixel[layout->bpp];
            uint buf1[buffer_size];
            uint buf2[buffer_size];
            uint *b = buffer;
            while (length) {
               int len = qMin(length, buffer_size / 2);
               int fracX = fx;
               for (int i = 0; i < len; ++i) {
                  int x1 = (fx >> 16);
                  int x2;
                  fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);

                  if (layout->bpp == QPixelLayout::BPP32) {
                     buf1[i * 2 + 0] = ((const uint *)s1)[x1];
                     buf1[i * 2 + 1] = ((const uint *)s1)[x2];
                     buf2[i * 2 + 0] = ((const uint *)s2)[x1];
                     buf2[i * 2 + 1] = ((const uint *)s2)[x2];
                  } else {
                     buf1[i * 2 + 0] = fetch(s1, x1);
                     buf1[i * 2 + 1] = fetch(s1, x2);
                     buf2[i * 2 + 0] = fetch(s2, x1);
                     buf2[i * 2 + 1] = fetch(s2, x2);
                  }

                  fx += fdx;
               }
               layout->convertToARGB32PM(buf1, buf1, len * 2, layout, clut);
               layout->convertToARGB32PM(buf2, buf2, len * 2, layout, clut);

               if ((fdx < 0 && fdx > -(fixed_scale / 8)) || std::abs(data->m22) < (1. / 8.)) { // scale up more than 8x
                  int disty = (fy & 0x0000ffff) >> 8;
                  for (int i = 0; i < len; ++i) {
                     uint tl = buf1[i * 2 + 0];
                     uint tr = buf1[i * 2 + 1];
                     uint bl = buf2[i * 2 + 0];
                     uint br = buf2[i * 2 + 1];
                     int distx = (fracX & 0x0000ffff) >> 8;
                     b[i] = interpolate_4_pixels(tl, tr, bl, br, distx, disty);
                     fracX += fdx;
                  }
               } else { //scale down
                  int disty = (fy & 0x0000ffff) >> 12;
                  for (int i = 0; i < len; ++i) {
                     uint tl = buf1[i * 2 + 0];
                     uint tr = buf1[i * 2 + 1];
                     uint bl = buf2[i * 2 + 0];
                     uint br = buf2[i * 2 + 1];
                     int distx = (fracX & 0x0000ffff) >> 12;
                     b[i] = interpolate_4_pixels_16(tl, tr, bl, br, distx, disty);
                     fracX += fdx;
                  }
               }
               length -= len;
               b += len;
            }
         }
      } else { //rotation
         FetchPixelFunc fetch = qFetchPixel[layout->bpp];
         uint buf1[buffer_size];
         uint buf2[buffer_size];
         uint *b = buffer;

         while (length) {
            int len = qMin(length, buffer_size / 2);
            int fracX = fx;
            int fracY = fy;
            for (int i = 0; i < len; ++i) {
               int x1 = (fx >> 16);
               int x2;
               int y1 = (fy >> 16);
               int y2;
               fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);
               fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);

               const uchar *s1 = data->texture.scanLine(y1);
               const uchar *s2 = data->texture.scanLine(y2);

               if (layout->bpp == QPixelLayout::BPP32) {
                  buf1[i * 2 + 0] = ((const uint *)s1)[x1];
                  buf1[i * 2 + 1] = ((const uint *)s1)[x2];
                  buf2[i * 2 + 0] = ((const uint *)s2)[x1];
                  buf2[i * 2 + 1] = ((const uint *)s2)[x2];
               } else {
                  buf1[i * 2 + 0] = fetch(s1, x1);
                  buf1[i * 2 + 1] = fetch(s1, x2);
                  buf2[i * 2 + 0] = fetch(s2, x1);
                  buf2[i * 2 + 1] = fetch(s2, x2);
               }

               fx += fdx;
               fy += fdy;
            }
            layout->convertToARGB32PM(buf1, buf1, len * 2, layout, clut);
            layout->convertToARGB32PM(buf2, buf2, len * 2, layout, clut);

            if (std::abs(data->m11) > 8 || std::abs(data->m22) > 8) {
               //if we are zooming more than 8 times, we use 8bit precision for the position.
               for (int i = 0; i < len; ++i) {
                  uint tl = buf1[i * 2 + 0];
                  uint tr = buf1[i * 2 + 1];
                  uint bl = buf2[i * 2 + 0];
                  uint br = buf2[i * 2 + 1];

                  int distx = (fracX & 0x0000ffff) >> 8;
                  int disty = (fracY & 0x0000ffff) >> 8;

                  b[i] = interpolate_4_pixels(tl, tr, bl, br, distx, disty);
                  fracX += fdx;
                  fracY += fdy;
               }
            } else {
               //we are zooming less than 8x, use 4bit precision
               for (int i = 0; i < len; ++i) {
                  uint tl = buf1[i * 2 + 0];
                  uint tr = buf1[i * 2 + 1];
                  uint bl = buf2[i * 2 + 0];
                  uint br = buf2[i * 2 + 1];

                  int distx = (fracX & 0x0000ffff) >> 12;
                  int disty = (fracY & 0x0000ffff) >> 12;

                  b[i] = interpolate_4_pixels_16(tl, tr, bl, br, distx, disty);
                  fracX += fdx;
                  fracY += fdy;
               }
            }

            length -= len;
            b += len;
         }
      }
   } else {
      const qreal fdx = data->m11;
      const qreal fdy = data->m12;
      const qreal fdw = data->m13;

      qreal fx = data->m21 * cy + data->m11 * cx + data->dx;
      qreal fy = data->m22 * cy + data->m12 * cx + data->dy;
      qreal fw = data->m23 * cy + data->m13 * cx + data->m33;

      FetchPixelFunc fetch = qFetchPixel[layout->bpp];
      uint buf1[buffer_size];
      uint buf2[buffer_size];
      uint *b = buffer;

      int distxs[buffer_size / 2];
      int distys[buffer_size / 2];

      while (length) {
         int len = qMin(length, buffer_size / 2);
         for (int i = 0; i < len; ++i) {
            const qreal iw = fw == 0 ? 1 : 1 / fw;
            const qreal px = fx * iw - qreal(0.5);
            const qreal py = fy * iw - qreal(0.5);

            int x1 = int(px) - (px < 0);
            int x2;
            int y1 = int(py) - (py < 0);
            int y2;

            distxs[i] = int((px - x1) * 256);
            distys[i] = int((py - y1) * 256);

            fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);
            fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);

            const uchar *s1 = data->texture.scanLine(y1);
            const uchar *s2 = data->texture.scanLine(y2);

            if (layout->bpp == QPixelLayout::BPP32) {
               buf1[i * 2 + 0] = ((const uint *)s1)[x1];
               buf1[i * 2 + 1] = ((const uint *)s1)[x2];
               buf2[i * 2 + 0] = ((const uint *)s2)[x1];
               buf2[i * 2 + 1] = ((const uint *)s2)[x2];
            } else {
               buf1[i * 2 + 0] = fetch(s1, x1);
               buf1[i * 2 + 1] = fetch(s1, x2);
               buf2[i * 2 + 0] = fetch(s2, x1);
               buf2[i * 2 + 1] = fetch(s2, x2);
            }

            fx += fdx;
            fy += fdy;
            fw += fdw;
            //force increment to avoid /0
            if (!fw) {
               fw += fdw;
            }
         }

         layout->convertToARGB32PM(buf1, buf1, len * 2, layout, clut);
         layout->convertToARGB32PM(buf2, buf2, len * 2, layout, clut);

         for (int i = 0; i < len; ++i) {
            int distx = distxs[i];
            int disty = distys[i];

            uint tl = buf1[i * 2 + 0];
            uint tr = buf1[i * 2 + 1];
            uint bl = buf2[i * 2 + 0];
            uint br = buf2[i * 2 + 1];

            b[i] = interpolate_4_pixels(tl, tr, bl, br, distx, disty);
         }
         length -= len;
         b += len;
      }
   }

   return buffer;
}

template<TextureBlendType blendType>
static const QRgba64 *fetchTransformedBilinear64(QRgba64 *buffer, const Operator *,
   const QSpanData *data, int y, int x, int length)
{
   const QPixelLayout *layout = &qPixelLayouts[data->texture.format];
   const QRgb *clut = data->texture.colorTable ? data->texture.colorTable->constData() : 0;

   int image_width = data->texture.width;
   int image_height = data->texture.height;

   int image_x1 = data->texture.x1;
   int image_y1 = data->texture.y1;
   int image_x2 = data->texture.x2 - 1;
   int image_y2 = data->texture.y2 - 1;

   const qreal cx = x + qreal(0.5);
   const qreal cy = y + qreal(0.5);

   const qreal fdx = data->m11;
   const qreal fdy = data->m12;
   const qreal fdw = data->m13;

   if (data->fast_matrix) {
      // The increment pr x in the scanline
      int fdx = (int)(data->m11 * fixed_scale);
      int fdy = (int)(data->m12 * fixed_scale);

      int fx = int((data->m21 * cy + data->m11 * cx + data->dx) * fixed_scale);
      int fy = int((data->m22 * cy + data->m12 * cx + data->dy) * fixed_scale);

      fx -= half_point;
      fy -= half_point;

      if (fdy == 0) { //simple scale, no rotation
         int y1 = (fy >> 16);
         int y2;
         fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);
         const uchar *s1 = data->texture.scanLine(y1);
         const uchar *s2 = data->texture.scanLine(y2);

         FetchPixelFunc fetch = qFetchPixel[layout->bpp];
         uint sbuf1[buffer_size];
         uint sbuf2[buffer_size];
         QRgba64 buf1[buffer_size];
         QRgba64 buf2[buffer_size];
         QRgba64 *b = buffer;
         while (length) {
            int len = qMin(length, buffer_size / 2);
            int fracX = fx;
            int i = 0;
            int disty = (fy & 0x0000ffff);
#if defined(__SSE2__)
            const __m128i vdy = _mm_set1_epi16(disty);
            const __m128i vidy = _mm_set1_epi16(0x10000 - disty);
            if (blendType != BlendTransformedBilinearTiled && layout->bpp == QPixelLayout::BPP32) {
               for (; i < len; ++i) {
                  int x1 = (fx >> 16);
                  int x2;
                  fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);

                  if (x1 != x2) {
                     break;
                  }
                  sbuf1[i * 2 + 0] = ((const uint *)s1)[x1];
                  sbuf1[i * 2 + 1] = ((const uint *)s1)[x2];
                  sbuf2[i * 2 + 0] = ((const uint *)s2)[x1];
                  sbuf2[i * 2 + 1] = ((const uint *)s2)[x2];
                  fx += fdx;
               }

               int fastLen;
               if (fdx > 0) {
                  fastLen = qMin(len, int((image_x2 - (fx >> 16)) / data->m11));
               } else {
                  fastLen = qMin(len, int((image_x1 - (fx >> 16)) / data->m11));
               }
               fastLen -= 3;

               const __m128i v_fdx = _mm_set1_epi32(fdx * 4);
               __m128i v_fx = _mm_setr_epi32(fx, fx + fdx, fx + fdx + fdx, fx + fdx + fdx + fdx);
               for (; i < fastLen; i += 4) {
                  int offset = _mm_extract_epi16(v_fx, 1);
                  sbuf1[i * 2 + 0] = ((const uint *)s1)[offset];
                  sbuf1[i * 2 + 1] = ((const uint *)s1)[offset + 1];
                  sbuf2[i * 2 + 0] = ((const uint *)s2)[offset];
                  sbuf2[i * 2 + 1] = ((const uint *)s2)[offset + 1];
                  offset = _mm_extract_epi16(v_fx, 3);
                  sbuf1[i * 2 + 2] = ((const uint *)s1)[offset];
                  sbuf1[i * 2 + 3] = ((const uint *)s1)[offset + 1];
                  sbuf2[i * 2 + 2] = ((const uint *)s2)[offset];
                  sbuf2[i * 2 + 3] = ((const uint *)s2)[offset + 1];
                  offset = _mm_extract_epi16(v_fx, 5);
                  sbuf1[i * 2 + 4] = ((const uint *)s1)[offset];
                  sbuf1[i * 2 + 5] = ((const uint *)s1)[offset + 1];
                  sbuf2[i * 2 + 4] = ((const uint *)s2)[offset];
                  sbuf2[i * 2 + 5] = ((const uint *)s2)[offset + 1];
                  offset = _mm_extract_epi16(v_fx, 7);
                  sbuf1[i * 2 + 6] = ((const uint *)s1)[offset];
                  sbuf1[i * 2 + 7] = ((const uint *)s1)[offset + 1];
                  sbuf2[i * 2 + 6] = ((const uint *)s2)[offset];
                  sbuf2[i * 2 + 7] = ((const uint *)s2)[offset + 1];
                  v_fx = _mm_add_epi32(v_fx, v_fdx);
               }
               fx = _mm_cvtsi128_si32(v_fx);
            }
#endif
            for (; i < len; ++i) {
               int x1 = (fx >> 16);
               int x2;
               fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);

               if (layout->bpp == QPixelLayout::BPP32) {
                  sbuf1[i * 2 + 0] = ((const uint *)s1)[x1];
                  sbuf1[i * 2 + 1] = ((const uint *)s1)[x2];
                  sbuf2[i * 2 + 0] = ((const uint *)s2)[x1];
                  sbuf2[i * 2 + 1] = ((const uint *)s2)[x2];

               } else {
                  sbuf1[i * 2 + 0] = fetch(s1, x1);
                  sbuf1[i * 2 + 1] = fetch(s1, x2);
                  sbuf2[i * 2 + 0] = fetch(s2, x1);
                  sbuf2[i * 2 + 1] = fetch(s2, x2);
               }

               fx += fdx;
            }
            layout->convertToARGB64PM(buf1, sbuf1, len * 2, layout, clut);
            if (disty) {
               layout->convertToARGB64PM(buf2, sbuf2, len * 2, layout, clut);
            }

            for (int i = 0; i < len; ++i) {
               int distx = (fracX & 0x0000ffff);
#if defined(__SSE2__)
               const __m128i vdistx = _mm_shufflelo_epi16(_mm_cvtsi32_si128(distx), _MM_SHUFFLE(0, 0, 0, 0));
               const __m128i vidistx = _mm_shufflelo_epi16(_mm_cvtsi32_si128(0x10000 - distx), _MM_SHUFFLE(0, 0, 0, 0));
               __m128i vt = _mm_loadu_si128((const __m128i *)(buf1 + i * 2));
               if (disty) {
                  __m128i vb = _mm_loadu_si128((const __m128i *)(buf2 + i * 2));
                  vt = _mm_mulhi_epu16(vt, vidy);
                  vb = _mm_mulhi_epu16(vb, vdy);
                  vt = _mm_add_epi16(vt, vb);
               }
               vt = _mm_mulhi_epu16(vt, _mm_unpacklo_epi64(vidistx, vdistx));
               vt = _mm_add_epi16(vt, _mm_srli_si128(vt, 8));
               _mm_storel_epi64((__m128i *)(b + i), vt);
#else
               b[i] = interpolate_4_pixels_rgb64(buf1 + i * 2, buf2 + i * 2, distx, disty);
#endif
               fracX += fdx;
            }
            length -= len;
            b += len;
         }
      } else { //rotation
         FetchPixelFunc fetch = qFetchPixel[layout->bpp];
         uint sbuf1[buffer_size];
         uint sbuf2[buffer_size];
         QRgba64 buf1[buffer_size];
         QRgba64 buf2[buffer_size];
         QRgba64 *end = buffer + length;
         QRgba64 *b = buffer;

         while (b < end) {
            int len = qMin(length, buffer_size / 2);
            int fracX = fx;
            int fracY = fy;
            int i = 0;
#if defined(__SSE2__)
            if (blendType != BlendTransformedBilinearTiled && layout->bpp == QPixelLayout::BPP32) {
               for (; i < len; ++i) {
                  int x1 = (fx >> 16);
                  int x2;
                  int y1 = (fy >> 16);
                  int y2;
                  fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);
                  fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);
                  if (x1 != x2 && y1 != y2) {
                     break;
                  }
                  const uchar *s1 = data->texture.scanLine(y1);
                  const uchar *s2 = data->texture.scanLine(y2);
                  sbuf1[i * 2 + 0] = ((const uint *)s1)[x1];
                  sbuf1[i * 2 + 1] = ((const uint *)s1)[x2];
                  sbuf2[i * 2 + 0] = ((const uint *)s2)[x1];
                  sbuf2[i * 2 + 1] = ((const uint *)s2)[x2];
                  fx += fdx;
                  fy += fdy;
               }

               const __m128i v_fdx = _mm_set1_epi32(fdx * 4);
               const __m128i v_fdy = _mm_set1_epi32(fdy * 4);
               __m128i v_fx = _mm_setr_epi32(fx, fx + fdx, fx + fdx + fdx, fx + fdx + fdx + fdx);
               __m128i v_fy = _mm_setr_epi32(fy, fy + fdy, fy + fdy + fdy, fy + fdy + fdy + fdy);
               const int bytesPerLine = data->texture.bytesPerLine;
               const uchar *s1 = data->texture.imageData;
               const uchar *s2 = s1 + bytesPerLine;
               const __m128i vbpl = _mm_shufflelo_epi16(_mm_cvtsi32_si128(bytesPerLine / 4), _MM_SHUFFLE(0, 0, 0, 0));
               for (; i < len - 3; i += 4) {
                  if (fdx > 0 && (short)_mm_extract_epi16(v_fx, 7) >= image_x2) {
                     break;
                  }
                  if (fdx < 0 && (short)_mm_extract_epi16(v_fx, 7) < image_x1) {
                     break;
                  }
                  if (fdy > 0 && (short)_mm_extract_epi16(v_fy, 7) >= image_y2) {
                     break;
                  }
                  if (fdy < 0 && (short)_mm_extract_epi16(v_fy, 7) < image_y1) {
                     break;
                  }
                  const __m128i vy = _mm_packs_epi32(_mm_srai_epi32(v_fy, 16), _mm_setzero_si128());
                  __m128i voffset = _mm_unpacklo_epi16(_mm_mullo_epi16(vy, vbpl), _mm_mulhi_epu16(vy, vbpl));
                  voffset = _mm_add_epi32(voffset, _mm_srli_epi32(v_fx, 16));

                  int offset = _mm_cvtsi128_si32(voffset);
                  voffset = _mm_srli_si128(voffset, 4);

                  sbuf1[i * 2 + 0] = ((const uint *)s1)[offset];
                  sbuf1[i * 2 + 1] = ((const uint *)s1)[offset + 1];
                  sbuf2[i * 2 + 0] = ((const uint *)s2)[offset];
                  sbuf2[i * 2 + 1] = ((const uint *)s2)[offset + 1];
                  offset = _mm_cvtsi128_si32(voffset);
                  voffset = _mm_srli_si128(voffset, 4);

                  sbuf1[i * 2 + 2] = ((const uint *)s1)[offset];
                  sbuf1[i * 2 + 3] = ((const uint *)s1)[offset + 1];
                  sbuf2[i * 2 + 2] = ((const uint *)s2)[offset];
                  sbuf2[i * 2 + 3] = ((const uint *)s2)[offset + 1];
                  offset = _mm_cvtsi128_si32(voffset);
                  voffset = _mm_srli_si128(voffset, 4);

                  sbuf1[i * 2 + 4] = ((const uint *)s1)[offset];
                  sbuf1[i * 2 + 5] = ((const uint *)s1)[offset + 1];
                  sbuf2[i * 2 + 4] = ((const uint *)s2)[offset];
                  sbuf2[i * 2 + 5] = ((const uint *)s2)[offset + 1];
                  offset = _mm_cvtsi128_si32(voffset);
                  sbuf1[i * 2 + 6] = ((const uint *)s1)[offset];
                  sbuf1[i * 2 + 7] = ((const uint *)s1)[offset + 1];
                  sbuf2[i * 2 + 6] = ((const uint *)s2)[offset];
                  sbuf2[i * 2 + 7] = ((const uint *)s2)[offset + 1];

                  v_fx = _mm_add_epi32(v_fx, v_fdx);
                  v_fy = _mm_add_epi32(v_fy, v_fdy);
               }
               fx = _mm_cvtsi128_si32(v_fx);
               fy = _mm_cvtsi128_si32(v_fy);
            }
#endif
            for (; i < len; ++i) {
               int x1 = (fx >> 16);
               int x2;
               int y1 = (fy >> 16);
               int y2;
               fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);
               fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);

               const uchar *s1 = data->texture.scanLine(y1);
               const uchar *s2 = data->texture.scanLine(y2);

               if (layout->bpp == QPixelLayout::BPP32) {
                  sbuf1[i * 2 + 0] = ((const uint *)s1)[x1];
                  sbuf1[i * 2 + 1] = ((const uint *)s1)[x2];
                  sbuf2[i * 2 + 0] = ((const uint *)s2)[x1];
                  sbuf2[i * 2 + 1] = ((const uint *)s2)[x2];

               } else {
                  sbuf1[i * 2 + 0] = fetch(s1, x1);
                  sbuf1[i * 2 + 1] = fetch(s1, x2);
                  sbuf2[i * 2 + 0] = fetch(s2, x1);
                  sbuf2[i * 2 + 1] = fetch(s2, x2);
               }

               fx += fdx;
               fy += fdy;
            }
            layout->convertToARGB64PM(buf1, sbuf1, len * 2, layout, clut);
            layout->convertToARGB64PM(buf2, sbuf2, len * 2, layout, clut);

            for (int i = 0; i < len; ++i) {
               int distx = (fracX & 0x0000ffff);
               int disty = (fracY & 0x0000ffff);
               b[i] = interpolate_4_pixels_rgb64(buf1 + i * 2, buf2 + i * 2, distx, disty);
               fracX += fdx;
               fracY += fdy;
            }

            length -= len;
            b += len;
         }
      }
   } else {
      qreal fx = data->m21 * cy + data->m11 * cx + data->dx;
      qreal fy = data->m22 * cy + data->m12 * cx + data->dy;
      qreal fw = data->m23 * cy + data->m13 * cx + data->m33;

      FetchPixelFunc fetch = qFetchPixel[layout->bpp];
      uint sbuf1[buffer_size];
      uint sbuf2[buffer_size];
      QRgba64 buf1[buffer_size];
      QRgba64 buf2[buffer_size];
      QRgba64 *b = buffer;

      int distxs[buffer_size / 2];
      int distys[buffer_size / 2];

      while (length) {
         int len = qMin(length, buffer_size / 2);
         for (int i = 0; i < len; ++i) {
            const qreal iw = fw == 0 ? 1 : 1 / fw;
            const qreal px = fx * iw - qreal(0.5);
            const qreal py = fy * iw - qreal(0.5);

            int x1 = int(px) - (px < 0);
            int x2;
            int y1 = int(py) - (py < 0);
            int y2;

            distxs[i] = int((px - x1) * (1 << 16));
            distys[i] = int((py - y1) * (1 << 16));

            fetchTransformedBilinear_pixelBounds<blendType>(image_width, image_x1, image_x2, x1, x2);
            fetchTransformedBilinear_pixelBounds<blendType>(image_height, image_y1, image_y2, y1, y2);

            const uchar *s1 = data->texture.scanLine(y1);
            const uchar *s2 = data->texture.scanLine(y2);

            if (layout->bpp == QPixelLayout::BPP32) {
               sbuf1[i * 2 + 0] = ((const uint *)s1)[x1];
               sbuf1[i * 2 + 1] = ((const uint *)s1)[x2];
               sbuf2[i * 2 + 0] = ((const uint *)s2)[x1];
               sbuf2[i * 2 + 1] = ((const uint *)s2)[x2];

            } else {
               sbuf1[i * 2 + 0] = fetch(s1, x1);
               sbuf1[i * 2 + 1] = fetch(s1, x2);
               sbuf2[i * 2 + 0] = fetch(s2, x1);
               sbuf2[i * 2 + 1] = fetch(s2, x2);
            }

            fx += fdx;
            fy += fdy;
            fw += fdw;
            //force increment to avoid /0
            if (!fw) {
               fw += fdw;
            }
         }

         layout->convertToARGB64PM(buf1, sbuf1, len * 2, layout, clut);
         layout->convertToARGB64PM(buf2, sbuf2, len * 2, layout, clut);

         for (int i = 0; i < len; ++i) {
            int distx = distxs[i];
            int disty = distys[i];
            b[i] = interpolate_4_pixels_rgb64(buf1 + i * 2, buf2 + i * 2, distx, disty);
         }

         length -= len;
         b += len;
      }
   }

   return buffer;
}

static SourceFetchProc sourceFetch[NBlendTypes][QImage::NImageFormats] = {
   // Untransformed
   {
      0, // Invalid
      fetchUntransformed,         // Mono
      fetchUntransformed,         // MonoLsb
      fetchUntransformed,         // Indexed8
      fetchUntransformedARGB32PM, // RGB32
      fetchUntransformed,         // ARGB32
      fetchUntransformedARGB32PM, // ARGB32_Premultiplied
      fetchUntransformedRGB16,    // RGB16
      fetchUntransformed,         // ARGB8565_Premultiplied
      fetchUntransformed,         // RGB666
      fetchUntransformed,         // ARGB6666_Premultiplied
      fetchUntransformed,         // RGB555
      fetchUntransformed,         // ARGB8555_Premultiplied
      fetchUntransformed,         // RGB888
      fetchUntransformed,         // RGB444
      fetchUntransformed,         // ARGB4444_Premultiplied
      fetchUntransformed,         // RGBX8888
      fetchUntransformed,         // RGBA8888
      fetchUntransformed,         // RGBA8888_Premultiplied
      fetchUntransformed,         // Format_BGR30
      fetchUntransformed,         // Format_A2BGR30_Premultiplied
      fetchUntransformed,         // Format_RGB30
      fetchUntransformed,         // Format_A2RGB30_Premultiplied
      fetchUntransformed,         // Alpha8
      fetchUntransformed,         // Grayscale8
   },
   // Tiled
   {
      0, // Invalid
      fetchUntransformed,         // Mono
      fetchUntransformed,         // MonoLsb
      fetchUntransformed,         // Indexed8
      fetchUntransformedARGB32PM, // RGB32
      fetchUntransformed,         // ARGB32
      fetchUntransformedARGB32PM, // ARGB32_Premultiplied
      fetchUntransformedRGB16,    // RGB16
      fetchUntransformed,         // ARGB8565_Premultiplied
      fetchUntransformed,         // RGB666
      fetchUntransformed,         // ARGB6666_Premultiplied
      fetchUntransformed,         // RGB555
      fetchUntransformed,         // ARGB8555_Premultiplied
      fetchUntransformed,         // RGB888
      fetchUntransformed,         // RGB444
      fetchUntransformed,         // ARGB4444_Premultiplied
      fetchUntransformed,         // RGBX8888
      fetchUntransformed,         // RGBA8888
      fetchUntransformed,         // RGBA8888_Premultiplied
      fetchUntransformed,         // BGR30
      fetchUntransformed,         // A2BGR30_Premultiplied
      fetchUntransformed,         // RGB30
      fetchUntransformed,         // A2RGB30_Premultiplied
      fetchUntransformed,         // Alpha8
      fetchUntransformed,         // Grayscale8
   },
   // Transformed
   {
      0, // Invalid
      fetchTransformed<BlendTransformed>,         // Mono
      fetchTransformed<BlendTransformed>,         // MonoLsb
      fetchTransformed<BlendTransformed>,         // Indexed8
      fetchTransformedARGB32PM<BlendTransformed>, // RGB32
      fetchTransformed<BlendTransformed>,         // ARGB32
      fetchTransformedARGB32PM<BlendTransformed>, // ARGB32_Premultiplied
      fetchTransformed<BlendTransformed>,         // RGB16
      fetchTransformed<BlendTransformed>,         // ARGB8565_Premultiplied
      fetchTransformed<BlendTransformed>,         // RGB666
      fetchTransformed<BlendTransformed>,         // ARGB6666_Premultiplied
      fetchTransformed<BlendTransformed>,         // RGB555
      fetchTransformed<BlendTransformed>,         // ARGB8555_Premultiplied
      fetchTransformed<BlendTransformed>,         // RGB888
      fetchTransformed<BlendTransformed>,         // RGB444
      fetchTransformed<BlendTransformed>,         // ARGB4444_Premultiplied
      fetchTransformed<BlendTransformed>,         // RGBX8888
      fetchTransformed<BlendTransformed>,         // RGBA8888
      fetchTransformed<BlendTransformed>,         // RGBA8888_Premultiplied
      fetchTransformed<BlendTransformed>,         // BGR30
      fetchTransformed<BlendTransformed>,         // A2BGR30_Premultiplied
      fetchTransformed<BlendTransformed>,         // RGB30
      fetchTransformed<BlendTransformed>,         // A2RGB30_Premultiplied
      fetchTransformed<BlendTransformed>,         // Alpah8
      fetchTransformed<BlendTransformed>,         // Grayscale8
   },
   {
      0, // TransformedTiled
      fetchTransformed<BlendTransformedTiled>,            // Mono
      fetchTransformed<BlendTransformedTiled>,            // MonoLsb
      fetchTransformed<BlendTransformedTiled>,            // Indexed8
      fetchTransformedARGB32PM<BlendTransformedTiled>,    // RGB32
      fetchTransformed<BlendTransformedTiled>,            // ARGB32
      fetchTransformedARGB32PM<BlendTransformedTiled>,    // ARGB32_Premultiplied
      fetchTransformed<BlendTransformedTiled>,            // RGB16
      fetchTransformed<BlendTransformedTiled>,            // ARGB8565_Premultiplied
      fetchTransformed<BlendTransformedTiled>,            // RGB666
      fetchTransformed<BlendTransformedTiled>,            // ARGB6666_Premultiplied
      fetchTransformed<BlendTransformedTiled>,            // RGB555
      fetchTransformed<BlendTransformedTiled>,            // ARGB8555_Premultiplied
      fetchTransformed<BlendTransformedTiled>,            // RGB888
      fetchTransformed<BlendTransformedTiled>,            // RGB444
      fetchTransformed<BlendTransformedTiled>,            // ARGB4444_Premultiplied
      fetchTransformed<BlendTransformedTiled>,            // RGBX8888
      fetchTransformed<BlendTransformedTiled>,            // RGBA8888
      fetchTransformed<BlendTransformedTiled>,            // RGBA8888_Premultiplied
      fetchTransformed<BlendTransformedTiled>,            // BGR30
      fetchTransformed<BlendTransformedTiled>,            // A2BGR30_Premultiplied
      fetchTransformed<BlendTransformedTiled>,            // RGB30
      fetchTransformed<BlendTransformedTiled>,            // A2RGB30_Premultiplied
      fetchTransformed<BlendTransformedTiled>,            // Alpha8
      fetchTransformed<BlendTransformedTiled>,            // Grayscale8
   },
   {
      0, // Bilinear
      fetchTransformedBilinear<BlendTransformedBilinear>,         // Mono
      fetchTransformedBilinear<BlendTransformedBilinear>,         // MonoLsb
      fetchTransformedBilinear<BlendTransformedBilinear>,         // Indexed8
      fetchTransformedBilinearARGB32PM<BlendTransformedBilinear>, // RGB32
      fetchTransformedBilinear<BlendTransformedBilinear>,         // ARGB32
      fetchTransformedBilinearARGB32PM<BlendTransformedBilinear>, // ARGB32_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinear>,         // RGB16
      fetchTransformedBilinear<BlendTransformedBilinear>,         // ARGB8565_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinear>,         // RGB666
      fetchTransformedBilinear<BlendTransformedBilinear>,         // ARGB6666_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinear>,         // RGB555
      fetchTransformedBilinear<BlendTransformedBilinear>,         // ARGB8555_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinear>,         // RGB888
      fetchTransformedBilinear<BlendTransformedBilinear>,         // RGB444
      fetchTransformedBilinear<BlendTransformedBilinear>,         // ARGB4444_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinear>,         // RGBX8888
      fetchTransformedBilinear<BlendTransformedBilinear>,         // RGBA8888
      fetchTransformedBilinear<BlendTransformedBilinear>,         // RGBA8888_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinear>,         // BGR30
      fetchTransformedBilinear<BlendTransformedBilinear>,         // A2BGR30_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinear>,         // RGB30
      fetchTransformedBilinear<BlendTransformedBilinear>,         // A2RGB30_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinear>,         // Alpha8
      fetchTransformedBilinear<BlendTransformedBilinear>,         // Grayscale8
   },
   {
      0, // BilinearTiled
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // Mono
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // MonoLsb
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // Indexed8
      fetchTransformedBilinearARGB32PM<BlendTransformedBilinearTiled>,    // RGB32
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // ARGB32
      fetchTransformedBilinearARGB32PM<BlendTransformedBilinearTiled>,    // ARGB32_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // RGB16
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // ARGB8565_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // RGB666
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // ARGB6666_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // RGB555
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // ARGB8555_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // RGB888
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // RGB444
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // ARGB4444_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // RGBX8888
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // RGBA8888
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // RGBA8888_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // BGR30
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // A2BGR30_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // RGB30
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // A2RGB30_Premultiplied
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // Alpha8
      fetchTransformedBilinear<BlendTransformedBilinearTiled>,            // Grayscale8
   },
};

static SourceFetchProc64 sourceFetch64[NBlendTypes][QImage::NImageFormats] = {
   // Untransformed
   {
      0, // Invalid
      fetchUntransformed64,         // Mono
      fetchUntransformed64,         // MonoLsb
      fetchUntransformed64,         // Indexed8
      fetchUntransformed64,         // RGB32
      fetchUntransformed64,         // ARGB32
      fetchUntransformed64,         // ARGB32_Premultiplied
      fetchUntransformed64,         // RGB16
      fetchUntransformed64,         // ARGB8565_Premultiplied
      fetchUntransformed64,         // RGB666
      fetchUntransformed64,         // ARGB6666_Premultiplied
      fetchUntransformed64,         // RGB555
      fetchUntransformed64,         // ARGB8555_Premultiplied
      fetchUntransformed64,         // RGB888
      fetchUntransformed64,         // RGB444
      fetchUntransformed64,         // ARGB4444_Premultiplied
      fetchUntransformed64,         // RGBX8888
      fetchUntransformed64,         // RGBA8888
      fetchUntransformed64,         // RGBA8888_Premultiplied
      fetchUntransformed64,         // Format_BGR30
      fetchUntransformed64,         // Format_A2BGR30_Premultiplied
      fetchUntransformed64,         // Format_RGB30
      fetchUntransformed64,         // Format_A2RGB30_Premultiplied
      fetchUntransformed64,         // Alpha8
      fetchUntransformed64,         // Grayscale8
   },
   // Tiled
   {
      0, // Invalid
      fetchUntransformed64,         // Mono
      fetchUntransformed64,         // MonoLsb
      fetchUntransformed64,         // Indexed8
      fetchUntransformed64,         // RGB32
      fetchUntransformed64,         // ARGB32
      fetchUntransformed64,         // ARGB32_Premultiplied
      fetchUntransformed64,         // RGB16
      fetchUntransformed64,         // ARGB8565_Premultiplied
      fetchUntransformed64,         // RGB666
      fetchUntransformed64,         // ARGB6666_Premultiplied
      fetchUntransformed64,         // RGB555
      fetchUntransformed64,         // ARGB8555_Premultiplied
      fetchUntransformed64,         // RGB888
      fetchUntransformed64,         // RGB444
      fetchUntransformed64,         // ARGB4444_Premultiplied
      fetchUntransformed64,         // RGBX8888
      fetchUntransformed64,         // RGBA8888
      fetchUntransformed64,         // RGBA8888_Premultiplied
      fetchUntransformed64,         // BGR30
      fetchUntransformed64,         // A2BGR30_Premultiplied
      fetchUntransformed64,         // RGB30
      fetchUntransformed64,         // A2RGB30_Premultiplied
      fetchUntransformed64,         // Alpha8
      fetchUntransformed64,         // Grayscale8
   },
   // Transformed
   {
      0, // Invalid
      fetchTransformed64<BlendTransformed>,         // Mono
      fetchTransformed64<BlendTransformed>,         // MonoLsb
      fetchTransformed64<BlendTransformed>,         // Indexed8
      fetchTransformed64<BlendTransformed>,         // RGB32
      fetchTransformed64<BlendTransformed>,         // ARGB32
      fetchTransformed64<BlendTransformed>,         // ARGB32_Premultiplied
      fetchTransformed64<BlendTransformed>,         // RGB16
      fetchTransformed64<BlendTransformed>,         // ARGB8565_Premultiplied
      fetchTransformed64<BlendTransformed>,         // RGB666
      fetchTransformed64<BlendTransformed>,         // ARGB6666_Premultiplied
      fetchTransformed64<BlendTransformed>,         // RGB555
      fetchTransformed64<BlendTransformed>,         // ARGB8555_Premultiplied
      fetchTransformed64<BlendTransformed>,         // RGB888
      fetchTransformed64<BlendTransformed>,         // RGB444
      fetchTransformed64<BlendTransformed>,         // ARGB4444_Premultiplied
      fetchTransformed64<BlendTransformed>,         // RGBX8888
      fetchTransformed64<BlendTransformed>,         // RGBA8888
      fetchTransformed64<BlendTransformed>,         // RGBA8888_Premultiplied
      fetchTransformed64<BlendTransformed>,         // BGR30
      fetchTransformed64<BlendTransformed>,         // A2BGR30_Premultiplied
      fetchTransformed64<BlendTransformed>,         // RGB30
      fetchTransformed64<BlendTransformed>,         // A2RGB30_Premultiplied
      fetchTransformed64<BlendTransformed>,         // Alpah8
      fetchTransformed64<BlendTransformed>,         // Grayscale8
   },
   {
      0, // TransformedTiled
      fetchTransformed64<BlendTransformedTiled>,            // Mono
      fetchTransformed64<BlendTransformedTiled>,            // MonoLsb
      fetchTransformed64<BlendTransformedTiled>,            // Indexed8
      fetchTransformed64<BlendTransformedTiled>,            // RGB32
      fetchTransformed64<BlendTransformedTiled>,            // ARGB32
      fetchTransformed64<BlendTransformedTiled>,            // ARGB32_Premultiplied
      fetchTransformed64<BlendTransformedTiled>,            // RGB16
      fetchTransformed64<BlendTransformedTiled>,            // ARGB8565_Premultiplied
      fetchTransformed64<BlendTransformedTiled>,            // RGB666
      fetchTransformed64<BlendTransformedTiled>,            // ARGB6666_Premultiplied
      fetchTransformed64<BlendTransformedTiled>,            // RGB555
      fetchTransformed64<BlendTransformedTiled>,            // ARGB8555_Premultiplied
      fetchTransformed64<BlendTransformedTiled>,            // RGB888
      fetchTransformed64<BlendTransformedTiled>,            // RGB444
      fetchTransformed64<BlendTransformedTiled>,            // ARGB4444_Premultiplied
      fetchTransformed64<BlendTransformedTiled>,            // RGBX8888
      fetchTransformed64<BlendTransformedTiled>,            // RGBA8888
      fetchTransformed64<BlendTransformedTiled>,            // RGBA8888_Premultiplied
      fetchTransformed64<BlendTransformedTiled>,            // BGR30
      fetchTransformed64<BlendTransformedTiled>,            // A2BGR30_Premultiplied
      fetchTransformed64<BlendTransformedTiled>,            // RGB30
      fetchTransformed64<BlendTransformedTiled>,            // A2RGB30_Premultiplied
      fetchTransformed64<BlendTransformedTiled>,            // Alpha8
      fetchTransformed64<BlendTransformedTiled>,            // Grayscale8
   },
   {
      0, // Bilinear
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // Mono
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // MonoLsb
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // Indexed8
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // RGB32
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // ARGB32
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // ARGB32_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // RGB16
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // ARGB8565_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // RGB666
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // ARGB6666_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // RGB555
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // ARGB8555_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // RGB888
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // RGB444
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // ARGB4444_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // RGBX8888
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // RGBA8888
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // RGBA8888_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // BGR30
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // A2BGR30_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // RGB30
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // A2RGB30_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // Alpha8
      fetchTransformedBilinear64<BlendTransformedBilinear>,         // Grayscale8
   },
   {
      0, // BilinearTiled
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // Mono
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // MonoLsb
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // Indexed8
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // RGB32
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // ARGB32
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // ARGB32_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // RGB16
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // ARGB8565_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // RGB666
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // ARGB6666_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // RGB555
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // ARGB8555_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // RGB888
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // RGB444
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // ARGB4444_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // RGBX8888
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // RGBA8888
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // RGBA8888_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // BGR30
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // A2BGR30_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // RGB30
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // A2RGB30_Premultiplied
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // Alpha8
      fetchTransformedBilinear64<BlendTransformedBilinearTiled>,            // Grayscale8
   },
};

#define FIXPT_BITS 8
#define FIXPT_SIZE (1<<FIXPT_BITS)

static uint qt_gradient_pixel_fixed(const QGradientData *data, int fixed_pos)
{
   int ipos = (fixed_pos + (FIXPT_SIZE / 2)) >> FIXPT_BITS;
   return data->colorTable32[qt_gradient_clamp(data, ipos)];
}

static const QRgba64 &qt_gradient_pixel64_fixed(const QGradientData *data, int fixed_pos)
{
   int ipos = (fixed_pos + (FIXPT_SIZE / 2)) >> FIXPT_BITS;
   return data->colorTable64[qt_gradient_clamp(data, ipos)];
}

static void getLinearGradientValues(LinearGradientValues *v, const QSpanData *data)
{
   v->dx = data->gradient.linear.end.x - data->gradient.linear.origin.x;
   v->dy = data->gradient.linear.end.y - data->gradient.linear.origin.y;
   v->l = v->dx * v->dx + v->dy * v->dy;
   v->off = 0;
   if (v->l != 0) {
      v->dx /= v->l;
      v->dy /= v->l;
      v->off = -v->dx * data->gradient.linear.origin.x - v->dy * data->gradient.linear.origin.y;
   }
}

class GradientBase32
{
 public:
   typedef uint Type;
   static Type null() {
      return 0;
   }
   static Type fetchSingle(const QGradientData &gradient, qreal v) {
      return qt_gradient_pixel(&gradient, v);
   }
   static Type fetchSingle(const QGradientData &gradient, int v) {
      return qt_gradient_pixel_fixed(&gradient, v);
   }
   static void memfill(Type *buffer, Type fill, int length) {
      qt_memfill32(buffer, fill, length);
   }
};

class GradientBase64
{
 public:
   typedef QRgba64 Type;
   static Type null() {
      return QRgba64::fromRgba64(0);
   }

   static Type fetchSingle(const QGradientData &gradient, qreal v) {
      return qt_gradient_pixel64(&gradient, v);
   }

   static Type fetchSingle(const QGradientData &gradient, int v) {
      return qt_gradient_pixel64_fixed(&gradient, v);
   }

   static void memfill(Type *buffer, Type fill, int length) {
      qt_memfill64((quint64 *)buffer, fill, length);
   }
};

template<class GradientBase, typename BlendType>
static inline const BlendType *qt_fetch_linear_gradient_template(
   BlendType *buffer, const Operator *op, const QSpanData *data,
   int y, int x, int length)
{
   const BlendType *b = buffer;
   qreal t, inc;

   bool affine = true;
   qreal rx = 0, ry = 0;
   if (op->linear.l == 0) {
      t = inc = 0;
   } else {
      rx = data->m21 * (y + qreal(0.5)) + data->m11 * (x + qreal(0.5)) + data->dx;
      ry = data->m22 * (y + qreal(0.5)) + data->m12 * (x + qreal(0.5)) + data->dy;
      t = op->linear.dx * rx + op->linear.dy * ry + op->linear.off;
      inc = op->linear.dx * data->m11 + op->linear.dy * data->m12;
      affine = !data->m13 && !data->m23;

      if (affine) {
         t *= (GRADIENT_STOPTABLE_SIZE - 1);
         inc *= (GRADIENT_STOPTABLE_SIZE - 1);
      }
   }

   const BlendType *end = buffer + length;
   if (affine) {
      if (inc > qreal(-1e-5) && inc < qreal(1e-5)) {
         GradientBase::memfill(buffer, GradientBase::fetchSingle(data->gradient, int(t * FIXPT_SIZE)), length);
      } else {
         if (t + inc * length < qreal(INT_MAX >> (FIXPT_BITS + 1)) &&
            t + inc * length > qreal(INT_MIN >> (FIXPT_BITS + 1))) {
            // we can use fixed point math
            int t_fixed = int(t * FIXPT_SIZE);
            int inc_fixed = int(inc * FIXPT_SIZE);
            while (buffer < end) {
               *buffer = GradientBase::fetchSingle(data->gradient, t_fixed);
               t_fixed += inc_fixed;
               ++buffer;
            }
         } else {
            // we have to fall back to float math
            while (buffer < end) {
               *buffer = GradientBase::fetchSingle(data->gradient, t / GRADIENT_STOPTABLE_SIZE);
               t += inc;
               ++buffer;
            }
         }
      }
   } else { // fall back to float math here as well
      qreal rw = data->m23 * (y + qreal(0.5)) + data->m13 * (x + qreal(0.5)) + data->m33;
      while (buffer < end) {
         qreal x = rx / rw;
         qreal y = ry / rw;
         t = (op->linear.dx * x + op->linear.dy * y) + op->linear.off;

         *buffer = GradientBase::fetchSingle(data->gradient, t);
         rx += data->m11;
         ry += data->m12;
         rw += data->m13;
         if (!rw) {
            rw += data->m13;
         }
         ++buffer;
      }
   }

   return b;
}

static const uint *qt_fetch_linear_gradient(uint *buffer, const Operator *op, const QSpanData *data,
   int y, int x, int length)
{
   return qt_fetch_linear_gradient_template<GradientBase32, uint>(buffer, op, data, y, x, length);
}

static const QRgba64 *qt_fetch_linear_gradient_rgb64(QRgba64 *buffer, const Operator *op, const QSpanData *data,
   int y, int x, int length)
{
   return qt_fetch_linear_gradient_template<GradientBase64, QRgba64>(buffer, op, data, y, x, length);
}

static void getRadialGradientValues(RadialGradientValues *v, const QSpanData *data)
{
   v->dx = data->gradient.radial.center.x - data->gradient.radial.focal.x;
   v->dy = data->gradient.radial.center.y - data->gradient.radial.focal.y;

   v->dr = data->gradient.radial.center.radius - data->gradient.radial.focal.radius;
   v->sqrfr = data->gradient.radial.focal.radius * data->gradient.radial.focal.radius;

   v->a = v->dr * v->dr - v->dx * v->dx - v->dy * v->dy;
   v->inv2a = 1 / (2 * v->a);

   v->extended = !qFuzzyIsNull(data->gradient.radial.focal.radius) || v->a <= 0;
}

template <class GradientBase>
class RadialFetchPlain : public GradientBase
{
 public:
   typedef typename GradientBase::Type BlendType;
   static void fetch(BlendType *buffer, BlendType *end,
      const Operator *op, const QSpanData *data, qreal det,
      qreal delta_det, qreal delta_delta_det, qreal b, qreal delta_b) {
      if (op->radial.extended) {
         while (buffer < end) {
            BlendType result = GradientBase::null();
            if (det >= 0) {
               qreal w = qSqrt(det) - b;
               if (data->gradient.radial.focal.radius + op->radial.dr * w >= 0) {
                  result = GradientBase::fetchSingle(data->gradient, w);
               }
            }

            *buffer = result;

            det += delta_det;
            delta_det += delta_delta_det;
            b += delta_b;

            ++buffer;
         }
      } else {
         while (buffer < end) {
            *buffer++ = GradientBase::fetchSingle(data->gradient, qSqrt(det) - b);

            det += delta_det;
            delta_det += delta_delta_det;
            b += delta_b;
         }
      }
   }
};

const uint *qt_fetch_radial_gradient_plain(uint *buffer, const Operator *op, const QSpanData *data,
   int y, int x, int length)
{
   return qt_fetch_radial_gradient_template<RadialFetchPlain<GradientBase32>, uint>(buffer, op, data, y, x, length);
}

static SourceFetchProc qt_fetch_radial_gradient = qt_fetch_radial_gradient_plain;

const QRgba64 *qt_fetch_radial_gradient_rgb64(QRgba64 *buffer, const Operator *op, const QSpanData *data,
   int y, int x, int length)
{
   return qt_fetch_radial_gradient_template<RadialFetchPlain<GradientBase64>, QRgba64>(buffer, op, data, y, x, length);
}

template <class GradientBase, typename BlendType>
static inline const BlendType *qt_fetch_conical_gradient_template(
   BlendType *buffer, const QSpanData *data, int y, int x, int length)
{
   const BlendType *b = buffer;
   qreal rx = data->m21 * (y + qreal(0.5)) + data->dx + data->m11 * (x + qreal(0.5));
   qreal ry = data->m22 * (y + qreal(0.5)) + data->dy + data->m12 * (x + qreal(0.5));
   bool affine = !data->m13 && !data->m23;

   const qreal inv2pi = (1 / M_PI) / 2.0;

   const BlendType *end = buffer + length;

   if (affine) {
      rx -= data->gradient.conical.center.x;
      ry -= data->gradient.conical.center.y;
      while (buffer < end) {
         qreal angle = qAtan2(ry, rx) + data->gradient.conical.angle;

         *buffer = GradientBase::fetchSingle(data->gradient, 1 - angle * inv2pi);

         rx += data->m11;
         ry += data->m12;
         ++buffer;
      }
   } else {
      qreal rw = data->m23 * (y + qreal(0.5))
         + data->m33 + data->m13 * (x + qreal(0.5));
      if (!rw) {
         rw = 1;
      }
      while (buffer < end) {
         qreal angle = qAtan2(ry / rw - data->gradient.conical.center.x,
               rx / rw - data->gradient.conical.center.y)
            + data->gradient.conical.angle;

         *buffer = GradientBase::fetchSingle(data->gradient, 1 - angle * inv2pi);

         rx += data->m11;
         ry += data->m12;
         rw += data->m13;
         if (!rw) {
            rw += data->m13;
         }
         ++buffer;
      }
   }
   return b;
}

static const uint *qt_fetch_conical_gradient(uint *buffer, const Operator *, const QSpanData *data,
   int y, int x, int length)
{
   return qt_fetch_conical_gradient_template<GradientBase32, uint>(buffer, data, y, x, length);
}

static const QRgba64 *qt_fetch_conical_gradient_rgb64(QRgba64 *buffer, const Operator *, const QSpanData *data,
   int y, int x, int length)
{
   return qt_fetch_conical_gradient_template<GradientBase64, QRgba64>(buffer, data, y, x, length);
}

extern CompositionFunctionSolid qt_functionForModeSolid_C[];
extern CompositionFunctionSolid64 qt_functionForModeSolid64_C[];

static const CompositionFunctionSolid *functionForModeSolid = qt_functionForModeSolid_C;
static const CompositionFunctionSolid64 *functionForModeSolid64 = qt_functionForModeSolid64_C;

extern CompositionFunction qt_functionForMode_C[];
extern CompositionFunction64 qt_functionForMode64_C[];

static const CompositionFunction *functionForMode = qt_functionForMode_C;
static const CompositionFunction64 *functionForMode64 = qt_functionForMode64_C;

static TextureBlendType getBlendType(const QSpanData *data)
{
   TextureBlendType ft;

   if (data->txop <= QTransform::TxTranslate) {
      if (data->texture.type == QTextureData::Tiled) {
         ft = BlendTiled;

      } else {
         ft = BlendUntransformed;

      }

  } else if (data->bilinear) {

      if (data->texture.type == QTextureData::Tiled) {
         ft = BlendTransformedBilinearTiled;

      } else {
         ft = BlendTransformedBilinear;

      }

   } else if (data->texture.type == QTextureData::Tiled) {
      ft = BlendTransformedTiled;

   } else {
      ft = BlendTransformed;

   }

   return ft;
}

static inline Operator getOperator(const QSpanData *data, const QSpan *spans, int spanCount)
{
   Operator op;
   bool solidSource = false;

   switch (data->type) {
      case QSpanData::Solid:
         solidSource = data->solid.color.isOpaque();
         op.srcFetch = 0;
         op.srcFetch64 = 0;
         break;
      case QSpanData::LinearGradient:
         solidSource = !data->gradient.alphaColor;
         getLinearGradientValues(&op.linear, data);
         op.srcFetch = qt_fetch_linear_gradient;
         op.srcFetch64 = qt_fetch_linear_gradient_rgb64;
         break;
      case QSpanData::RadialGradient:
         solidSource = !data->gradient.alphaColor;
         getRadialGradientValues(&op.radial, data);
         op.srcFetch = qt_fetch_radial_gradient;
         op.srcFetch64 = qt_fetch_radial_gradient_rgb64;
         break;
      case QSpanData::ConicalGradient:
         solidSource = !data->gradient.alphaColor;
         op.srcFetch = qt_fetch_conical_gradient;
         op.srcFetch64 = qt_fetch_conical_gradient_rgb64;
         break;
      case QSpanData::Texture:
         solidSource = !data->texture.hasAlpha;
         op.srcFetch = sourceFetch[getBlendType(data)][data->texture.format];
         op.srcFetch64 = sourceFetch64[getBlendType(data)][data->texture.format];
         break;
      default:
         // error, may want to throw
         break;
   }

   op.mode = data->rasterBuffer->compositionMode;
   if (op.mode == QPainter::CompositionMode_SourceOver && solidSource) {
      op.mode = QPainter::CompositionMode_Source;
   }

   op.destFetch = destFetchProc[data->rasterBuffer->format];
   op.destFetch64 = destFetchProc64[data->rasterBuffer->format];

   if (op.mode == QPainter::CompositionMode_Source) {
      switch (data->rasterBuffer->format) {
         case QImage::Format_RGB32:
         case QImage::Format_ARGB32_Premultiplied:
            // don't clear destFetch as it sets up the pointer correctly to save one copy
            break;

         default: {
            if (data->type == QSpanData::Texture && data->texture.const_alpha != 256) {
               break;
            }
            const QSpan *lastSpan = spans + spanCount;
            bool alphaSpans = false;
            while (spans < lastSpan) {
               if (spans->coverage != 255) {
                  alphaSpans = true;
                  break;
               }
               ++spans;
            }
            if (!alphaSpans) {
               op.destFetch = 0;
            }
         }
      }
   }

   op.destStore = destStoreProc[data->rasterBuffer->format];
   op.destStore64 = destStoreProc64[data->rasterBuffer->format];

   op.funcSolid = functionForModeSolid[op.mode];
   op.funcSolid64 = functionForModeSolid64[op.mode];
   op.func = functionForMode[op.mode];
   op.func64 = functionForMode64[op.mode];

   return op;
}


// -------------------- blend methods ---------------------

static void blend_color_generic(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   uint buffer[buffer_size];
   Operator op = getOperator(data, spans, count);
   const uint color = data->solid.color.toArgb32();

   while (count--) {
      int x = spans->x;
      int length = spans->len;
      while (length) {
         int l = qMin(buffer_size, length);
         uint *dest = op.destFetch ? op.destFetch(buffer, data->rasterBuffer, x, spans->y, l) : buffer;
         op.funcSolid(dest, l, color, spans->coverage);

         if (op.destStore) {
            op.destStore(data->rasterBuffer, x, spans->y, dest, l);
         }

         length -= l;
         x += l;
      }
      ++spans;
   }
}

static void blend_color_argb(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);

   Operator op = getOperator(data, spans, count);
   const uint color = data->solid.color.toArgb32();

   if (op.mode == QPainter::CompositionMode_Source) {
      // inline for performance
      while (count--) {
         uint *target = ((uint *)data->rasterBuffer->scanLine(spans->y)) + spans->x;

         if (spans->coverage == 255) {
            qt_memfill<quint32>(target, color, spans->len);

         } else {
            uint c = BYTE_MUL(color, spans->coverage);
            int ialpha = 255 - spans->coverage;

            for (int i = 0; i < spans->len; ++i) {
               target[i] = c + BYTE_MUL(target[i], ialpha);
            }
         }
         ++spans;
      }
      return;
   }

   while (count--) {
      uint *target = ((uint *)data->rasterBuffer->scanLine(spans->y)) + spans->x;
      op.funcSolid(target, spans->len, color, spans->coverage);
      ++spans;
   }
}

void blend_color_generic_rgb64(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   Operator op = getOperator(data, spans, count);
   if (!op.funcSolid64) {
      qDebug("unsupported 64bit blend attempted");
      return blend_color_generic(count, spans, userData);
   }

   QRgba64 buffer[buffer_size];
   const QRgba64 color = data->solid.color;

   while (count--) {
      int x = spans->x;
      int length = spans->len;
      while (length) {
         int l = qMin(buffer_size, length);
         QRgba64 *dest = op.destFetch64(buffer, data->rasterBuffer, x, spans->y, l);
         op.funcSolid64(dest, l, color, spans->coverage);
         op.destStore64(data->rasterBuffer, x, spans->y, dest, l);
         length -= l;
         x += l;
      }
      ++spans;
   }
}

static void blend_color_rgb16(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);

   /*
       We duplicate a little logic from getOperator() and calculate the
       composition mode directly.  This allows blend_color_rgb16 to be used
       from qt_gradient_quint16 with minimal overhead.
    */
   QPainter::CompositionMode mode = data->rasterBuffer->compositionMode;

   if (mode == QPainter::CompositionMode_SourceOver && data->solid.color.isOpaque()) {
      mode = QPainter::CompositionMode_Source;
   }

   if (mode == QPainter::CompositionMode_Source) {
      // inline for performance
      ushort c = data->solid.color.toRgb16();

      while (count--) {
         ushort *target = ((ushort *)data->rasterBuffer->scanLine(spans->y)) + spans->x;

         if (spans->coverage == 255) {
            qt_memfill<quint16>(target, c, spans->len);

         } else {
            ushort color = BYTE_MUL_RGB16(c, spans->coverage);
            int ialpha   = 255 - spans->coverage;

            const ushort *end = target + spans->len;

            while (target < end) {
               *target = color + BYTE_MUL_RGB16(*target, ialpha);
               ++target;
            }
         }
         ++spans;
      }
      return;
   }

   if (mode == QPainter::CompositionMode_SourceOver) {
      while (count--) {
         uint color = BYTE_MUL(data->solid.color.toArgb32(), spans->coverage);
         int ialpha = qAlpha(~color);
         ushort c = qConvertRgb32To16(color);
         ushort *target = ((ushort *)data->rasterBuffer->scanLine(spans->y)) + spans->x;
         int len = spans->len;
         bool pre = (((quintptr)target) & 0x3) != 0;
         bool post = false;
         if (pre) {
            // skip to word boundary
            *target = c + BYTE_MUL_RGB16(*target, ialpha);
            ++target;
            --len;
         }
         if (len & 0x1) {
            post = true;
            --len;
         }
         uint *target32 = (uint *)target;
         uint c32 = c | (c << 16);
         len >>= 1;
         uint salpha = (ialpha + 1) >> 3; // calculate here rather than in loop
         while (len--) {
            // blend full words
            *target32 = c32 + BYTE_MUL_RGB16_32(*target32, salpha);
            ++target32;
            target += 2;
         }
         if (post) {
            // one last pixel beyond a full word
            *target = c + BYTE_MUL_RGB16(*target, ialpha);
         }
         ++spans;
      }
      return;
   }

   blend_color_generic(count, spans, userData);
}

template <typename T>
void handleSpans(int count, const QSpan *spans, const QSpanData *data, T &handler)
{
   uint const_alpha = 256;
   if (data->type == QSpanData::Texture) {
      const_alpha = data->texture.const_alpha;
   }

   int coverage = 0;
   while (count) {
      int x = spans->x;
      const int y = spans->y;
      int right = x + spans->len;

      // compute length of adjacent spans
      for (int i = 1; i < count && spans[i].y == y && spans[i].x == right; ++i) {
         right += spans[i].len;
      }
      int length = right - x;

      while (length) {
         int l = qMin(buffer_size, length);
         length -= l;

         int process_length = l;
         int process_x = x;

         const typename T::BlendType *src = handler.fetch(process_x, y, process_length);
         int offset = 0;
         while (l > 0) {
            if (x == spans->x) { // new span?
               coverage = (spans->coverage * const_alpha) >> 8;
            }

            int right = spans->x + spans->len;
            int len = qMin(l, right - x);

            handler.process(x, y, len, coverage, src, offset);

            l -= len;
            x += len;
            offset += len;

            if (x == right) { // done with current span?
               ++spans;
               --count;
            }
         }
         handler.store(process_x, y, process_length);
      }
   }
}

template<typename T>
struct QBlendBase {
   typedef T BlendType;
   QBlendBase(QSpanData *d, Operator o)
      : data(d)
      , op(o)
      , dest(0)
   { }

   QSpanData *data;
   Operator op;

   BlendType *dest;

   BlendType buffer[buffer_size];
   BlendType src_buffer[buffer_size];
};

class BlendSrcGeneric : public QBlendBase<uint>
{
 public:
   BlendSrcGeneric(QSpanData *d, Operator o)
      : QBlendBase<uint>(d, o) {
   }

   const uint *fetch(int x, int y, int len) {
      dest = op.destFetch ? op.destFetch(buffer, data->rasterBuffer, x, y, len) : buffer;
      return op.srcFetch(src_buffer, &op, data, y, x, len);
   }

   void process(int, int, int len, int coverage, const uint *src, int offset) {
      op.func(dest + offset, src + offset, len, coverage);
   }

   void store(int x, int y, int len) {
      if (op.destStore) {
         op.destStore(data->rasterBuffer, x, y, dest, len);
      }
   }
};

class BlendSrcGenericRGB64 : public QBlendBase<QRgba64>
{
 public:
   BlendSrcGenericRGB64(QSpanData *d, Operator o)
      : QBlendBase<QRgba64>(d, o) {
   }

   bool isSupported() const {
      return op.func64 && op.destFetch64 && op.destStore64;
   }

   const QRgba64 *fetch(int x, int y, int len) {
      dest = op.destFetch64(buffer, data->rasterBuffer, x, y, len);
      return op.srcFetch64(src_buffer, &op, data, y, x, len);
   }

   void process(int, int, int len, int coverage, const QRgba64 *src, int offset) {
      op.func64(dest + offset, src + offset, len, coverage);
   }

   void store(int x, int y, int len) {
      op.destStore64(data->rasterBuffer, x, y, dest, len);
   }
};

static void blend_src_generic(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   BlendSrcGeneric blend(data, getOperator(data, spans, count));
   handleSpans(count, spans, data, blend);
}

static void blend_src_generic_rgb64(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   Operator op = getOperator(data, spans, count);
   BlendSrcGenericRGB64 blend64(data, op);

   if (blend64.isSupported()) {
      handleSpans(count, spans, data, blend64);

   } else {
      qDebug("blend_src_generic_rgb64: unsupported 64-bit blend attempted");
      BlendSrcGeneric blend32(data, op);
      handleSpans(count, spans, data, blend32);
   }
}

static void blend_untransformed_generic(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);

   uint buffer[buffer_size];
   uint src_buffer[buffer_size];
   Operator op = getOperator(data, spans, count);

   const int image_width = data->texture.width;
   const int image_height = data->texture.height;
   int xoff = -qRound(-data->dx);
   int yoff = -qRound(-data->dy);

   while (count--) {
      int x = spans->x;
      int length = spans->len;
      int sx = xoff + x;
      int sy = yoff + spans->y;
      if (sy >= 0 && sy < image_height && sx < image_width) {
         if (sx < 0) {
            x -= sx;
            length += sx;
            sx = 0;
         }

         if (sx + length > image_width) {
            length = image_width - sx;
         }

         if (length > 0) {
            const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
            while (length) {
               int l = qMin(buffer_size, length);
               const uint *src = op.srcFetch(src_buffer, &op, data, sy, sx, l);
               uint *dest = op.destFetch ? op.destFetch(buffer, data->rasterBuffer, x, spans->y, l) : buffer;
               op.func(dest, src, l, coverage);
               if (op.destStore) {
                  op.destStore(data->rasterBuffer, x, spans->y, dest, l);
               }
               x += l;
               sx += l;
               length -= l;
            }
         }
      }
      ++spans;
   }
}

static void blend_untransformed_generic_rgb64(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);

   Operator op = getOperator(data, spans, count);
   if (!op.func64) {
      qWarning("Unsupported blend");
      return blend_untransformed_generic(count, spans, userData);
   }
   QRgba64 buffer[buffer_size];
   QRgba64 src_buffer[buffer_size];

   const int image_width = data->texture.width;
   const int image_height = data->texture.height;
   int xoff = -qRound(-data->dx);
   int yoff = -qRound(-data->dy);

   while (count--) {
      int x = spans->x;
      int length = spans->len;
      int sx = xoff + x;
      int sy = yoff + spans->y;
      if (sy >= 0 && sy < image_height && sx < image_width) {
         if (sx < 0) {
            x -= sx;
            length += sx;
            sx = 0;
         }

         if (sx + length > image_width) {
            length = image_width - sx;
         }

         if (length > 0) {
            const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
            while (length) {
               int l = qMin(buffer_size, length);
               const QRgba64 *src = op.srcFetch64(src_buffer, &op, data, sy, sx, l);
               QRgba64 *dest = op.destFetch64(buffer, data->rasterBuffer, x, spans->y, l);
               op.func64(dest, src, l, coverage);
               op.destStore64(data->rasterBuffer, x, spans->y, dest, l);
               x += l;
               sx += l;
               length -= l;
            }
         }
      }
      ++spans;
   }
}

static void blend_untransformed_argb(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   if (data->texture.format != QImage::Format_ARGB32_Premultiplied
      && data->texture.format != QImage::Format_RGB32) {
      blend_untransformed_generic(count, spans, userData);
      return;
   }

   Operator op = getOperator(data, spans, count);

   const int image_width = data->texture.width;
   const int image_height = data->texture.height;
   int xoff = -qRound(-data->dx);
   int yoff = -qRound(-data->dy);

   while (count--) {
      int x = spans->x;
      int length = spans->len;
      int sx = xoff + x;
      int sy = yoff + spans->y;

      if (sy >= 0 && sy < image_height && sx < image_width) {
         if (sx < 0) {
            x -= sx;
            length += sx;
            sx = 0;
         }

         if (sx + length > image_width) {
            length = image_width - sx;
         }

         if (length > 0) {
            const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
            const uint *src = (const uint *)data->texture.scanLine(sy) + sx;
            uint *dest = ((uint *)data->rasterBuffer->scanLine(spans->y)) + x;
            op.func(dest, src, length, coverage);
         }
      }
      ++spans;
   }
}

static inline quint16 interpolate_pixel_rgb16_255(quint16 x, quint8 a,
   quint16 y, quint8 b)
{
   quint16 t = ((((x & 0x07e0) * a) + ((y & 0x07e0) * b)) >> 5) & 0x07e0;
   t |= ((((x & 0xf81f) * a) + ((y & 0xf81f) * b)) >> 5) & 0xf81f;

   return t;
}

static inline quint32 interpolate_pixel_rgb16x2_255(quint32 x, quint8 a,
   quint32 y, quint8 b)
{
   uint t;
   t = ((((x & 0xf81f07e0) >> 5) * a) + (((y & 0xf81f07e0) >> 5) * b)) & 0xf81f07e0;
   t |= ((((x & 0x07e0f81f) * a) + ((y & 0x07e0f81f) * b)) >> 5) & 0x07e0f81f;
   return t;
}

static inline void blend_sourceOver_rgb16_rgb16(quint16 *__restrict dest,
   const quint16 *__restrict src,
   int length,
   const quint8 alpha,
   const quint8 ialpha)
{
   const int dstAlign = ((quintptr)dest) & 0x3;
   if (dstAlign) {
      *dest = interpolate_pixel_rgb16_255(*src, alpha, *dest, ialpha);
      ++dest;
      ++src;
      --length;
   }
   const int srcAlign = ((quintptr)src) & 0x3;
   int length32 = length >> 1;
   if (length32 && srcAlign == 0) {
      while (length32--) {
         const quint32 *src32 = reinterpret_cast<const quint32 *>(src);
         quint32 *dest32 = reinterpret_cast<quint32 *>(dest);
         *dest32 = interpolate_pixel_rgb16x2_255(*src32, alpha,
               *dest32, ialpha);
         dest += 2;
         src += 2;
      }
      length &= 0x1;
   }
   while (length--) {
      *dest = interpolate_pixel_rgb16_255(*src, alpha, *dest, ialpha);
      ++dest;
      ++src;
   }
}

static void blend_untransformed_rgb565(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   QPainter::CompositionMode mode = data->rasterBuffer->compositionMode;

   if (data->texture.format != QImage::Format_RGB16
      || (mode != QPainter::CompositionMode_SourceOver
         && mode != QPainter::CompositionMode_Source)) {
      blend_untransformed_generic(count, spans, userData);
      return;
   }

   const int image_width = data->texture.width;
   const int image_height = data->texture.height;
   int xoff = -qRound(-data->dx);
   int yoff = -qRound(-data->dy);

   while (count--) {
      const quint8 coverage = (data->texture.const_alpha * spans->coverage) >> 8;
      if (coverage == 0) {
         ++spans;
         continue;
      }

      int x = spans->x;
      int length = spans->len;
      int sx = xoff + x;
      int sy = yoff + spans->y;
      if (sy >= 0 && sy < image_height && sx < image_width) {
         if (sx < 0) {
            x -= sx;
            length += sx;
            sx = 0;
         }
         if (sx + length > image_width) {
            length = image_width - sx;
         }
         if (length > 0) {
            quint16 *dest = (quint16 *)data->rasterBuffer->scanLine(spans->y) + x;
            const quint16 *src = (const quint16 *)data->texture.scanLine(sy) + sx;
            if (coverage == 255) {
               memcpy(dest, src, length * sizeof(quint16));
            } else {
               const quint8 alpha = (coverage + 1) >> 3;
               const quint8 ialpha = 0x20 - alpha;
               if (alpha > 0) {
                  blend_sourceOver_rgb16_rgb16(dest, src, length, alpha, ialpha);
               }
            }
         }
      }
      ++spans;
   }
}

static void blend_tiled_generic(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);

   uint buffer[buffer_size];
   uint src_buffer[buffer_size];
   Operator op = getOperator(data, spans, count);

   const int image_width = data->texture.width;
   const int image_height = data->texture.height;
   int xoff = -qRound(-data->dx) % image_width;
   int yoff = -qRound(-data->dy) % image_height;

   if (xoff < 0) {
      xoff += image_width;
   }
   if (yoff < 0) {
      yoff += image_height;
   }

   while (count--) {
      int x = spans->x;
      int length = spans->len;
      int sx = (xoff + spans->x) % image_width;
      int sy = (spans->y + yoff) % image_height;
      if (sx < 0) {
         sx += image_width;
      }
      if (sy < 0) {
         sy += image_height;
      }

      const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
      while (length) {
         int l = qMin(image_width - sx, length);
         if (buffer_size < l) {
            l = buffer_size;
         }
         const uint *src = op.srcFetch(src_buffer, &op, data, sy, sx, l);
         uint *dest = op.destFetch ? op.destFetch(buffer, data->rasterBuffer, x, spans->y, l) : buffer;
         op.func(dest, src, l, coverage);
         if (op.destStore) {
            op.destStore(data->rasterBuffer, x, spans->y, dest, l);
         }
         x += l;
         sx += l;
         length -= l;
         if (sx >= image_width) {
            sx = 0;
         }
      }
      ++spans;
   }
}

static void blend_tiled_generic_rgb64(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);

   Operator op = getOperator(data, spans, count);
   if (!op.func64) {
      qDebug("unsupported rgb64 blend");
      return blend_tiled_generic(count, spans, userData);
   }
   QRgba64 buffer[buffer_size];
   QRgba64 src_buffer[buffer_size];

   const int image_width = data->texture.width;
   const int image_height = data->texture.height;
   int xoff = -qRound(-data->dx) % image_width;
   int yoff = -qRound(-data->dy) % image_height;

   if (xoff < 0) {
      xoff += image_width;
   }
   if (yoff < 0) {
      yoff += image_height;
   }

   while (count--) {
      int x = spans->x;
      int length = spans->len;
      int sx = (xoff + spans->x) % image_width;
      int sy = (spans->y + yoff) % image_height;
      if (sx < 0) {
         sx += image_width;
      }
      if (sy < 0) {
         sy += image_height;
      }

      const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
      while (length) {
         int l = qMin(image_width - sx, length);
         if (buffer_size < l) {
            l = buffer_size;
         }
         const QRgba64 *src = op.srcFetch64(src_buffer, &op, data, sy, sx, l);
         QRgba64 *dest = op.destFetch64(buffer, data->rasterBuffer, x, spans->y, l);
         op.func64(dest, src, l, coverage);
         op.destStore64(data->rasterBuffer, x, spans->y, dest, l);
         x += l;
         sx += l;
         length -= l;
         if (sx >= image_width) {
            sx = 0;
         }
      }
      ++spans;
   }
}

static void blend_tiled_argb(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   if (data->texture.format != QImage::Format_ARGB32_Premultiplied
      && data->texture.format != QImage::Format_RGB32) {
      blend_tiled_generic(count, spans, userData);
      return;
   }

   Operator op = getOperator(data, spans, count);

   int image_width = data->texture.width;
   int image_height = data->texture.height;
   int xoff = -qRound(-data->dx) % image_width;
   int yoff = -qRound(-data->dy) % image_height;

   if (xoff < 0) {
      xoff += image_width;
   }
   if (yoff < 0) {
      yoff += image_height;
   }

   while (count--) {
      int x = spans->x;
      int length = spans->len;
      int sx = (xoff + spans->x) % image_width;
      int sy = (spans->y + yoff) % image_height;
      if (sx < 0) {
         sx += image_width;
      }
      if (sy < 0) {
         sy += image_height;
      }

      const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
      while (length) {
         int l = qMin(image_width - sx, length);
         if (buffer_size < l) {
            l = buffer_size;
         }
         const uint *src = (const uint *)data->texture.scanLine(sy) + sx;
         uint *dest = ((uint *)data->rasterBuffer->scanLine(spans->y)) + x;
         op.func(dest, src, l, coverage);
         x += l;
         sx += l;
         length -= l;
         if (sx >= image_width) {
            sx = 0;
         }
      }
      ++spans;
   }
}

static void blend_tiled_rgb565(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   QPainter::CompositionMode mode = data->rasterBuffer->compositionMode;

   if (data->texture.format != QImage::Format_RGB16
      || (mode != QPainter::CompositionMode_SourceOver
         && mode != QPainter::CompositionMode_Source)) {
      blend_tiled_generic(count, spans, userData);
      return;
   }

   const int image_width = data->texture.width;
   const int image_height = data->texture.height;
   int xoff = -qRound(-data->dx) % image_width;
   int yoff = -qRound(-data->dy) % image_height;

   if (xoff < 0) {
      xoff += image_width;
   }
   if (yoff < 0) {
      yoff += image_height;
   }

   while (count--) {
      const quint8 coverage = (data->texture.const_alpha * spans->coverage) >> 8;
      if (coverage == 0) {
         ++spans;
         continue;
      }

      int x = spans->x;
      int length = spans->len;
      int sx = (xoff + spans->x) % image_width;
      int sy = (spans->y + yoff) % image_height;
      if (sx < 0) {
         sx += image_width;
      }
      if (sy < 0) {
         sy += image_height;
      }

      if (coverage == 255) {
         // Copy the first texture block
         length = qMin(image_width, length);
         int tx = x;
         while (length) {
            int l = qMin(image_width - sx, length);
            if (buffer_size < l) {
               l = buffer_size;
            }
            quint16 *dest = ((quint16 *)data->rasterBuffer->scanLine(spans->y)) + tx;
            const quint16 *src = (const quint16 *)data->texture.scanLine(sy) + sx;
            memcpy(dest, src, l * sizeof(quint16));
            length -= l;
            tx += l;
            sx += l;
            if (sx >= image_width) {
               sx = 0;
            }
         }

         // Now use the rasterBuffer as the source of the texture,
         // We can now progressively copy larger blocks
         // - Less cpu time in code figuring out what to copy
         // We are dealing with one block of data
         // - More likely to fit in the cache
         // - can use memcpy
         int copy_image_width = qMin(image_width, int(spans->len));
         length = spans->len - copy_image_width;
         quint16 *src = ((quint16 *)data->rasterBuffer->scanLine(spans->y)) + x;
         quint16 *dest = src + copy_image_width;
         while (copy_image_width < length) {
            memcpy(dest, src, copy_image_width * sizeof(quint16));
            dest += copy_image_width;
            length -= copy_image_width;
            copy_image_width *= 2;
         }
         if (length > 0) {
            memcpy(dest, src, length * sizeof(quint16));
         }
      } else {
         const quint8 alpha = (coverage + 1) >> 3;
         const quint8 ialpha = 0x20 - alpha;
         if (alpha > 0) {
            while (length) {
               int l = qMin(image_width - sx, length);
               if (buffer_size < l) {
                  l = buffer_size;
               }
               quint16 *dest = ((quint16 *)data->rasterBuffer->scanLine(spans->y)) + x;
               const quint16 *src = (const quint16 *)data->texture.scanLine(sy) + sx;
               blend_sourceOver_rgb16_rgb16(dest, src, l, alpha, ialpha);
               x += l;
               sx += l;
               length -= l;
               if (sx >= image_width) {
                  sx = 0;
               }
            }
         }
      }
      ++spans;
   }
}

static void blend_transformed_bilinear_rgb565(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   QPainter::CompositionMode mode = data->rasterBuffer->compositionMode;

   if (data->texture.format != QImage::Format_RGB16
      || (mode != QPainter::CompositionMode_SourceOver
         && mode != QPainter::CompositionMode_Source)) {
      blend_src_generic(count, spans, userData);
      return;
   }

   quint16 buffer[buffer_size];

   const int src_minx = data->texture.x1;
   const int src_miny = data->texture.y1;
   const int src_maxx = data->texture.x2 - 1;
   const int src_maxy = data->texture.y2 - 1;

   if (data->fast_matrix) {
      // The increment pr x in the scanline
      const int fdx = (int)(data->m11 * fixed_scale);
      const int fdy = (int)(data->m12 * fixed_scale);

      while (count--) {
         const quint8 coverage = (data->texture.const_alpha * spans->coverage) >> 8;
         const quint8 alpha = (coverage + 1) >> 3;
         const quint8 ialpha = 0x20 - alpha;
         if (alpha == 0) {
            ++spans;
            continue;
         }

         quint16 *dest = (quint16 *)data->rasterBuffer->scanLine(spans->y) + spans->x;
         const qreal cx = spans->x + qreal(0.5);
         const qreal cy = spans->y + qreal(0.5);
         int x = int((data->m21 * cy
                  + data->m11 * cx + data->dx) * fixed_scale) - half_point;
         int y = int((data->m22 * cy
                  + data->m12 * cx + data->dy) * fixed_scale) - half_point;
         int length = spans->len;

         while (length) {
            int l;
            quint16 *b;
            if (ialpha == 0) {
               l = length;
               b = dest;
            } else {
               l = qMin(length, buffer_size);
               b = buffer;
            }
            const quint16 *end = b + l;

            while (b < end) {
               int x1 = (x >> 16);
               int x2;
               int y1 = (y >> 16);
               int y2;

               fetchTransformedBilinear_pixelBounds<BlendTransformedBilinear>(0, src_minx, src_maxx, x1, x2);
               fetchTransformedBilinear_pixelBounds<BlendTransformedBilinear>(0, src_miny, src_maxy, y1, y2);

               const quint16 *src1 = (const quint16 *)data->texture.scanLine(y1);
               const quint16 *src2 = (const quint16 *)data->texture.scanLine(y2);
               quint16 tl = src1[x1];
               const quint16 tr = src1[x2];
               quint16 bl = src2[x1];
               const quint16 br = src2[x2];

               const uint distxsl8 = x & 0xff00;
               const uint distysl8 = y & 0xff00;
               const uint distx = distxsl8 >> 8;
               const uint disty = distysl8 >> 8;
               const uint distxy = distx * disty;

               const uint tlw = 0x10000 - distxsl8 - distysl8 + distxy; // (256 - distx) * (256 - disty)
               const uint trw = distxsl8 - distxy; // distx * (256 - disty)
               const uint blw = distysl8 - distxy; // (256 - distx) * disty
               const uint brw = distxy; // distx * disty
               uint red = ((tl & 0xf800) * tlw + (tr & 0xf800) * trw
                     + (bl & 0xf800) * blw + (br & 0xf800) * brw) & 0xf8000000;
               uint green = ((tl & 0x07e0) * tlw + (tr & 0x07e0) * trw
                     + (bl & 0x07e0) * blw + (br & 0x07e0) * brw) & 0x07e00000;
               uint blue = ((tl & 0x001f) * tlw + (tr & 0x001f) * trw
                     + (bl & 0x001f) * blw + (br & 0x001f) * brw);
               *b = quint16((red | green | blue) >> 16);

               ++b;
               x += fdx;
               y += fdy;
            }

            if (ialpha != 0) {
               blend_sourceOver_rgb16_rgb16(dest, buffer, l, alpha, ialpha);
            }

            dest += l;
            length -= l;
         }
         ++spans;
      }
   } else {
      const qreal fdx = data->m11;
      const qreal fdy = data->m12;
      const qreal fdw = data->m13;

      while (count--) {
         const quint8 coverage = (data->texture.const_alpha * spans->coverage) >> 8;
         const quint8 alpha = (coverage + 1) >> 3;
         const quint8 ialpha = 0x20 - alpha;
         if (alpha == 0) {
            ++spans;
            continue;
         }

         quint16 *dest = (quint16 *)data->rasterBuffer->scanLine(spans->y) + spans->x;

         const qreal cx = spans->x + qreal(0.5);
         const qreal cy = spans->y + qreal(0.5);

         qreal x = data->m21 * cy + data->m11 * cx + data->dx;
         qreal y = data->m22 * cy + data->m12 * cx + data->dy;
         qreal w = data->m23 * cy + data->m13 * cx + data->m33;

         int length = spans->len;
         while (length) {
            int l;
            quint16 *b;
            if (ialpha == 0) {
               l = length;
               b = dest;
            } else {
               l = qMin(length, buffer_size);
               b = buffer;
            }
            const quint16 *end = b + l;

            while (b < end) {
               const qreal iw = w == 0 ? 1 : 1 / w;
               const qreal px = x * iw - qreal(0.5);
               const qreal py = y * iw - qreal(0.5);

               int x1 = int(px) - (px < 0);
               int x2;
               int y1 = int(py) - (py < 0);
               int y2;

               fetchTransformedBilinear_pixelBounds<BlendTransformedBilinear>(0, src_minx, src_maxx, x1, x2);
               fetchTransformedBilinear_pixelBounds<BlendTransformedBilinear>(0, src_miny, src_maxy, y1, y2);

               const quint16 *src1 = (const quint16 *)data->texture.scanLine(y1);
               const quint16 *src2 = (const quint16 *)data->texture.scanLine(y2);
               quint16 tl = src1[x1];
               const quint16 tr = src1[x2];
               quint16 bl = src2[x1];
               const quint16 br = src2[x2];

               const uint distx = uint((px - x1) * 256);
               const uint disty = uint((py - y1) * 256);
               const uint distxsl8 = distx << 8;
               const uint distysl8 = disty << 8;
               const uint distxy = distx * disty;

               const uint tlw = 0x10000 - distxsl8 - distysl8 + distxy; // (256 - distx) * (256 - disty)
               const uint trw = distxsl8 - distxy; // distx * (256 - disty)
               const uint blw = distysl8 - distxy; // (256 - distx) * disty
               const uint brw = distxy; // distx * disty
               uint red = ((tl & 0xf800) * tlw + (tr & 0xf800) * trw
                     + (bl & 0xf800) * blw + (br & 0xf800) * brw) & 0xf8000000;
               uint green = ((tl & 0x07e0) * tlw + (tr & 0x07e0) * trw
                     + (bl & 0x07e0) * blw + (br & 0x07e0) * brw) & 0x07e00000;
               uint blue = ((tl & 0x001f) * tlw + (tr & 0x001f) * trw
                     + (bl & 0x001f) * blw + (br & 0x001f) * brw);
               *b = quint16((red | green | blue) >> 16);

               ++b;
               x += fdx;
               y += fdy;
               w += fdw;
            }

            if (ialpha != 0) {
               blend_sourceOver_rgb16_rgb16(dest, buffer, l, alpha, ialpha);
            }

            dest += l;
            length -= l;
         }
         ++spans;
      }
   }
}

static void blend_transformed_argb(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   if (data->texture.format != QImage::Format_ARGB32_Premultiplied
      && data->texture.format != QImage::Format_RGB32) {
      blend_src_generic(count, spans, userData);
      return;
   }

   CompositionFunction func = functionForMode[data->rasterBuffer->compositionMode];
   uint buffer[buffer_size];

   int image_width = data->texture.width;
   int image_height = data->texture.height;

   if (data->fast_matrix) {
      // The increment pr x in the scanline
      int fdx = (int)(data->m11 * fixed_scale);
      int fdy = (int)(data->m12 * fixed_scale);

      while (count--) {
         void *t = data->rasterBuffer->scanLine(spans->y);

         uint *target = ((uint *)t) + spans->x;

         const qreal cx = spans->x + qreal(0.5);
         const qreal cy = spans->y + qreal(0.5);

         int x = int((data->m21 * cy
                  + data->m11 * cx + data->dx) * fixed_scale);
         int y = int((data->m22 * cy
                  + data->m12 * cx + data->dy) * fixed_scale);

         int length = spans->len;
         const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
         while (length) {
            int l = qMin(length, buffer_size);
            const uint *end = buffer + l;
            uint *b = buffer;
            while (b < end) {
               int px = qBound(0, x >> 16, image_width - 1);
               int py = qBound(0, y >> 16, image_height - 1);
               *b = reinterpret_cast<const uint *>(data->texture.scanLine(py))[px];

               x += fdx;
               y += fdy;
               ++b;
            }
            func(target, buffer, l, coverage);
            target += l;
            length -= l;
         }
         ++spans;
      }
   } else {
      const qreal fdx = data->m11;
      const qreal fdy = data->m12;
      const qreal fdw = data->m13;
      while (count--) {
         void *t = data->rasterBuffer->scanLine(spans->y);

         uint *target = ((uint *)t) + spans->x;

         const qreal cx = spans->x + qreal(0.5);
         const qreal cy = spans->y + qreal(0.5);

         qreal x = data->m21 * cy + data->m11 * cx + data->dx;
         qreal y = data->m22 * cy + data->m12 * cx + data->dy;
         qreal w = data->m23 * cy + data->m13 * cx + data->m33;

         int length = spans->len;
         const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
         while (length) {
            int l = qMin(length, buffer_size);
            const uint *end = buffer + l;
            uint *b = buffer;
            while (b < end) {
               const qreal iw = w == 0 ? 1 : 1 / w;
               const qreal tx = x * iw;
               const qreal ty = y * iw;
               const int px = qBound(0, int(tx) - (tx < 0), image_width - 1);
               const int py = qBound(0, int(ty) - (ty < 0), image_height - 1);

               *b = reinterpret_cast<const uint *>(data->texture.scanLine(py))[px];
               x += fdx;
               y += fdy;
               w += fdw;

               ++b;
            }
            func(target, buffer, l, coverage);
            target += l;
            length -= l;
         }
         ++spans;
      }
   }
}

static void blend_transformed_rgb565(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   QPainter::CompositionMode mode = data->rasterBuffer->compositionMode;

   if (data->texture.format != QImage::Format_RGB16
      || (mode != QPainter::CompositionMode_SourceOver
         && mode != QPainter::CompositionMode_Source)) {
      blend_src_generic(count, spans, userData);
      return;
   }

   quint16 buffer[buffer_size];
   const int image_width = data->texture.width;
   const int image_height = data->texture.height;

   if (data->fast_matrix) {
      // The increment pr x in the scanline
      const int fdx = (int)(data->m11 * fixed_scale);
      const int fdy = (int)(data->m12 * fixed_scale);

      while (count--) {
         const quint8 coverage = (data->texture.const_alpha * spans->coverage) >> 8;
         const quint8 alpha = (coverage + 1) >> 3;
         const quint8 ialpha = 0x20 - alpha;
         if (alpha == 0) {
            ++spans;
            continue;
         }

         quint16 *dest = (quint16 *)data->rasterBuffer->scanLine(spans->y) + spans->x;
         const qreal cx = spans->x + qreal(0.5);
         const qreal cy = spans->y + qreal(0.5);
         int x = int((data->m21 * cy
                  + data->m11 * cx + data->dx) * fixed_scale);
         int y = int((data->m22 * cy
                  + data->m12 * cx + data->dy) * fixed_scale);
         int length = spans->len;

         while (length) {
            int l;
            quint16 *b;
            if (ialpha == 0) {
               l = length;
               b = dest;
            } else {
               l = qMin(length, buffer_size);
               b = buffer;
            }
            const quint16 *end = b + l;

            while (b < end) {
               const int px = qBound(0, x >> 16, image_width - 1);
               const int py = qBound(0, y >> 16, image_height - 1);

               *b = ((const quint16 *)data->texture.scanLine(py))[px];
               ++b;

               x += fdx;
               y += fdy;
            }

            if (ialpha != 0) {
               blend_sourceOver_rgb16_rgb16(dest, buffer, l, alpha, ialpha);
            }

            dest += l;
            length -= l;
         }
         ++spans;
      }
   } else {
      const qreal fdx = data->m11;
      const qreal fdy = data->m12;
      const qreal fdw = data->m13;

      while (count--) {
         const quint8 coverage = (data->texture.const_alpha * spans->coverage) >> 8;
         const quint8 alpha = (coverage + 1) >> 3;
         const quint8 ialpha = 0x20 - alpha;
         if (alpha == 0) {
            ++spans;
            continue;
         }

         quint16 *dest = (quint16 *)data->rasterBuffer->scanLine(spans->y) + spans->x;

         const qreal cx = spans->x + qreal(0.5);
         const qreal cy = spans->y + qreal(0.5);

         qreal x = data->m21 * cy + data->m11 * cx + data->dx;
         qreal y = data->m22 * cy + data->m12 * cx + data->dy;
         qreal w = data->m23 * cy + data->m13 * cx + data->m33;

         int length = spans->len;
         while (length) {
            int l;
            quint16 *b;
            if (ialpha == 0) {
               l = length;
               b = dest;
            } else {
               l = qMin(length, buffer_size);
               b = buffer;
            }
            const quint16 *end = b + l;

            while (b < end) {
               const qreal iw = w == 0 ? 1 : 1 / w;
               const qreal tx = x * iw;
               const qreal ty = y * iw;

               const int px = qBound(0, int(tx) - (tx < 0), image_width - 1);
               const int py = qBound(0, int(ty) - (ty < 0), image_height - 1);

               *b = ((const quint16 *)data->texture.scanLine(py))[px];
               ++b;

               x += fdx;
               y += fdy;
               w += fdw;
            }

            if (ialpha != 0) {
               blend_sourceOver_rgb16_rgb16(dest, buffer, l, alpha, ialpha);
            }

            dest += l;
            length -= l;
         }
         ++spans;
      }
   }
}

static void blend_transformed_tiled_argb(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   if (data->texture.format != QImage::Format_ARGB32_Premultiplied
      && data->texture.format != QImage::Format_RGB32) {
      blend_src_generic(count, spans, userData);
      return;
   }

   CompositionFunction func = functionForMode[data->rasterBuffer->compositionMode];
   uint buffer[buffer_size];

   int image_width = data->texture.width;
   int image_height = data->texture.height;
   const int scanline_offset = data->texture.bytesPerLine / 4;

   if (data->fast_matrix) {
      // The increment pr x in the scanline
      int fdx = (int)(data->m11 * fixed_scale);
      int fdy = (int)(data->m12 * fixed_scale);

      while (count--) {
         void *t = data->rasterBuffer->scanLine(spans->y);

         uint *target = ((uint *)t) + spans->x;
         const uint *image_bits = (const uint *)data->texture.imageData;

         const qreal cx = spans->x + qreal(0.5);
         const qreal cy = spans->y + qreal(0.5);

         int x = int((data->m21 * cy
                  + data->m11 * cx + data->dx) * fixed_scale);
         int y = int((data->m22 * cy
                  + data->m12 * cx + data->dy) * fixed_scale);

         const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
         int length = spans->len;
         while (length) {
            int l = qMin(length, buffer_size);
            const uint *end = buffer + l;
            uint *b = buffer;
            int px16 = x % (image_width << 16);
            int py16 = y % (image_height << 16);
            int px_delta = fdx % (image_width << 16);
            int py_delta = fdy % (image_height << 16);
            while (b < end) {
               if (px16 < 0) {
                  px16 += image_width << 16;
               }
               if (py16 < 0) {
                  py16 += image_height << 16;
               }
               int px = px16 >> 16;
               int py = py16 >> 16;
               int y_offset = py * scanline_offset;

               Q_ASSERT(px >= 0 && px < image_width);
               Q_ASSERT(py >= 0 && py < image_height);

               *b = image_bits[y_offset + px];
               x += fdx;
               y += fdy;
               px16 += px_delta;
               if (px16 >= image_width << 16) {
                  px16 -= image_width << 16;
               }
               py16 += py_delta;
               if (py16 >= image_height << 16) {
                  py16 -= image_height << 16;
               }
               ++b;
            }
            func(target, buffer, l, coverage);
            target += l;
            length -= l;
         }
         ++spans;
      }
   } else {
      const qreal fdx = data->m11;
      const qreal fdy = data->m12;
      const qreal fdw = data->m13;
      while (count--) {
         void *t = data->rasterBuffer->scanLine(spans->y);

         uint *target = ((uint *)t) + spans->x;
         const uint *image_bits = (const uint *)data->texture.imageData;

         const qreal cx = spans->x + qreal(0.5);
         const qreal cy = spans->y + qreal(0.5);

         qreal x = data->m21 * cy + data->m11 * cx + data->dx;
         qreal y = data->m22 * cy + data->m12 * cx + data->dy;
         qreal w = data->m23 * cy + data->m13 * cx + data->m33;

         const int coverage = (spans->coverage * data->texture.const_alpha) >> 8;
         int length = spans->len;
         while (length) {
            int l = qMin(length, buffer_size);
            const uint *end = buffer + l;
            uint *b = buffer;
            while (b < end) {
               const qreal iw = w == 0 ? 1 : 1 / w;
               const qreal tx = x * iw;
               const qreal ty = y * iw;
               int px = int(tx) - (tx < 0);
               int py = int(ty) - (ty < 0);

               px %= image_width;
               py %= image_height;
               if (px < 0) {
                  px += image_width;
               }
               if (py < 0) {
                  py += image_height;
               }
               int y_offset = py * scanline_offset;

               Q_ASSERT(px >= 0 && px < image_width);
               Q_ASSERT(py >= 0 && py < image_height);

               *b = image_bits[y_offset + px];
               x += fdx;
               y += fdy;
               w += fdw;
               //force increment to avoid /0
               if (!w) {
                  w += fdw;
               }
               ++b;
            }
            func(target, buffer, l, coverage);
            target += l;
            length -= l;
         }
         ++spans;
      }
   }
}

static void blend_transformed_tiled_rgb565(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   QPainter::CompositionMode mode = data->rasterBuffer->compositionMode;

   if (data->texture.format != QImage::Format_RGB16
      || (mode != QPainter::CompositionMode_SourceOver
         && mode != QPainter::CompositionMode_Source)) {
      blend_src_generic(count, spans, userData);
      return;
   }

   quint16 buffer[buffer_size];
   const int image_width = data->texture.width;
   const int image_height = data->texture.height;

   if (data->fast_matrix) {
      // The increment pr x in the scanline
      const int fdx = (int)(data->m11 * fixed_scale);
      const int fdy = (int)(data->m12 * fixed_scale);

      while (count--) {
         const quint8 coverage = (data->texture.const_alpha * spans->coverage) >> 8;
         const quint8 alpha = (coverage + 1) >> 3;
         const quint8 ialpha = 0x20 - alpha;
         if (alpha == 0) {
            ++spans;
            continue;
         }

         quint16 *dest = (quint16 *)data->rasterBuffer->scanLine(spans->y) + spans->x;
         const qreal cx = spans->x + qreal(0.5);
         const qreal cy = spans->y + qreal(0.5);
         int x = int((data->m21 * cy
                  + data->m11 * cx + data->dx) * fixed_scale);
         int y = int((data->m22 * cy
                  + data->m12 * cx + data->dy) * fixed_scale);
         int length = spans->len;

         while (length) {
            int l;
            quint16 *b;
            if (ialpha == 0) {
               l = length;
               b = dest;
            } else {
               l = qMin(length, buffer_size);
               b = buffer;
            }
            const quint16 *end = b + l;

            while (b < end) {
               int px = (x >> 16) % image_width;
               int py = (y >> 16) % image_height;

               if (px < 0) {
                  px += image_width;
               }
               if (py < 0) {
                  py += image_height;
               }

               *b = ((const quint16 *)data->texture.scanLine(py))[px];
               ++b;

               x += fdx;
               y += fdy;
            }

            if (ialpha != 0) {
               blend_sourceOver_rgb16_rgb16(dest, buffer, l, alpha, ialpha);
            }

            dest += l;
            length -= l;
         }
         ++spans;
      }
   } else {
      const qreal fdx = data->m11;
      const qreal fdy = data->m12;
      const qreal fdw = data->m13;

      while (count--) {
         const quint8 coverage = (data->texture.const_alpha * spans->coverage) >> 8;
         const quint8 alpha = (coverage + 1) >> 3;
         const quint8 ialpha = 0x20 - alpha;
         if (alpha == 0) {
            ++spans;
            continue;
         }

         quint16 *dest = (quint16 *)data->rasterBuffer->scanLine(spans->y) + spans->x;

         const qreal cx = spans->x + qreal(0.5);
         const qreal cy = spans->y + qreal(0.5);

         qreal x = data->m21 * cy + data->m11 * cx + data->dx;
         qreal y = data->m22 * cy + data->m12 * cx + data->dy;
         qreal w = data->m23 * cy + data->m13 * cx + data->m33;

         int length = spans->len;
         while (length) {
            int l;
            quint16 *b;
            if (ialpha == 0) {
               l = length;
               b = dest;
            } else {
               l = qMin(length, buffer_size);
               b = buffer;
            }
            const quint16 *end = b + l;

            while (b < end) {
               const qreal iw = w == 0 ? 1 : 1 / w;
               const qreal tx = x * iw;
               const qreal ty = y * iw;

               int px = int(tx) - (tx < 0);
               int py = int(ty) - (ty < 0);

               px %= image_width;
               py %= image_height;
               if (px < 0) {
                  px += image_width;
               }
               if (py < 0) {
                  py += image_height;
               }

               *b = ((const quint16 *)data->texture.scanLine(py))[px];
               ++b;

               x += fdx;
               y += fdy;
               w += fdw;
               // force increment to avoid /0
               if (!w) {
                  w += fdw;
               }
            }

            if (ialpha != 0) {
               blend_sourceOver_rgb16_rgb16(dest, buffer, l, alpha, ialpha);
            }

            dest += l;
            length -= l;
         }
         ++spans;
      }
   }
}


/* Image formats here are target formats */
static const ProcessSpans processTextureSpans[NBlendTypes][QImage::NImageFormats] = {
   // Untransformed
   {
      0, // Invalid
      blend_untransformed_generic, // Mono
      blend_untransformed_generic, // MonoLsb
      blend_untransformed_generic, // Indexed8
      blend_untransformed_generic, // RGB32
      blend_untransformed_generic, // ARGB32
      blend_untransformed_argb, // ARGB32_Premultiplied
      blend_untransformed_rgb565,
      blend_untransformed_generic,
      blend_untransformed_generic,
      blend_untransformed_generic,
      blend_untransformed_generic,
      blend_untransformed_generic,
      blend_untransformed_generic,
      blend_untransformed_generic,
      blend_untransformed_generic,
      blend_untransformed_generic,
      blend_untransformed_generic,
      blend_untransformed_generic,
      blend_untransformed_generic_rgb64,
      blend_untransformed_generic_rgb64,
      blend_untransformed_generic_rgb64,
      blend_untransformed_generic_rgb64,
      blend_untransformed_generic,
      blend_untransformed_generic,
   },
   // Tiled
   {
      0, // Invalid
      blend_tiled_generic, // Mono
      blend_tiled_generic, // MonoLsb
      blend_tiled_generic, // Indexed8
      blend_tiled_generic, // RGB32
      blend_tiled_generic, // ARGB32
      blend_tiled_argb, // ARGB32_Premultiplied
      blend_tiled_rgb565,
      blend_tiled_generic,
      blend_tiled_generic,
      blend_tiled_generic,
      blend_tiled_generic,
      blend_tiled_generic,
      blend_tiled_generic,
      blend_tiled_generic,
      blend_tiled_generic,
      blend_tiled_generic,
      blend_tiled_generic,
      blend_tiled_generic,
      blend_tiled_generic_rgb64,
      blend_tiled_generic_rgb64,
      blend_tiled_generic_rgb64,
      blend_tiled_generic_rgb64,
      blend_tiled_generic,
      blend_tiled_generic,
   },
   // Transformed
   {
      0, // Invalid
      blend_src_generic, // Mono
      blend_src_generic, // MonoLsb
      blend_src_generic, // Indexed8
      blend_src_generic, // RGB32
      blend_src_generic, // ARGB32
      blend_transformed_argb, // ARGB32_Premultiplied
      blend_transformed_rgb565,
      blend_src_generic, // ARGB8565_Premultiplied
      blend_src_generic, // RGB666
      blend_src_generic, // ARGB6666_Premultiplied
      blend_src_generic, // RGB555
      blend_src_generic, // ARGB8555_Premultiplied
      blend_src_generic, // RGB888
      blend_src_generic, // RGB444
      blend_src_generic, // ARGB4444_Premultiplied
      blend_src_generic, // RGBX8888
      blend_src_generic, // RGBA8888
      blend_src_generic, // RGBA8888_Premultiplied
      blend_src_generic_rgb64,
      blend_src_generic_rgb64,
      blend_src_generic_rgb64,
      blend_src_generic_rgb64,
      blend_src_generic,
      blend_src_generic,
   },
   // TransformedTiled
   {
      0,
      blend_src_generic, // Mono
      blend_src_generic, // MonoLsb
      blend_src_generic, // Indexed8
      blend_src_generic, // RGB32
      blend_src_generic, // ARGB32
      blend_transformed_tiled_argb, // ARGB32_Premultiplied
      blend_transformed_tiled_rgb565,
      blend_src_generic, // ARGB8565_Premultiplied
      blend_src_generic, // RGB666
      blend_src_generic, // ARGB6666_Premultiplied
      blend_src_generic, // RGB555
      blend_src_generic, // ARGB8555_Premultiplied
      blend_src_generic, // RGB888
      blend_src_generic, // RGB444
      blend_src_generic, // ARGB4444_Premultiplied
      blend_src_generic, // RGBX8888
      blend_src_generic, // RGBA8888
      blend_src_generic, // RGBA8888_Premultiplied
      blend_src_generic_rgb64,
      blend_src_generic_rgb64,
      blend_src_generic_rgb64,
      blend_src_generic_rgb64,
      blend_src_generic,
      blend_src_generic,
   },
   // Bilinear
   {
      0,
      blend_src_generic, // Mono
      blend_src_generic, // MonoLsb
      blend_src_generic, // Indexed8
      blend_src_generic, // RGB32
      blend_src_generic, // ARGB32
      blend_src_generic, // ARGB32_Premultiplied
      blend_transformed_bilinear_rgb565,
      blend_src_generic, // ARGB8565_Premultiplied
      blend_src_generic, // RGB666
      blend_src_generic, // ARGB6666_Premultiplied
      blend_src_generic, // RGB555
      blend_src_generic, // ARGB8555_Premultiplied
      blend_src_generic, // RGB888
      blend_src_generic, // RGB444
      blend_src_generic, // ARGB4444_Premultiplied
      blend_src_generic, // RGBX8888
      blend_src_generic, // RGBA8888
      blend_src_generic, // RGBA8888_Premultiplied
      blend_src_generic_rgb64,
      blend_src_generic_rgb64,
      blend_src_generic_rgb64,
      blend_src_generic_rgb64,
      blend_src_generic,
      blend_src_generic,
   },
   // BilinearTiled
   {
      0,
      blend_src_generic, // Mono
      blend_src_generic, // MonoLsb
      blend_src_generic, // Indexed8
      blend_src_generic, // RGB32
      blend_src_generic, // ARGB32
      blend_src_generic, // ARGB32_Premultiplied
      blend_src_generic, // RGB16
      blend_src_generic, // ARGB8565_Premultiplied
      blend_src_generic, // RGB666
      blend_src_generic, // ARGB6666_Premultiplied
      blend_src_generic, // RGB555
      blend_src_generic, // ARGB8555_Premultiplied
      blend_src_generic, // RGB888
      blend_src_generic, // RGB444
      blend_src_generic, // ARGB4444_Premultiplied
      blend_src_generic, // RGBX8888
      blend_src_generic, // RGBA8888
      blend_src_generic, // RGBA8888_Premultiplied
      blend_src_generic_rgb64, // BGR30
      blend_src_generic_rgb64, // A2BGR30_Premultiplied
      blend_src_generic_rgb64, // RGB30
      blend_src_generic_rgb64, // A2RGB30_Premultiplied
      blend_src_generic, // Alpha8
      blend_src_generic, // Grayscale8
   }
};

void qBlendTexture(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);
   ProcessSpans proc = processTextureSpans[getBlendType(data)][data->rasterBuffer->format];
   proc(count, spans, userData);
}

template <class DST>
static inline void qt_bitmapblit_template(QRasterBuffer *rasterBuffer,
   int x, int y, DST color,
   const uchar *map,
   int mapWidth, int mapHeight, int mapStride)
{
   DST *dest = reinterpret_cast<DST *>(rasterBuffer->scanLine(y)) + x;
   const int destStride = rasterBuffer->bytesPerLine() / sizeof(DST);

   if (mapWidth > 8) {
      while (mapHeight--) {
         int x0 = 0;
         int n = 0;
         for (int x = 0; x < mapWidth; x += 8) {
            uchar s = map[x >> 3];
            for (int i = 0; i < 8; ++i) {
               if (s & 0x80) {
                  ++n;
               } else {
                  if (n) {
                     qt_memfill(dest + x0, color, n);
                     x0 += n + 1;
                     n = 0;
                  } else {
                     ++x0;
                  }
                  if (!s) {
                     x0 += 8 - 1 - i;
                     break;
                  }
               }
               s <<= 1;
            }
         }
         if (n) {
            qt_memfill(dest + x0, color, n);
         }
         dest += destStride;
         map += mapStride;
      }
   } else {
      while (mapHeight--) {
         int x0 = 0;
         int n = 0;
         for (uchar s = *map; s; s <<= 1) {
            if (s & 0x80) {
               ++n;
            } else if (n) {
               qt_memfill(dest + x0, color, n);
               x0 += n + 1;
               n = 0;
            } else {
               ++x0;
            }
         }
         if (n) {
            qt_memfill(dest + x0, color, n);
         }
         dest += destStride;
         map += mapStride;
      }
   }
}

static void qt_gradient_argb32(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);

   bool isVerticalGradient =
      data->txop <= QTransform::TxScale &&
      data->type == QSpanData::LinearGradient &&
      data->gradient.linear.end.x == data->gradient.linear.origin.x;

   if (isVerticalGradient) {
      LinearGradientValues linear;
      getLinearGradientValues(&linear, data);

      CompositionFunctionSolid funcSolid =
         functionForModeSolid[data->rasterBuffer->compositionMode];

      /*
          The logic for vertical gradient calculations is a mathematically
          reduced copy of that in fetchLinearGradient() - which is basically:

              qreal ry = data->m22 * (y + 0.5) + data->dy;
              qreal t = linear.dy*ry + linear.off;
              t *= (GRADIENT_STOPTABLE_SIZE - 1);
              quint32 color =
                  qt_gradient_pixel_fixed(&data->gradient,
                                          int(t * FIXPT_SIZE));

          This has then been converted to fixed point to improve performance.
       */
      const int gss = GRADIENT_STOPTABLE_SIZE - 1;
      int yinc = int((linear.dy * data->m22 * gss) * FIXPT_SIZE);
      int off = int((((linear.dy * (data->m22 * qreal(0.5) + data->dy) + linear.off) * gss) * FIXPT_SIZE));

      while (count--) {
         int y = spans->y;
         int x = spans->x;

         quint32 *dst = (quint32 *)(data->rasterBuffer->scanLine(y)) + x;
         quint32 color =
            qt_gradient_pixel_fixed(&data->gradient, yinc * y + off);

         funcSolid(dst, spans->len, color, spans->coverage);
         ++spans;
      }

   } else {
      blend_src_generic(count, spans, userData);
   }
}

static void qt_gradient_quint16(int count, const QSpan *spans, void *userData)
{
   QSpanData *data = reinterpret_cast<QSpanData *>(userData);

   bool isVerticalGradient =
      data->txop <= QTransform::TxScale &&
      data->type == QSpanData::LinearGradient &&
      data->gradient.linear.end.x == data->gradient.linear.origin.x;

   if (isVerticalGradient) {

      LinearGradientValues linear;
      getLinearGradientValues(&linear, data);

      /*
          The logic for vertical gradient calculations is a mathematically
          reduced copy of that in fetchLinearGradient() - which is basically:

              qreal ry = data->m22 * (y + 0.5) + data->dy;
              qreal t = linear.dy*ry + linear.off;
              t *= (GRADIENT_STOPTABLE_SIZE - 1);
              quint32 color =
                  qt_gradient_pixel_fixed(&data->gradient,
                                          int(t * FIXPT_SIZE));

          This has then been converted to fixed point to improve performance.
       */
      const int gss = GRADIENT_STOPTABLE_SIZE - 1;
      int yinc = int((linear.dy * data->m22 * gss) * FIXPT_SIZE);
      int off = int((((linear.dy * (data->m22 * qreal(0.5) + data->dy) + linear.off) * gss) * FIXPT_SIZE));

      // Save the fillData since we overwrite it when setting solid.color.
      QGradientData gradient = data->gradient;
      while (count--) {
         int y = spans->y;

         data->solid.color = QRgba64::fromArgb32(qt_gradient_pixel_fixed(&gradient, yinc * y + off));
         blend_color_rgb16(1, spans, userData);
         ++spans;
      }
      data->gradient = gradient;

   } else {
      blend_src_generic(count, spans, userData);
   }
}

inline static void qt_bitmapblit_argb32(QRasterBuffer *rasterBuffer,
   int x, int y, const QRgba64 &color,
   const uchar *map,
   int mapWidth, int mapHeight, int mapStride)
{
   qt_bitmapblit_template<quint32>(rasterBuffer, x,  y, color.toArgb32(),
      map, mapWidth, mapHeight, mapStride);
}

inline static void qt_bitmapblit_rgba8888(QRasterBuffer *rasterBuffer,
   int x, int y, const QRgba64 &color,
   const uchar *map,
   int mapWidth, int mapHeight, int mapStride)
{
   qt_bitmapblit_template<quint32>(rasterBuffer, x, y, ARGB2RGBA(color.toArgb32()),
      map, mapWidth, mapHeight, mapStride);
}

template<QtPixelOrder PixelOrder>
inline static void qt_bitmapblit_rgb30(QRasterBuffer *rasterBuffer,
   int x, int y, const QRgba64 &color,
   const uchar *map,
   int mapWidth, int mapHeight, int mapStride)
{
   qt_bitmapblit_template<quint32>(rasterBuffer, x, y, qConvertRgb64ToRgb30<PixelOrder>(color),
      map, mapWidth, mapHeight, mapStride);
}

inline static void qt_bitmapblit_quint16(QRasterBuffer *rasterBuffer,
   int x, int y, const QRgba64 &color,
   const uchar *map,
   int mapWidth, int mapHeight, int mapStride)
{
   qt_bitmapblit_template<quint16>(rasterBuffer, x,  y, color.toRgb16(),
      map, mapWidth, mapHeight, mapStride);
}

static void qt_alphamapblit_quint16(QRasterBuffer *rasterBuffer,
   int x, int y, const QRgba64 &color,
   const uchar *map,
   int mapWidth, int mapHeight, int mapStride,
   const QClipData *)
{
   const quint16 c = color.toRgb16();
   quint16 *dest = reinterpret_cast<quint16 *>(rasterBuffer->scanLine(y)) + x;
   const int destStride = rasterBuffer->bytesPerLine() / sizeof(quint16);

   while (mapHeight--) {
      for (int i = 0; i < mapWidth; ++i) {
         const int coverage = map[i];

         if (coverage == 0) {
            // nothing
         } else if (coverage == 255) {
            dest[i] = c;
         } else {
            int ialpha = 255 - coverage;
            dest[i] = BYTE_MUL_RGB16(c, coverage)
               + BYTE_MUL_RGB16(dest[i], ialpha);
         }
      }
      dest += destStride;
      map += mapStride;
   }
}

static inline void rgbBlendPixel(quint32 *dst, int coverage, int sr, int sg, int sb, const uchar *gamma, const uchar *invgamma)
{
   // Do a gray alphablend...
   int da = qAlpha(*dst);
   int dr = qRed(*dst);
   int dg = qGreen(*dst);
   int db = qBlue(*dst);

   if (da != 255
   ) {

      int a = qGray(coverage);
      sr = qt_div_255(invgamma[sr] * a);
      sg = qt_div_255(invgamma[sg] * a);
      sb = qt_div_255(invgamma[sb] * a);

      int ia = 255 - a;
      dr = qt_div_255(dr * ia);
      dg = qt_div_255(dg * ia);
      db = qt_div_255(db * ia);

      *dst = ((a + qt_div_255((255 - a) * da)) << 24)
         |  ((sr + dr) << 16)
         |  ((sg + dg) << 8)
         |  ((sb + db));
      return;
   }

   int mr = qRed(coverage);
   int mg = qGreen(coverage);
   int mb = qBlue(coverage);

   dr = gamma[dr];
   dg = gamma[dg];
   db = gamma[db];

   int nr = qt_div_255(sr * mr + dr * (255 - mr));
   int ng = qt_div_255(sg * mg + dg * (255 - mg));
   int nb = qt_div_255(sb * mb + db * (255 - mb));

   nr = invgamma[nr];
   ng = invgamma[ng];
   nb = invgamma[nb];

   *dst = qRgb(nr, ng, nb);
}

#if defined(Q_OS_WIN)
Q_GUI_EXPORT bool qt_needs_a8_gamma_correction = false;

static inline void grayBlendPixel(quint32 *dst, int coverage, int sr, int sg, int sb, const uint *gamma, const uchar *invgamma)
{
   // Do a gammacorrected gray alphablend...
   int dr = qRed(*dst);
   int dg = qGreen(*dst);
   int db = qBlue(*dst);

   dr = gamma[dr];
   dg = gamma[dg];
   db = gamma[db];

   int alpha = coverage;
   int ialpha = 255 - alpha;
   int nr = qt_div_255(sr * alpha + dr * ialpha);
   int ng = qt_div_255(sg * alpha + dg * ialpha);
   int nb = qt_div_255(sb * alpha + db * ialpha);

   nr = invgamma[nr];
   ng = invgamma[ng];
   nb = invgamma[nb];

   *dst = qRgb(nr, ng, nb);
}
#endif

static void qt_alphamapblit_uint32(QRasterBuffer *rasterBuffer,
   int x, int y, quint32 color,
   const uchar *map,
   int mapWidth, int mapHeight, int mapStride,
   const QClipData *clip)
{
   const quint32 c = color;
   const int destStride = rasterBuffer->bytesPerLine() / sizeof(quint32);

#if defined(Q_OS_WIN)
   const QDrawHelperGammaTables *tables = QApplicationPrivate::instance()->gammaTables();

   if (!tables) {
      return;
   }

   const uint *gamma = tables->qt_pow_gamma;
   const uchar *invgamma = tables->qt_pow_invgamma;

   int sr = gamma[qRed(color)];
   int sg = gamma[qGreen(color)];
   int sb = gamma[qBlue(color)];

   bool opaque_src = (qAlpha(color) == 255);
   bool doGrayBlendPixel = opaque_src && qt_needs_a8_gamma_correction;
#endif

   if (!clip) {
      quint32 *dest = reinterpret_cast<quint32 *>(rasterBuffer->scanLine(y)) + x;
      while (mapHeight--) {
         for (int i = 0; i < mapWidth; ++i) {
            const int coverage = map[i];

            if (coverage == 0) {
               // nothing
            } else if (coverage == 255) {
               dest[i] = c;
            } else {

#if defined(Q_OS_WIN)
               if (QSysInfo::WindowsVersion >= QSysInfo::WV_XP && doGrayBlendPixel
                  && qAlpha(dest[i]) == 255) {
                  grayBlendPixel(dest + i, coverage, sr, sg, sb, gamma, invgamma);
               } else
#endif
               {
                  int ialpha = 255 - coverage;
                  dest[i] = INTERPOLATE_PIXEL_255(c, coverage, dest[i], ialpha);
               }
            }
         }
         dest += destStride;
         map += mapStride;
      }
   } else {
      int bottom = qMin(y + mapHeight, rasterBuffer->height());

      int top = qMax(y, 0);
      map += (top - y) * mapStride;

      const_cast<QClipData *>(clip)->initialize();
      for (int yp = top; yp < bottom; ++yp) {
         const QClipData::ClipLine &line = clip->m_clipLines[yp];

         quint32 *dest = reinterpret_cast<quint32 *>(rasterBuffer->scanLine(yp));

         for (int i = 0; i < line.count; ++i) {
            const QSpan &clip = line.spans[i];

            int start = qMax<int>(x, clip.x);
            int end = qMin<int>(x + mapWidth, clip.x + clip.len);

            for (int xp = start; xp < end; ++xp) {
               const int coverage = map[xp - x];

               if (coverage == 0) {
                  // nothing

               } else if (coverage == 255) {
                  dest[xp] = c;

               } else {

#if defined(Q_OS_WIN)
                  if (QSysInfo::WindowsVersion >= QSysInfo::WV_XP && doGrayBlendPixel
                     && qAlpha(dest[xp]) == 255) {
                     grayBlendPixel(dest + xp, coverage, sr, sg, sb, gamma, invgamma);
                  } else
#endif
                  {
                     int ialpha = 255 - coverage;
                     dest[xp] = INTERPOLATE_PIXEL_255(c, coverage, dest[xp], ialpha);
                  }
               }

            } // for (i -> line.count)
         } // for (yp -> bottom)
         map += mapStride;
      }
   }
}


static void qt_alphamapblit_argb32(QRasterBuffer *rasterBuffer,
   int x, int y, const QRgba64 &color,
   const uchar *map,
   int mapWidth, int mapHeight, int mapStride,
   const QClipData *clip)
{
   qt_alphamapblit_uint32(rasterBuffer, x, y, color.toArgb32(), map, mapWidth, mapHeight, mapStride, clip);
}

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
static void qt_alphamapblit_rgba8888(QRasterBuffer *rasterBuffer,
   int x, int y, const QRgba64 &color,
   const uchar *map,
   int mapWidth, int mapHeight, int mapStride,
   const QClipData *clip)
{
   qt_alphamapblit_uint32(rasterBuffer, x, y, ARGB2RGBA(color.toArgb32()), map, mapWidth, mapHeight, mapStride, clip);
}
#endif

static void qt_alphargbblit_argb32(QRasterBuffer *rasterBuffer,
   int x, int y, const QRgba64 &color,
   const uint *src, int mapWidth, int mapHeight, int srcStride,
   const QClipData *clip)
{
   const quint32 c = color.toArgb32();

   int sr = qRed(c);
   int sg = qGreen(c);
   int sb = qBlue(c);
   int sa = qAlpha(c);

   const QDrawHelperGammaTables *tables = QApplicationPrivate::instance()->gammaTables();
   if (!tables) {
      return;
   }

   const uchar *gamma = tables->qt_pow_rgb_gamma;
   const uchar *invgamma = tables->qt_pow_rgb_invgamma;

   sr = gamma[sr];
   sg = gamma[sg];
   sb = gamma[sb];

   if (sa == 0) {
      return;
   }

   if (!clip) {
      quint32 *dst = reinterpret_cast<quint32 *>(rasterBuffer->scanLine(y)) + x;
      const int destStride = rasterBuffer->bytesPerLine() / sizeof(quint32);
      while (mapHeight--) {
         for (int i = 0; i < mapWidth; ++i) {
            const uint coverage = src[i];
            if (coverage == 0xffffffff) {
               dst[i] = c;
            } else if (coverage != 0xff000000) {
               rgbBlendPixel(dst + i, coverage, sr, sg, sb, gamma, invgamma);
            }
         }

         dst += destStride;
         src += srcStride;
      }
   } else {
      int bottom = qMin(y + mapHeight, rasterBuffer->height());

      int top = qMax(y, 0);
      src += (top - y) * srcStride;

      const_cast<QClipData *>(clip)->initialize();
      for (int yp = top; yp < bottom; ++yp) {
         const QClipData::ClipLine &line = clip->m_clipLines[yp];

         quint32 *dst = reinterpret_cast<quint32 *>(rasterBuffer->scanLine(yp));

         for (int i = 0; i < line.count; ++i) {
            const QSpan &clip = line.spans[i];

            int start = qMax<int>(x, clip.x);
            int end = qMin<int>(x + mapWidth, clip.x + clip.len);

            for (int xp = start; xp < end; ++xp) {
               const uint coverage = src[xp - x];
               if (coverage == 0xffffffff) {
                  dst[xp] = c;
               } else if (coverage != 0xff000000) {
                  rgbBlendPixel(dst + xp, coverage, sr, sg, sb, gamma, invgamma);
               }
            }
         } // for (i -> line.count)
         src += srcStride;
      } // for (yp -> bottom)

   }
}

static void qt_rectfill_argb32(QRasterBuffer *rasterBuffer,
   int x, int y, int width, int height,
   const QRgba64 &color)
{
   qt_rectfill<quint32>(reinterpret_cast<quint32 *>(rasterBuffer->buffer()),
      color.toArgb32(), x, y, width, height, rasterBuffer->bytesPerLine());
}

static void qt_rectfill_quint16(QRasterBuffer *rasterBuffer,
   int x, int y, int width, int height,
   const QRgba64 &color)
{
   qt_rectfill<quint16>(reinterpret_cast<quint16 *>(rasterBuffer->buffer()),
      color.toRgb16(), x, y, width, height, rasterBuffer->bytesPerLine());
}

static void qt_rectfill_nonpremul_argb32(QRasterBuffer *rasterBuffer,
   int x, int y, int width, int height,
   const QRgba64 &color)
{
   qt_rectfill<quint32>(reinterpret_cast<quint32 *>(rasterBuffer->buffer()),
      color.unpremultiplied().toArgb32(), x, y, width, height, rasterBuffer->bytesPerLine());
}

static void qt_rectfill_rgba(QRasterBuffer *rasterBuffer,
   int x, int y, int width, int height,
   const QRgba64 &color)
{
   qt_rectfill<quint32>(reinterpret_cast<quint32 *>(rasterBuffer->buffer()),
      ARGB2RGBA(color.toArgb32()), x, y, width, height, rasterBuffer->bytesPerLine());
}

static void qt_rectfill_nonpremul_rgba(QRasterBuffer *rasterBuffer,
   int x, int y, int width, int height,
   const QRgba64 &color)
{
   qt_rectfill<quint32>(reinterpret_cast<quint32 *>(rasterBuffer->buffer()),
      ARGB2RGBA(color.unpremultiplied().toArgb32()), x, y, width, height, rasterBuffer->bytesPerLine());
}

template<QtPixelOrder PixelOrder>
static void qt_rectfill_rgb30(QRasterBuffer *rasterBuffer,
   int x, int y, int width, int height,
   const QRgba64 &color)
{
   qt_rectfill<quint32>(reinterpret_cast<quint32 *>(rasterBuffer->buffer()),
      qConvertRgb64ToRgb30<PixelOrder>(color), x, y, width, height, rasterBuffer->bytesPerLine());
}

static void qt_rectfill_alpha(QRasterBuffer *rasterBuffer,
   int x, int y, int width, int height,
   const QRgba64 &color)
{
   qt_rectfill<quint8>(reinterpret_cast<quint8 *>(rasterBuffer->buffer()),
      color.alpha() >> 8, x, y, width, height, rasterBuffer->bytesPerLine());
}

static void qt_rectfill_gray(QRasterBuffer *rasterBuffer,
   int x, int y, int width, int height,
   const QRgba64 &color)
{
   qt_rectfill<quint8>(reinterpret_cast<quint8 *>(rasterBuffer->buffer()),
      qGray(color.toArgb32()), x, y, width, height, rasterBuffer->bytesPerLine());
}

// Map table for destination image format.
// Contains function pointers r blends of various types onto the destination

DrawHelper qDrawHelper[QImage::NImageFormats] = {
   // Format_Invalid,
   { 0, 0, 0, 0, 0, 0 },
   // Format_Mono,
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_MonoLSB,
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_Indexed8,
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_RGB32,
   {
      blend_color_argb,
      qt_gradient_argb32,
      qt_bitmapblit_argb32,
      qt_alphamapblit_argb32,
      qt_alphargbblit_argb32,
      qt_rectfill_argb32
   },
   // Format_ARGB32,
   {
      blend_color_generic,
      qt_gradient_argb32,
      qt_bitmapblit_argb32,
      qt_alphamapblit_argb32,
      qt_alphargbblit_argb32,
      qt_rectfill_nonpremul_argb32
   },
   // Format_ARGB32_Premultiplied
   {
      blend_color_argb,
      qt_gradient_argb32,
      qt_bitmapblit_argb32,
      qt_alphamapblit_argb32,
      qt_alphargbblit_argb32,
      qt_rectfill_argb32
   },
   // Format_RGB16
   {
      blend_color_rgb16,
      qt_gradient_quint16,
      qt_bitmapblit_quint16,
      qt_alphamapblit_quint16,
      0,
      qt_rectfill_quint16
   },
   // Format_ARGB8565_Premultiplied
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_RGB666
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_ARGB6666_Premultiplied
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_RGB555
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_ARGB8555_Premultiplied
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_RGB888
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_RGB444
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_ARGB4444_Premultiplied
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0, 0
   },
   // Format_RGBX8888
   {
      blend_color_generic,
      blend_src_generic,
      qt_bitmapblit_rgba8888,
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
      qt_alphamapblit_rgba8888,
#else
      0,
#endif
      0,
      qt_rectfill_rgba
   },
   // Format_RGBA8888
   {
      blend_color_generic,
      blend_src_generic,
      qt_bitmapblit_rgba8888,
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
      qt_alphamapblit_rgba8888,
#else
      0,
#endif
      0,
      qt_rectfill_nonpremul_rgba
   },
   // Format_RGB8888_Premultiplied
   {
      blend_color_generic,
      blend_src_generic,
      qt_bitmapblit_rgba8888,
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
      qt_alphamapblit_rgba8888,
#else
      0,
#endif
      0,
      qt_rectfill_rgba
   },
   // Format_BGR30
   {
      blend_color_generic_rgb64,
      blend_src_generic_rgb64,
      qt_bitmapblit_rgb30<PixelOrderBGR>,
      0,
      0,
      qt_rectfill_rgb30<PixelOrderBGR>
   },
   // Format_A2BGR30_Premultiplied
   {
      blend_color_generic_rgb64,
      blend_src_generic_rgb64,
      qt_bitmapblit_rgb30<PixelOrderBGR>,
      0,
      0,
      qt_rectfill_rgb30<PixelOrderBGR>
   },
   // Format_RGB30
   {
      blend_color_generic_rgb64,
      blend_src_generic_rgb64,
      qt_bitmapblit_rgb30<PixelOrderRGB>,
      0,
      0,
      qt_rectfill_rgb30<PixelOrderRGB>
   },
   // Format_A2RGB30_Premultiplied
   {
      blend_color_generic_rgb64,
      blend_src_generic_rgb64,
      qt_bitmapblit_rgb30<PixelOrderRGB>,
      0,
      0,
      qt_rectfill_rgb30<PixelOrderRGB>
   },
   // Format_Alpha8
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0,
      qt_rectfill_alpha
   },
   // Format_Grayscale8
   {
      blend_color_generic,
      blend_src_generic,
      0, 0, 0,
      qt_rectfill_gray
   },
};

#if defined(Q_CC_MSVC) && !defined(_MIPS_)
template <class T>
inline void qt_memfill_template(T *dest, T color, int count)
{
   while (count--) {
      *dest++ = color;
   }
}

#else

template <class T>
inline void qt_memfill_template(T *dest, T color, int count)
{
   int n = (count + 7) / 8;
   switch (count & 0x07) {
      case 0:
         do {
            *dest++ = color;
            [[fallthrough]];

         case 7:
            *dest++ = color;
            [[fallthrough]];

         case 6:
            *dest++ = color;
            [[fallthrough]];

         case 5:
            *dest++ = color;
            [[fallthrough]];

         case 4:
            *dest++ = color;
            [[fallthrough]];

         case 3:
            *dest++ = color;
            [[fallthrough]];

         case 2:
            *dest++ = color;
            [[fallthrough]];
         case 1:
            *dest++ = color;

         } while (--n > 0);
   }
}

template <>
inline void qt_memfill_template(quint16 *dest, quint16 value, int count)
{
   if (count < 3) {
      switch (count) {
         case 2:
            *dest++ = value;
         case 1:
            *dest = value;
      }
      return;
   }

   const int align = (quintptr)(dest) & 0x3;
   switch (align) {
      case 2:
         *dest++ = value;
         --count;
   }

   const quint32 value32 = (value << 16) | value;
   qt_memfill(reinterpret_cast<quint32 *>(dest), value32, count / 2);
   if (count & 0x1) {
      dest[count - 1] = value;
   }
}
#endif

void qt_memfill64(quint64 *dest, quint64 color, int count)
{
   qt_memfill_template<quint64>(dest, color, count);
}

#if !defined(__SSE2__)
void qt_memfill16(quint16 *dest, quint16 color, int count)
{
   qt_memfill_template<quint16>(dest, color, count);
}
#endif

#if !defined(__SSE2__) && !defined(__ARM_NEON__)
#  ifdef QT_COMPILER_SUPPORTS_MIPS_DSP
extern "C" void qt_memfill32_asm_mips_dsp(quint32 *, quint32, int);
#  endif

void qt_memfill32(quint32 *dest, quint32 color, int count)
{
#  ifdef QT_COMPILER_SUPPORTS_MIPS_DSP
   qt_memfill32_asm_mips_dsp(dest, color, count);
#  else
   qt_memfill_template<quint32>(dest, color, count);
#  endif
}
#endif

#ifdef __SSE4_1__

template<QtPixelOrder> const uint *convertA2RGB30PMFromARGB32PM_sse4(uint *buffer, const uint *src, int count,
   const QPixelLayout *, const QRgb *);
#endif

QDrawHelperFunctions::QDrawHelperFunctions()
{
   memset(blendFunctions,     0, sizeof(blendFunctions));
   memset(scaleFunctions,     0, sizeof(scaleFunctions));
   memset(transformFunctions, 0, sizeof(transformFunctions));
   memset(memRotateFunctions, 0, sizeof(memRotateFunctions));
   memset(drawHelper,         0, sizeof(drawHelper));

   // copy file local static to data member
   memcpy(drawHelper, qDrawHelper, sizeof(drawHelper));

   // set up basic blend function tables
   initBlendFunctions();
   initMemRotate();

#ifdef __SSE2__
   drawHelper[QImage::Format_RGB32].bitmapBlit = qt_bitmapblit32_sse2;
   drawHelper[QImage::Format_ARGB32].bitmapBlit = qt_bitmapblit32_sse2;
   drawHelper[QImage::Format_ARGB32_Premultiplied].bitmapBlit = qt_bitmapblit32_sse2;
   drawHelper[QImage::Format_RGB16].bitmapBlit = qt_bitmapblit16_sse2;
   drawHelper[QImage::Format_RGBX8888].bitmapBlit = qt_bitmapblit8888_sse2;
   drawHelper[QImage::Format_RGBA8888].bitmapBlit = qt_bitmapblit8888_sse2;
   drawHelper[QImage::Format_RGBA8888_Premultiplied].bitmapBlit = qt_bitmapblit8888_sse2;

   extern void qt_scale_image_argb32_on_argb32_sse2(uchar * destPixels, int dbpl,
      const uchar * srcPixels, int sbpl, int srch, const QRectF & targetRect,
      const QRectF & sourceRect, const QRect & clip, int const_alpha);

   scaleFunctions[QImage::Format_ARGB32_Premultiplied][QImage::Format_ARGB32_Premultiplied] = qt_scale_image_argb32_on_argb32_sse2;
   scaleFunctions[QImage::Format_RGB32][QImage::Format_ARGB32_Premultiplied] = qt_scale_image_argb32_on_argb32_sse2;
   scaleFunctions[QImage::Format_RGBA8888_Premultiplied][QImage::Format_RGBA8888_Premultiplied] = qt_scale_image_argb32_on_argb32_sse2;
   scaleFunctions[QImage::Format_RGBX8888][QImage::Format_RGBA8888_Premultiplied] = qt_scale_image_argb32_on_argb32_sse2;

   extern void qt_blend_rgb32_on_rgb32_sse2(uchar * destPixels, int dbpl,
      const uchar * srcPixels, int sbpl, int w, int h, int const_alpha);

   extern void qt_blend_argb32_on_argb32_sse2(uchar * destPixels, int dbpl,
      const uchar * srcPixels, int sbpl, int w, int h, int const_alpha);

   blendFunctions[QImage::Format_RGB32][QImage::Format_RGB32] = qt_blend_rgb32_on_rgb32_sse2;
   blendFunctions[QImage::Format_ARGB32_Premultiplied][QImage::Format_RGB32] = qt_blend_rgb32_on_rgb32_sse2;
   blendFunctions[QImage::Format_RGB32][QImage::Format_ARGB32_Premultiplied] = qt_blend_argb32_on_argb32_sse2;
   blendFunctions[QImage::Format_ARGB32_Premultiplied][QImage::Format_ARGB32_Premultiplied] = qt_blend_argb32_on_argb32_sse2;
   blendFunctions[QImage::Format_RGBX8888][QImage::Format_RGBX8888] = qt_blend_rgb32_on_rgb32_sse2;
   blendFunctions[QImage::Format_RGBA8888_Premultiplied][QImage::Format_RGBX8888] = qt_blend_rgb32_on_rgb32_sse2;
   blendFunctions[QImage::Format_RGBX8888][QImage::Format_RGBA8888_Premultiplied] = qt_blend_argb32_on_argb32_sse2;
   blendFunctions[QImage::Format_RGBA8888_Premultiplied][QImage::Format_RGBA8888_Premultiplied] = qt_blend_argb32_on_argb32_sse2;

   extern const uint *qt_fetch_radial_gradient_sse2(uint * buffer, const Operator * op, const QSpanData * data,
      int y, int x, int length);

   qt_fetch_radial_gradient = qt_fetch_radial_gradient_sse2;

#ifdef QT_COMPILER_SUPPORTS_SSSE3
   if (qCpuHasFeature(SSSE3)) {
      extern void qt_blend_argb32_on_argb32_ssse3(uchar * destPixels, int dbpl, const uchar * srcPixels, int sbpl,
         int w, int h, int const_alpha);

      blendFunctions[QImage::Format_RGB32][QImage::Format_ARGB32_Premultiplied] = qt_blend_argb32_on_argb32_ssse3;
      blendFunctions[QImage::Format_ARGB32_Premultiplied][QImage::Format_ARGB32_Premultiplied] = qt_blend_argb32_on_argb32_ssse3;
      blendFunctions[QImage::Format_RGBX8888][QImage::Format_RGBA8888_Premultiplied] = qt_blend_argb32_on_argb32_ssse3;
      blendFunctions[QImage::Format_RGBA8888_Premultiplied][QImage::Format_RGBA8888_Premultiplied] = qt_blend_argb32_on_argb32_ssse3;
   }
#endif

#if defined(QT_COMPILER_SUPPORTS_SSE4_1)

   if (qCpuHasFeature(SSE4_1)) {

#if ! defined(__SSE4_1__)
      extern const uint *convertARGB32ToARGB32PM_sse4(uint * buffer, const uint * src, int count, const QPixelLayout *,
         const QRgb *);
      extern const uint *convertRGBA8888ToARGB32PM_sse4(uint * buffer, const uint * src, int count, const QPixelLayout *,
         const QRgb *);

      qPixelLayouts[QImage::Format_ARGB32].convertToARGB32PM   = convertARGB32ToARGB32PM_sse4;
      qPixelLayouts[QImage::Format_RGBA8888].convertToARGB32PM = convertRGBA8888ToARGB32PM_sse4;
#endif

      extern const uint *convertARGB32FromARGB32PM_sse4(uint * buffer, const uint * src, int count, const QPixelLayout *,
         const QRgb *);
      extern const uint *convertRGBA8888FromARGB32PM_sse4(uint * buffer, const uint * src, int count, const QPixelLayout *,
         const QRgb *);
      extern const uint *convertRGBXFromARGB32PM_sse4(uint * buffer, const uint * src, int count, const QPixelLayout *,
         const QRgb *);

      qPixelLayouts[QImage::Format_ARGB32].convertFromARGB32PM = convertARGB32FromARGB32PM_sse4;
      qPixelLayouts[QImage::Format_RGBA8888].convertFromARGB32PM = convertRGBA8888FromARGB32PM_sse4;
      qPixelLayouts[QImage::Format_RGBX8888].convertFromARGB32PM = convertRGBXFromARGB32PM_sse4;
      qPixelLayouts[QImage::Format_A2BGR30_Premultiplied].convertFromARGB32PM = convertA2RGB30PMFromARGB32PM_sse4<PixelOrderBGR>;
      qPixelLayouts[QImage::Format_A2RGB30_Premultiplied].convertFromARGB32PM = convertA2RGB30PMFromARGB32PM_sse4<PixelOrderRGB>;
   }
#endif

#if defined(QT_COMPILER_SUPPORTS_AVX2) && ! defined(__AVX2__)
   if (qCpuHasFeature(AVX2)) {
      extern const uint *convertARGB32ToARGB32PM_avx2(uint * buffer, const uint * src, int count, const QPixelLayout *,
         const QRgb *);
      extern const uint *convertRGBA8888ToARGB32PM_avx2(uint * buffer, const uint * src, int count, const QPixelLayout *,
         const QRgb *);

      qPixelLayouts[QImage::Format_ARGB32].convertToARGB32PM = convertARGB32ToARGB32PM_avx2;
      qPixelLayouts[QImage::Format_RGBA8888].convertToARGB32PM = convertRGBA8888ToARGB32PM_avx2;
   }
#endif
   extern void comp_func_SourceOver_sse2(uint * destPixels, const uint * srcPixels, int length, uint const_alpha);
   extern void comp_func_solid_SourceOver_sse2(uint * destPixels, int length, uint color, uint const_alpha);
   extern void comp_func_Source_sse2(uint * destPixels, const uint * srcPixels, int length, uint const_alpha);
   extern void comp_func_Plus_sse2(uint * destPixels, const uint * srcPixels, int length, uint const_alpha);

   qt_functionForMode_C[QPainter::CompositionMode_SourceOver] = comp_func_SourceOver_sse2;
   qt_functionForModeSolid_C[QPainter::CompositionMode_SourceOver] = comp_func_solid_SourceOver_sse2;
   qt_functionForMode_C[QPainter::CompositionMode_Source] = comp_func_Source_sse2;
   qt_functionForMode_C[QPainter::CompositionMode_Plus] = comp_func_Plus_sse2;

#endif // SSE2

#if defined(__ARM_NEON__)
   blendFunctions[QImage::Format_RGB32][QImage::Format_RGB32] = qt_blend_rgb32_on_rgb32_neon;
   blendFunctions[QImage::Format_ARGB32_Premultiplied][QImage::Format_RGB32] = qt_blend_rgb32_on_rgb32_neon;
   blendFunctions[QImage::Format_RGB32][QImage::Format_ARGB32_Premultiplied] = qt_blend_argb32_on_argb32_neon;
   blendFunctions[QImage::Format_ARGB32_Premultiplied][QImage::Format_ARGB32_Premultiplied] = qt_blend_argb32_on_argb32_neon;

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
   blendFunctions[QImage::Format_RGBX8888][QImage::Format_RGBX8888] = qt_blend_rgb32_on_rgb32_neon;
   blendFunctions[QImage::Format_RGBA8888_Premultiplied][QImage::Format_RGBX8888] = qt_blend_rgb32_on_rgb32_neon;
   blendFunctions[QImage::Format_RGBX8888][QImage::Format_RGBA8888_Premultiplied] = qt_blend_argb32_on_argb32_neon;
   blendFunctions[QImage::Format_RGBA8888_Premultiplied][QImage::Format_RGBA8888_Premultiplied] = qt_blend_argb32_on_argb32_neon;
#endif

   qt_functionForMode_C[QPainter::CompositionMode_SourceOver] = qt_blend_argb32_on_argb32_scanline_neon;
   qt_functionForModeSolid_C[QPainter::CompositionMode_SourceOver] = comp_func_solid_SourceOver_neon;
   qt_functionForMode_C[QPainter::CompositionMode_Plus] = comp_func_Plus_neon;

   extern const uint *qt_fetch_radial_gradient_neon(uint * buffer, const Operator * op, const QSpanData * data,
      int y, int x, int length);

   qt_fetch_radial_gradient = qt_fetch_radial_gradient_neon;

#if defined(ENABLE_PIXMAN_DRAWHELPERS)
   // The RGB16 helpers are using Arm32 assemblythat has not been ported to AArch64
   blendFunctions[QImage::Format_RGB16][QImage::Format_ARGB32_Premultiplied] = qt_blend_argb32_on_rgb16_neon;
   blendFunctions[QImage::Format_ARGB32_Premultiplied][QImage::Format_RGB16] = qt_blend_rgb16_on_argb32_neon;
   blendFunctions[QImage::Format_RGB16][QImage::Format_RGB16] = qt_blend_rgb16_on_rgb16_neon;

   scaleFunctions[QImage::Format_RGB16][QImage::Format_ARGB32_Premultiplied] = qt_scale_image_argb32_on_rgb16_neon;
   scaleFunctions[QImage::Format_RGB16][QImage::Format_RGB16] = qt_scale_image_rgb16_on_rgb16_neon;

   transformFunctions[QImage::Format_RGB16][QImage::Format_ARGB32_Premultiplied] = qt_transform_image_argb32_on_rgb16_neon;
   transformFunctions[QImage::Format_RGB16][QImage::Format_RGB16] = qt_transform_image_rgb16_on_rgb16_neon;

   drawHelper[QImage::Format_RGB16].alphamapBlit = qt_alphamapblit_quint16_neon;

   destFetchProc[QImage::Format_RGB16] = qt_destFetchRGB16_neon;
   destStoreProc[QImage::Format_RGB16] = qt_destStoreRGB16_neon;

   memRotateFunctions[QImage::Format_RGB16][0] = qt_memrotate90_16_neon;
   memRotateFunctions[QImage::Format_RGB16][2] = qt_memrotate270_16_neon;
#endif

#endif

#if defined(Q_PROCESSOR_MIPS_32) && defined(QT_COMPILER_SUPPORTS_MIPS_DSP)
   qt_memfill32 = qt_memfill32_asm_mips_dsp;
#endif

#if defined(QT_COMPILER_SUPPORTS_MIPS_DSP) || defined(QT_COMPILER_SUPPORTS_MIPS_DSPR2)

   if (qCpuHasFeature(DSP) && qCpuHasFeature(DSPR2)) {
      // Composition functions are all DSP r1

      qt_functionForMode_C[QPainter::CompositionMode_SourceOver] = comp_func_SourceOver_asm_mips_dsp;
      qt_functionForMode_C[QPainter::CompositionMode_Source] = comp_func_Source_mips_dsp;
      qt_functionForMode_C[QPainter::CompositionMode_DestinationOver] = comp_func_DestinationOver_mips_dsp;
      qt_functionForMode_C[QPainter::CompositionMode_SourceIn] = comp_func_SourceIn_mips_dsp;
      qt_functionForMode_C[QPainter::CompositionMode_DestinationIn] = comp_func_DestinationIn_mips_dsp;
      qt_functionForMode_C[QPainter::CompositionMode_DestinationOut] = comp_func_DestinationOut_mips_dsp;
      qt_functionForMode_C[QPainter::CompositionMode_SourceAtop] = comp_func_SourceAtop_mips_dsp;
      qt_functionForMode_C[QPainter::CompositionMode_DestinationAtop] = comp_func_DestinationAtop_mips_dsp;
      qt_functionForMode_C[QPainter::CompositionMode_Xor] = comp_func_XOR_mips_dsp;
      qt_functionForMode_C[QPainter::CompositionMode_SourceOut] = comp_func_SourceOut_mips_dsp;

      qt_functionForModeSolid_C[QPainter::CompositionMode_SourceOver] = comp_func_solid_SourceOver_mips_dsp;
      qt_functionForModeSolid_C[QPainter::CompositionMode_DestinationOver] = comp_func_solid_DestinationOver_mips_dsp;
      qt_functionForModeSolid_C[QPainter::CompositionMode_SourceIn] = comp_func_solid_SourceIn_mips_dsp;
      qt_functionForModeSolid_C[QPainter::CompositionMode_DestinationIn] = comp_func_solid_DestinationIn_mips_dsp;
      qt_functionForModeSolid_C[QPainter::CompositionMode_SourceAtop] = comp_func_solid_SourceAtop_mips_dsp;
      qt_functionForModeSolid_C[QPainter::CompositionMode_DestinationAtop] = comp_func_solid_DestinationAtop_mips_dsp;
      qt_functionForModeSolid_C[QPainter::CompositionMode_Xor] = comp_func_solid_XOR_mips_dsp;
      qt_functionForModeSolid_C[QPainter::CompositionMode_SourceOut] = comp_func_solid_SourceOut_mips_dsp;

      blendFunctions[QImage::Format_RGB32][QImage::Format_RGB32] = qt_blend_rgb32_on_rgb32_mips_dsp;
      blendFunctions[QImage::Format_ARGB32_Premultiplied][QImage::Format_RGB32] = qt_blend_rgb32_on_rgb32_mips_dsp;
      blendFunctions[QImage::Format_RGB32][QImage::Format_ARGB32_Premultiplied] = qt_blend_argb32_on_argb32_mips_dsp;
      blendFunctions[QImage::Format_ARGB32_Premultiplied][QImage::Format_ARGB32_Premultiplied] = qt_blend_argb32_on_argb32_mips_dsp;

      destFetchProc[QImage::Format_ARGB32] = qt_destFetchARGB32_mips_dsp;

      destStoreProc[QImage::Format_ARGB32] = qt_destStoreARGB32_mips_dsp;

      sourceFetch[BlendUntransformed][QImage::Format_RGB888] = qt_fetchUntransformed_888_mips_dsp;
      sourceFetch[BlendTiled][QImage::Format_RGB888] = qt_fetchUntransformed_888_mips_dsp;

      sourceFetch[BlendUntransformed][QImage::Format_RGB444] = qt_fetchUntransformed_444_mips_dsp;
      sourceFetch[BlendTiled][QImage::Format_RGB444] = qt_fetchUntransformed_444_mips_dsp;

      sourceFetch[BlendUntransformed][QImage::Format_ARGB8565_Premultiplied] = qt_fetchUntransformed_argb8565_premultiplied_mips_dsp;
      sourceFetch[BlendTiled][QImage::Format_ARGB8565_Premultiplied] = qt_fetchUntransformed_argb8565_premultiplied_mips_dsp;

#if defined(QT_COMPILER_SUPPORTS_MIPS_DSPR2)
      blendFunctions[QImage::Format_RGB16][QImage::Format_RGB16] = qt_blend_rgb16_on_rgb16_mips_dspr2;
#else
      blendFunctions[QImage::Format_RGB16][QImage::Format_RGB16] = qt_blend_rgb16_on_rgb16_mips_dsp;
#endif

   }

#endif // QT_COMPILER_SUPPORTS_MIPS_DSP || QT_COMPILER_SUPPORTS_MIPS_DSPR2

}

const QDrawHelperFunctions & QDrawHelperFunctions::instance()
{
  static QDrawHelperFunctions retval;
  return retval;
}
