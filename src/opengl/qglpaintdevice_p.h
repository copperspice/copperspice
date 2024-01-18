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

#ifndef QGLPAINTDEVICE_P_H
#define QGLPAINTDEVICE_P_H

#include <qpaintdevice.h>
#include <qgl.h>


class Q_OPENGL_EXPORT QGLPaintDevice : public QPaintDevice
{
 public:
   QGLPaintDevice();
   virtual ~QGLPaintDevice();

   int devType() const override {
      return QInternal::OpenGL;
   }

   virtual void beginPaint();
   virtual void ensureActiveTarget();
   virtual void endPaint();

   virtual QGLContext *context() const = 0;
   virtual QGLFormat format() const;
   virtual QSize size() const = 0;
   virtual bool alphaRequested() const;
   virtual bool isFlipped() const;

   // returns the QGLPaintDevice for the given QPaintDevice
   static QGLPaintDevice *getDevice(QPaintDevice *);

 protected:
   int metric(QPaintDevice::PaintDeviceMetric metric) const override;
   GLuint m_previousFBO;
   GLuint m_thisFBO;
};

// Wraps a QGLWidget
class QGLWidget;
class QGLWidgetGLPaintDevice : public QGLPaintDevice
{
 public:
   QGLWidgetGLPaintDevice();

   QPaintEngine *paintEngine() const override;

   // QGLWidgets need to do swapBufers in endPaint:
   void beginPaint() override;
   void endPaint() override;
   QSize size() const override;
   QGLContext *context() const override;

   void setWidget(QGLWidget *);

 private:
   friend class QGLWidget;
   QGLWidget *glWidget;
};



#endif
