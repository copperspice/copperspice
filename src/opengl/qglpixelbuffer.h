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

#ifndef QGLPIXELBUFFER_H
#define QGLPIXELBUFFER_H

#include <QtOpenGL/qgl.h>
#include <QtGui/qpaintdevice.h>

QT_BEGIN_NAMESPACE

class QGLPixelBufferPrivate;

class Q_OPENGL_EXPORT QGLPixelBuffer : public QPaintDevice
{
   Q_DECLARE_PRIVATE(QGLPixelBuffer)
 public:
   QGLPixelBuffer(const QSize &size, const QGLFormat &format = QGLFormat::defaultFormat(),
                  QGLWidget *shareWidget = 0);
   QGLPixelBuffer(int width, int height, const QGLFormat &format = QGLFormat::defaultFormat(),
                  QGLWidget *shareWidget = 0);
   virtual ~QGLPixelBuffer();

   bool isValid() const;
   bool makeCurrent();
   bool doneCurrent();

   GLuint generateDynamicTexture() const;
   bool bindToDynamicTexture(GLuint texture);
   void releaseFromDynamicTexture();
   void updateDynamicTexture(GLuint texture_id) const;

   GLuint bindTexture(const QImage &image, GLenum target = GL_TEXTURE_2D);
   GLuint bindTexture(const QPixmap &pixmap, GLenum target = GL_TEXTURE_2D);
   GLuint bindTexture(const QString &fileName);
   void deleteTexture(GLuint texture_id);

   void drawTexture(const QRectF &target, GLuint textureId, GLenum textureTarget = GL_TEXTURE_2D);
   void drawTexture(const QPointF &point, GLuint textureId, GLenum textureTarget = GL_TEXTURE_2D);

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
   bool bindToDynamicTexture(QMacCompatGLuint texture);
   void updateDynamicTexture(QMacCompatGLuint texture_id) const;
   GLuint bindTexture(const QImage &image, QMacCompatGLenum target = GL_TEXTURE_2D);
   GLuint bindTexture(const QPixmap &pixmap, QMacCompatGLenum target = GL_TEXTURE_2D);

   void drawTexture(const QRectF &target, QMacCompatGLuint textureId, QMacCompatGLenum textureTarget = GL_TEXTURE_2D);
   void drawTexture(const QPointF &point, QMacCompatGLuint textureId, QMacCompatGLenum textureTarget = GL_TEXTURE_2D);

   void deleteTexture(QMacCompatGLuint texture_id);
#endif

   QSize size() const;
   Qt::HANDLE handle() const;
   QImage toImage() const;

   QPaintEngine *paintEngine() const override;
   QGLFormat format() const;

   static bool hasOpenGLPbuffers();

 protected:
   int metric(PaintDeviceMetric metric) const override;

   int devType() const override{
      return QInternal::Pbuffer;
   }

 private:
   Q_DISABLE_COPY(QGLPixelBuffer)
   QScopedPointer<QGLPixelBufferPrivate> d_ptr;
   friend class QGLDrawable;
   friend class QGLWindowSurface;
   friend class QGLPaintDevice;
   friend class QGLPBufferGLPaintDevice;
   friend class QGLContextPrivate;
};

QT_END_NAMESPACE

#endif // QGLPIXELBUFFER_H
