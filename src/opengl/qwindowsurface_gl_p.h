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

#ifndef QWINDOWSURFACE_GL_P_H
#define QWINDOWSURFACE_GL_P_H

#include <qglobal.h>
#include <qgl.h>
#include <qwindowsurface_p.h>
#include <qglpaintdevice_p.h>

QT_BEGIN_NAMESPACE

class QPaintDevice;
class QPoint;
class QRegion;
class QWidget;
struct QGLWindowSurfacePrivate;

Q_OPENGL_EXPORT QGLWidget *qt_gl_share_widget();
Q_OPENGL_EXPORT void qt_destroy_gl_share_widget();
bool qt_initializing_gl_share_widget();

class QGLWindowSurfaceGLPaintDevice : public QGLPaintDevice
{
 public:
   QPaintEngine *paintEngine() const;
   QSize size() const;
   int metric(PaintDeviceMetric m) const;
   QGLContext *context() const;
   QGLWindowSurfacePrivate *d;
};

class Q_OPENGL_EXPORT QGLWindowSurface : public QObject, public QWindowSurface // , public QPaintDevice
{
   CS_OBJECT(QGLWindowSurface)
 public:
   QGLWindowSurface(QWidget *window);
   ~QGLWindowSurface();

   QPaintDevice *paintDevice();
   void flush(QWidget *widget, const QRegion &region, const QPoint &offset);

#if !defined(Q_WS_QPA)
   void setGeometry(const QRect &rect);
#else
   virtual void resize(const QSize &size);
#endif

   void updateGeometry();
   bool scroll(const QRegion &area, int dx, int dy);

   void beginPaint(const QRegion &region);
   void endPaint(const QRegion &region);

   QImage *buffer(const QWidget *widget);

   WindowSurfaceFeatures features() const;

   QGLContext *context() const;

   static QGLFormat surfaceFormat;

   enum SwapMode { AutomaticSwap, AlwaysFullSwap, AlwaysPartialSwap, KillSwap };
   static SwapMode swapBehavior;

 private :
   OPENGL_CS_SLOT_1(Private, void deleted(QObject *object))
   OPENGL_CS_SLOT_2(deleted)

 private:
   void hijackWindow(QWidget *widget);
   bool initializeOffscreenTexture(const QSize &size);

   QGLWindowSurfacePrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QWINDOWSURFACE_GL_P_H

