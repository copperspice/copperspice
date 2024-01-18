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

#include <qglcolormap.h>

QGLColormap::QGLColormapData QGLColormap::shared_null = { 1, nullptr, nullptr };

QGLColormap::QGLColormap()
   : d(&shared_null)
{
   d->ref.ref();
}

QGLColormap::QGLColormap(const QGLColormap &map)
   : d(map.d)
{
   d->ref.ref();
}

QGLColormap::~QGLColormap()
{
   if (! d->ref.deref()) {
      cleanup(d);
   }
}

void QGLColormap::cleanup(QGLColormap::QGLColormapData *x)
{
   delete x->cells;
   x->cells = nullptr;

   delete x;
}

QGLColormap &QGLColormap::operator=(const QGLColormap &map)
{
   map.d->ref.ref();

   if (!d->ref.deref()) {
      cleanup(d);
   }
   d = map.d;
   return *this;
}

void QGLColormap::detach_helper()
{
   QGLColormapData *x = new QGLColormapData;
   x->ref.store(1);
   x->cmapHandle = nullptr;
   x->cells = nullptr;

   if (d->cells) {
      x->cells = new QVector<QRgb>(256);
      *x->cells = *d->cells;
   }

   if (!d->ref.deref()) {
      cleanup(d);
   }
   d = x;
}

void QGLColormap::setEntry(int idx, QRgb color)
{
   detach();

   if (!d->cells) {
      d->cells = new QVector<QRgb>(256);
   }
   d->cells->replace(idx, color);
}

/*!
    Set an array of cells in this colormap. \a count is the number of
    colors that should be set, \a colors is the array of colors, and
    \a base is the starting index.  The first element in \a colors
    is set at \a base in the colormap.
*/
void QGLColormap::setEntries(int count, const QRgb *colors, int base)
{
   detach();
   if (!d->cells) {
      d->cells = new QVector<QRgb>(256);
   }

   Q_ASSERT_X(colors && base >= 0 && (base + count) <= d->cells->size(), "QGLColormap::setEntries",
      "preconditions not met");
   for (int i = 0; i < count; ++i) {
      setEntry(base + i, colors[i]);
   }
}

/*!
    Returns the QRgb value in the colorcell with index \a idx.
*/
QRgb QGLColormap::entryRgb(int idx) const
{
   if (d == &shared_null || !d->cells) {
      return 0;
   } else {
      return d->cells->at(idx);
   }
}

void QGLColormap::setEntry(int idx, const QColor &color)
{
   setEntry(idx, color.rgb());
}

QColor QGLColormap::entryColor(int idx) const
{
   if (d == &shared_null || !d->cells) {
      return QColor();
   } else {
      return QColor(d->cells->at(idx));
   }
}

bool QGLColormap::isEmpty() const
{
   return d == &shared_null || d->cells == nullptr || d->cells->size() == 0 || d->cmapHandle == nullptr;
}

int QGLColormap::size() const
{
   return d->cells ? d->cells->size() : 0;
}

int QGLColormap::find(QRgb color) const
{
   if (d->cells) {
      return d->cells->indexOf(color);
   }

   return -1;
}

int QGLColormap::findNearest(QRgb color) const
{
   int idx = find(color);
   if (idx >= 0) {
      return idx;
   }

   int mapSize = size();
   int mindist = 200000;
   int r = qRed(color);
   int g = qGreen(color);
   int b = qBlue(color);

   int rx, gx, bx, dist;

   for (int i = 0; i < mapSize; ++i) {
      QRgb ci = d->cells->at(i);
      rx = r - qRed(ci);
      gx = g - qGreen(ci);
      bx = b - qBlue(ci);
      dist = rx * rx + gx * gx + bx * bx;        // calculate distance

      if (dist < mindist) {
         // minimal?
         mindist = dist;
         idx = i;
      }
   }

   return idx;
}

