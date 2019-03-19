/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qcolor.h>
#include <qcolormap.h>
#include <qvector.h>
#include <qt_windows.h>

QT_BEGIN_NAMESPACE

class QColormapPrivate
{
 public:
   inline QColormapPrivate()
      : ref(1), mode(QColormap::Direct), depth(0), hpal(0) {
   }

   QAtomicInt ref;

   QColormap::Mode mode;
   int depth;
   int numcolors;

   HPALETTE hpal;
   QVector<QColor> palette;
};

static QColormapPrivate *screenMap = 0;

void QColormap::initialize()
{
   HDC dc = qt_win_display_dc();

   screenMap = new QColormapPrivate;
   screenMap->depth = GetDeviceCaps(dc, BITSPIXEL);

   screenMap->numcolors = -1;
   if (GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE) {
      screenMap->numcolors = GetDeviceCaps(dc, SIZEPALETTE);
   }

   if (screenMap->numcolors <= 16 || screenMap->numcolors > 256) {      // no need to create palette
      return;
   }

   LOGPALETTE *pal = 0;
   int numPalEntries = 6 * 6 * 6; // color cube

   pal = (LOGPALETTE *)malloc(sizeof(LOGPALETTE) + numPalEntries * sizeof(PALETTEENTRY));
   // Make 6x6x6 color cube
   int idx = 0;
   for (int ir = 0x0; ir <= 0xff; ir += 0x33) {
      for (int ig = 0x0; ig <= 0xff; ig += 0x33) {
         for (int ib = 0x0; ib <= 0xff; ib += 0x33) {
            pal->palPalEntry[idx].peRed = ir;
            pal->palPalEntry[idx].peGreen = ig;
            pal->palPalEntry[idx].peBlue = ib;
            pal->palPalEntry[idx].peFlags = 0;
            idx++;
         }
      }
   }

   pal->palVersion = 0x300;
   pal->palNumEntries = numPalEntries;

   screenMap->hpal = CreatePalette(pal);
   if (!screenMap->hpal) {
      qErrnoWarning("QColor::initialize: Failed to create logical palette");
   }
   free (pal);

   SelectPalette(dc, screenMap->hpal, FALSE);
   RealizePalette(dc);

   PALETTEENTRY paletteEntries[256];
   screenMap->numcolors = GetPaletteEntries(screenMap->hpal, 0, 255, paletteEntries);

   screenMap->palette.resize(screenMap->numcolors);
   for (int i = 0; i < screenMap->numcolors; i++) {
      screenMap->palette[i] = qRgb(paletteEntries[i].peRed,
                                   paletteEntries[i].peGreen,
                                   paletteEntries[i].peBlue);
   }
}

void QColormap::cleanup()
{
   if (!screenMap) {
      return;
   }

   if (screenMap->hpal) {                                // delete application global
      DeleteObject(screenMap->hpal);                        // palette
      screenMap->hpal = 0;
   }

   delete screenMap;
   screenMap = 0;
}

QColormap QColormap::instance(int)
{
   Q_ASSERT_X(screenMap, "QColormap",
              "A QApplication object needs to be constructed before QColormap is used.");
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
   const QColor c = color.toRgb();
   COLORREF rgb = RGB(c.red(), c.green(), c.blue());
   if (d->hpal) {
      return PALETTEINDEX(GetNearestPaletteIndex(d->hpal, rgb));
   }
   return rgb;
}

const QColor QColormap::colorAt(uint pixel) const
{
   if (d->hpal) {
      if (pixel < uint(d->numcolors)) {
         return d->palette.at(pixel);
      }
      return QColor();
   }
   return QColor(GetRValue(pixel), GetGValue(pixel), GetBValue(pixel));
}


HPALETTE QColormap::hPal()
{
   return screenMap ? screenMap->hpal : 0;
}


const QVector<QColor> QColormap::colormap() const
{
   return d->palette;
}

QColormap &QColormap::operator=(const QColormap &colormap)
{
   qAtomicAssign(d, colormap.d);
   return *this;
}


QT_END_NAMESPACE
