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

#include <qcolormap.h>
#include <qcolor.h>
#include <qpaintdevice.h>
#include <qscreen_qws.h>
#include <qwsdisplay_qws.h>

QT_BEGIN_NAMESPACE

class QColormapPrivate
{
 public:
   inline QColormapPrivate()
      : ref(1), mode(QColormap::Direct), depth(0), numcolors(0) {
   }

   QAtomicInt ref;

   QColormap::Mode mode;
   int depth;
   int numcolors;
};

static QColormapPrivate *screenMap = 0;

void QColormap::initialize()
{
   screenMap = new QColormapPrivate;

   screenMap->depth = QPaintDevice::qwsDisplay()->depth();
   if (screenMap->depth < 8) {
      screenMap->mode = QColormap::Indexed;
      screenMap->numcolors = 256;
   } else {
      screenMap->mode = QColormap::Direct;
      screenMap->numcolors = -1;
   }
}

void QColormap::cleanup()
{
   delete screenMap;
   screenMap = 0;
}

QColormap QColormap::instance(int /*screen*/)
{
   return QColormap();
}

QColormap::QColormap()
   : d(screenMap)
{
   d->ref.ref();
}

QColormap::QColormap(const QColormap &colormap)
   : d (colormap.d)
{
   d->ref.ref();
}

QColormap::~QColormap()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

QColormap::Mode QColormap::mode() const
{
   return d->mode;
}


int QColormap::depth() const
{
   return d->depth;
}


int QColormap::size() const
{
   return d->numcolors;
}

uint QColormap::pixel(const QColor &color) const
{
   QRgb rgb = color.rgba();
   if (d->mode == QColormap::Direct) {
      switch (d->depth) {
         case 16:
            return qt_convRgbTo16(rgb);
         case 24:
         case 32: {
            const int r = qRed(rgb);
            const int g = qGreen(rgb);
            const int b = qBlue(rgb);
            const int red_shift = 16;
            const int green_shift = 8;
            const int red_mask   = 0xff0000;
            const int green_mask = 0x00ff00;
            const int blue_mask  = 0x0000ff;
            const int tg = g << green_shift;
#ifdef QT_QWS_DEPTH_32_BGR
            if (qt_screen->pixelType() == QScreen::BGRPixel) {
               const int tb = b << red_shift;
               return 0xff000000 | (r & blue_mask) | (tg & green_mask) | (tb & red_mask);
            }
#endif
            const int tr = r << red_shift;
            return 0xff000000 | (b & blue_mask) | (tg & green_mask) | (tr & red_mask);
         }
      }
   }
   return qt_screen->alloc(qRed(rgb), qGreen(rgb), qBlue(rgb));
}

const QColor QColormap::colorAt(uint pixel) const
{
   if (d->mode == Direct) {
      if (d->depth == 16) {
         pixel = qt_conv16ToRgb(pixel);
      }
      const int red_shift = 16;
      const int green_shift = 8;
      const int red_mask   = 0xff0000;
      const int green_mask = 0x00ff00;
      const int blue_mask  = 0x0000ff;
#ifdef QT_QWS_DEPTH_32_BGR
      if (qt_screen->pixelType() == QScreen::BGRPixel) {
         return QColor((pixel & blue_mask),
                       (pixel & green_mask) >> green_shift,
                       (pixel & red_mask) >> red_shift);
      }
#endif
      return QColor((pixel & red_mask) >> red_shift,
                    (pixel & green_mask) >> green_shift,
                    (pixel & blue_mask));
   }
   Q_ASSERT_X(int(pixel) < qt_screen->colorCount(), "QColormap::colorAt", "pixel out of bounds of palette");
   return QColor(qt_screen->clut()[pixel]);
}

const QVector<QColor> QColormap::colormap() const
{
   return QVector<QColor>();
}

QColormap &QColormap::operator=(const QColormap &colormap)
{
   qAtomicAssign(d, colormap.d);
   return *this;
}

QT_END_NAMESPACE
