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

#ifndef QGL_PIXELBUFFER_P_H
#define QGL_PIXELBUFFER_P_H

#include <qglpixelbuffer.h>
#include <qgl_p.h>
#include <qglpaintdevice_p.h>
#include <qglobal.h>

class QEglContext;
class QOpenGLFramebufferObject;

class QGLPBufferGLPaintDevice : public QGLPaintDevice
{
 public:
   QPaintEngine *paintEngine() const override {
      return pbuf->paintEngine();
   }

   QSize size() const override {
      return pbuf->size();
   }

   QGLContext *context() const override;
   void beginPaint() override;
   void endPaint() override;
   void setPBuffer(QGLPixelBuffer *pb);
   void setFbo(GLuint fbo);

 private:
   QGLPixelBuffer *pbuf;
};

class QGLPixelBufferPrivate
{
   Q_DECLARE_PUBLIC(QGLPixelBuffer)

 public:
   QGLPixelBufferPrivate(QGLPixelBuffer *q)
      : q_ptr(q), invalid(true), qctx(nullptr), widget(nullptr), fbo(nullptr), blit_fbo(nullptr), pbuf(nullptr), ctx(nullptr)
   {
   }

   bool init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget);
   void common_init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget);
   bool cleanup();

   QGLPixelBuffer *q_ptr;
   bool invalid;
   QGLContext *qctx;
   QGLPBufferGLPaintDevice glDevice;

   QGLWidget *widget;
   QOpenGLFramebufferObject *fbo;
   QOpenGLFramebufferObject *blit_fbo;

   QGLFormat format;

   QGLFormat req_format;
   QPointer<QGLWidget> req_shareWidget;
   QSize req_size;

   //stubs
   void *pbuf;
   void *ctx;
};

#endif
