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

QT_BEGIN_NAMESPACE

class QColormapPrivate
{
 public:
   inline QColormapPrivate()
      : ref(1) {
   }

   QAtomicInt ref;
};
static QColormap *qt_mac_global_map = 0;

void QColormap::initialize()
{
   qt_mac_global_map = new QColormap;
}

void QColormap::cleanup()
{
   delete qt_mac_global_map;
   qt_mac_global_map = 0;
}

QColormap QColormap::instance(int)
{
   return *qt_mac_global_map;
}

QColormap::QColormap() : d(new QColormapPrivate)
{}

QColormap::QColormap(const QColormap &colormap) : d (colormap.d)
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
   return QColormap::Direct;
}

int QColormap::depth() const
{
   return 32;
}

int QColormap::size() const
{
   return -1;
}

uint QColormap::pixel(const QColor &color) const
{
   return color.rgba();
}

const QColor QColormap::colorAt(uint pixel) const
{
   return QColor(pixel);
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
