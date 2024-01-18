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

#ifndef QGLPIXELBUFFER_H
#define QGLPIXELBUFFER_H

#include <qgl.h>
#include <qpaintdevice.h>

class QGLPixelBufferPrivate;

class Q_OPENGL_EXPORT QGLPixelBuffer : public QPaintDevice
{
 public:
   QGLPixelBuffer(const QSize &size, const QGLFormat &format = QGLFormat::defaultFormat(),
      QGLWidget *shareWidget = nullptr);

   QGLPixelBuffer(int width, int height, const QGLFormat &format = QGLFormat::defaultFormat(),
      QGLWidget *shareWidget = nullptr);

   QGLPixelBuffer(const QGLPixelBuffer &) = delete;
   QGLPixelBuffer &operator=(const QGLPixelBuffer &) = delete;

   virtual ~QGLPixelBuffer();

   bool isValid() const;
   bool makeCurrent();
   bool doneCurrent();
   QGLContext *context() const;

   GLuint generateDynamicTexture() const;
   bool bindToDynamicTexture(GLuint texture_id);
   void releaseFromDynamicTexture();
   void updateDynamicTexture(GLuint texture_id) const;

   GLuint bindTexture(const QImage &image, GLenum target = GL_TEXTURE_2D);
   GLuint bindTexture(const QPixmap &pixmap, GLenum target = GL_TEXTURE_2D);
   GLuint bindTexture(const QString &fileName);
   void deleteTexture(GLuint texture_id);

   void drawTexture(const QRectF &target, GLuint texture_id, GLenum textureTarget = GL_TEXTURE_2D);
   void drawTexture(const QPointF &point, GLuint texture_id, GLenum textureTarget = GL_TEXTURE_2D);

   QSize size() const;
   Qt::HANDLE handle() const;
   QImage toImage() const;

   QPaintEngine *paintEngine() const override;
   QGLFormat format() const;

   static bool hasOpenGLPbuffers();

 protected:
   int metric(PaintDeviceMetric metric) const override;

   int devType() const override {
      return QInternal::Pbuffer;
   }

 private:
   Q_DECLARE_PRIVATE(QGLPixelBuffer)
   QScopedPointer<QGLPixelBufferPrivate> d_ptr;

   friend class QGLDrawable;
   friend class QGLPaintDevice;
   friend class QGLPBufferGLPaintDevice;
   friend class QGLContextPrivate;
};

#endif
