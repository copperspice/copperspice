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

#ifndef QGLCOLORMAP_H
#define QGLCOLORMAP_H

#include <qcolor.h>
#include <qvector.h>

class Q_OPENGL_EXPORT QGLColormap
{
 public:
   QGLColormap();
   QGLColormap(const QGLColormap &other);

   ~QGLColormap();

   QGLColormap &operator=(const QGLColormap &other);

   bool   isEmpty() const;
   int    size() const;
   void   detach();

   void   setEntries(int count, const QRgb *colors, int base = 0);
   void   setEntry(int idx, QRgb color);
   void   setEntry(int idx, const QColor &color);
   QRgb   entryRgb(int idx) const;
   QColor entryColor(int idx) const;
   int    find(QRgb color) const;
   int    findNearest(QRgb color) const;

 protected:
   Qt::HANDLE handle() {
      return d ? d->cmapHandle : nullptr;
   }

   void setHandle(Qt::HANDLE ahandle) {
      d->cmapHandle = ahandle;
   }

 private:
   struct QGLColormapData {
      QAtomicInt ref;
      QVector<QRgb> *cells;
      Qt::HANDLE cmapHandle;
   };

   QGLColormapData *d;
   static struct QGLColormapData shared_null;
   static void cleanup(QGLColormapData *x);
   void detach_helper();

   friend class QGLWidget;
   friend class QGLWidgetPrivate;
};

inline void QGLColormap::detach()
{
   if (d->ref.load() != 1) {
      detach_helper();
   }
}

#endif
