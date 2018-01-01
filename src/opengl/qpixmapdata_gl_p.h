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

#ifndef QPIXMAPDATA_GL_P_H
#define QPIXMAPDATA_GL_P_H

#include "qgl_p.h"
#include "qgl.h"
#include "qpixmapdata_p.h"
#include "qglpaintdevice_p.h"

QT_BEGIN_NAMESPACE

class QPaintEngine;
class QGLFramebufferObject;
class QGLFramebufferObjectFormat;
class QGLPixmapData;

class QGLFramebufferObjectPool
{
 public:
   QGLFramebufferObject *acquire(const QSize &size, const QGLFramebufferObjectFormat &format, bool strictSize = false);
   void release(QGLFramebufferObject *fbo);

 private:
   QList<QGLFramebufferObject *> m_fbos;
};

QGLFramebufferObjectPool *qgl_fbo_pool();


class QGLPixmapGLPaintDevice : public QGLPaintDevice
{
 public:
   QPaintEngine *paintEngine() const override;

   void beginPaint() override;
   void endPaint() override;
   QGLContext *context() const override;
   QSize size() const override;
   bool alphaRequested() const override;

   void setPixmapData(QGLPixmapData *);

 private:
   QGLPixmapData *data;
};

class Q_OPENGL_EXPORT QGLPixmapData : public QPixmapData
{
 public:
   QGLPixmapData(PixelType type);
   ~QGLPixmapData();

   QPixmapData *createCompatiblePixmapData() const override;

   // Re-implemented from QPixmapData:
   void resize(int width, int height) override;
   void fromImage(const QImage &image, Qt::ImageConversionFlags flags) override;
   void fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags) override;
   bool fromFile(const QString &filename, const char *format, Qt::ImageConversionFlags flags) override;
   bool fromData(const uchar *buffer, uint len, const char *format, Qt::ImageConversionFlags flags) override;
   void copy(const QPixmapData *data, const QRect &rect) override;
   bool scroll(int dx, int dy, const QRect &rect) override;
   void fill(const QColor &color) override;
   bool hasAlphaChannel() const override;
   QImage toImage() const override;
   QPaintEngine *paintEngine() const override;
   int metric(QPaintDevice::PaintDeviceMetric metric) const override;

   // For accessing as a target:
   QGLPaintDevice *glDevice() const;

   // For accessing as a source:
   bool isValidContext(const QGLContext *ctx) const;
   GLuint bind(bool copyBack = true) const;
   QGLTexture *texture() const;

 private:
   bool isValid() const;

   void ensureCreated() const;

   bool isUninitialized() const {
      return m_dirty && m_source.isNull();
   }

   bool needsFill() const {
      return m_hasFillColor;
   }
   QColor fillColor() const {
      return m_fillColor;
   }

   QGLPixmapData(const QGLPixmapData &other);
   QGLPixmapData &operator=(const QGLPixmapData &other);

   void copyBackFromRenderFbo(bool keepCurrentFboBound) const;
   QSize size() const {
      return QSize(w, h);
   }

   bool useFramebufferObjects() const;

   QImage fillImage(const QColor &color) const;

   void createPixmapForImage(QImage &image, Qt::ImageConversionFlags flags, bool inPlace);

   mutable QGLFramebufferObject *m_renderFbo;
   mutable QPaintEngine *m_engine;
   mutable QGLContext *m_ctx;

   mutable QImage m_source;
   mutable QGLTexture m_texture;

   // the texture is not in sync with the source image
   mutable bool m_dirty;

   // fill has been called and no painting has been done, so the pixmap is
   // represented by a single fill color
   mutable QColor m_fillColor;
   mutable bool m_hasFillColor;

   mutable bool m_hasAlpha;
   mutable QGLPixmapGLPaintDevice m_glDevice;

   friend class QGLPixmapGLPaintDevice;
   friend class QMeeGoPixmapData;
   friend class QMeeGoLivePixmapData;
};

QT_END_NAMESPACE

#endif // QPIXMAPDATA_GL_P_H


