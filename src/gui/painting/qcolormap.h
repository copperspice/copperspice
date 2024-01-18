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

#ifndef QCOLORMAP_H
#define QCOLORMAP_H

#include <qatomic.h>
#include <qrgb.h>
#include <qvector.h>
#include <qwindowdefs.h>


class QColor;
class QColormapPrivate;

class Q_GUI_EXPORT QColormap
{
 public:
   enum Mode { Direct, Indexed, Gray };

   static void initialize();
   static void cleanup();

   static QColormap instance(int screen = -1);

   QColormap(const QColormap &colormap);
   ~QColormap();

   QColormap &operator=(const QColormap &colormap);

   Mode mode() const;

   int depth() const;
   int size() const;

   uint pixel(const QColor &color) const;
   const QColor colorAt(uint pixel) const;

   const QVector<QColor> colormap() const;


 private:
   QColormap();
   QColormapPrivate *d;
};

#endif
