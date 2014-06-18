/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPIXMAPDATA_X11GL_P_H
#define QPIXMAPDATA_X11GL_P_H

#include <qpixmapdata_p.h>
#include <qpixmap_x11_p.h>
#include <qglpaintdevice_p.h>
#include <qgl.h>

#ifndef QT_NO_EGL
#include <qeglcontext_p.h>
#endif

QT_BEGIN_NAMESPACE

class QX11GLSharedContexts;

class QX11GLPixmapData : public QX11PixmapData, public QGLPaintDevice
{
 public:
   QX11GLPixmapData();
   virtual ~QX11GLPixmapData();

   // Re-implemented from QX11PixmapData:
   void fill(const QColor &color);
   void copy(const QPixmapData *data, const QRect &rect);
   bool scroll(int dx, int dy, const QRect &rect);

   // Re-implemented from QGLPaintDevice
   QPaintEngine *paintEngine() const; // Also re-implements QX11PixmapData::paintEngine
   void beginPaint();
   QGLContext *context() const;
   QSize size() const;

   static bool hasX11GLPixmaps();
   static QGLFormat glFormat();
   static QX11GLSharedContexts *sharedContexts();

 private:
   mutable QGLContext *ctx;
};


QT_END_NAMESPACE

#endif // QPIXMAPDATA_X11GL_P_H
