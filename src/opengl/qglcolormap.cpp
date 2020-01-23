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

#include <qglcolormap.h>


QGLColormap::QGLColormapData QGLColormap::shared_null = { 1, 0, 0 };

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
   x->cells = 0;
   delete x;
}

/*!
    Assign a shallow copy of \a map to this QGLColormap.
*/
QGLColormap &QGLColormap::operator=(const QGLColormap &map)
{
   map.d->ref.ref();
   if (!d->ref.deref()) {
      cleanup(d);
   }
   d = map.d;
   return *this;
}

/*!
    \fn void QGLColormap::detach()
    \internal

    Detaches this QGLColormap from the shared block.
*/

void QGLColormap::detach_helper()
{
   QGLColormapData *x = new QGLColormapData;
   x->ref.store(1);
   x->cmapHandle = 0;
   x->cells = 0;
   if (d->cells) {
      x->cells = new QVector<QRgb>(256);
      *x->cells = *d->cells;
   }
   if (!d->ref.deref()) {
      cleanup(d);
   }
   d = x;
}

/*!
    Set cell at index \a idx in the colormap to color \a color.
*/
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

/*!
    \overload

    Set the cell with index \a idx in the colormap to color \a color.
*/
void QGLColormap::setEntry(int idx, const QColor &color)
{
   setEntry(idx, color.rgb());
}

/*!
    Returns the QRgb value in the colorcell with index \a idx.
*/
QColor QGLColormap::entryColor(int idx) const
{
   if (d == &shared_null || !d->cells) {
      return QColor();
   } else {
      return QColor(d->cells->at(idx));
   }
}

/*!
    Returns true if the colormap is empty or it is not in use
    by a QGLWidget; otherwise returns false.

    A colormap with no color values set is considered to be empty.
    For historical reasons, a colormap that has color values set
    but which is not in use by a QGLWidget is also considered empty.

    Compare size() with zero to determine if the colormap is empty
    regardless of whether it is in use by a QGLWidget or not.

    \sa size()
*/
bool QGLColormap::isEmpty() const
{
   return d == &shared_null || d->cells == 0 || d->cells->size() == 0 || d->cmapHandle == 0;
}


/*!
    Returns the number of colorcells in the colormap.
*/
int QGLColormap::size() const
{
   return d->cells ? d->cells->size() : 0;
}

/*!
    Returns the index of the color \a color. If \a color is not in the
    map, -1 is returned.
*/
int QGLColormap::find(QRgb color) const
{
   if (d->cells) {
      return d->cells->indexOf(color);
   }
   return -1;
}

/*!
    Returns the index of the color that is the closest match to color
    \a color.
*/
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
      if (dist < mindist) {                // minimal?
         mindist = dist;
         idx = i;
      }
   }
   return idx;
}

