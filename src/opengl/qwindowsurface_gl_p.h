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
   QPaintEngine *paintEngine() const override;
   QSize size() const override;
   int metric(PaintDeviceMetric m) const override;
   QGLContext *context() const override;
   QGLWindowSurfacePrivate *d;
};

class Q_OPENGL_EXPORT QGLWindowSurface : public QObject, public QWindowSurface // , public QPaintDevice
{
   OPENGL_CS_OBJECT(QGLWindowSurface)
 public:
   QGLWindowSurface(QWidget *window);
   ~QGLWindowSurface();

   QPaintDevice *paintDevice() override;
   void flush(QWidget *widget, const QRegion &region, const QPoint &offset) override;

#if !defined(Q_WS_QPA)
   void setGeometry(const QRect &rect) override;
#else
   virtual void resize(const QSize &size);
#endif

   void updateGeometry();
   bool scroll(const QRegion &area, int dx, int dy) override;

   void beginPaint(const QRegion &region) override;
   void endPaint(const QRegion &region) override;

   QImage *buffer(const QWidget *widget) override;

   WindowSurfaceFeatures features() const override;

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

