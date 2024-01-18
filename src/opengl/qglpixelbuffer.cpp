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

#include <qimage.h>
#include <qglobal.h>
#include <qopengl_framebufferobject.h>
#include <qglframebufferobject.h>
#include <qglpixelbuffer.h>

#include <qopengl_extensions_p.h>
#include <qpaintengineex_opengl2_p.h>
#include <qglpixelbuffer_p.h>
#include <qfont_p.h>

QImage cs_glRead_frameBuffer(const QSize &, bool, bool);

QGLContext *QGLPBufferGLPaintDevice::context() const
{
   return pbuf->d_func()->qctx;
}

void QGLPBufferGLPaintDevice::beginPaint()
{
   pbuf->makeCurrent();
   QGLPaintDevice::beginPaint();
}
void QGLPBufferGLPaintDevice::endPaint()
{
   QOpenGLContext::currentContext()->functions()->glFlush();
   QGLPaintDevice::endPaint();
}

void QGLPBufferGLPaintDevice::setFbo(GLuint fbo)
{
   m_thisFBO = fbo;
}

void QGLPBufferGLPaintDevice::setPBuffer(QGLPixelBuffer *pb)
{
   pbuf = pb;
}

void QGLPixelBufferPrivate::common_init(const QSize &size, const QGLFormat &format, QGLWidget *shareWidget)
{
   Q_Q(QGLPixelBuffer);

   if (init(size, format, shareWidget)) {
      req_size = size;
      req_format = format;
      req_shareWidget = shareWidget;
      invalid = false;

      glDevice.setPBuffer(q);

   }
}

QGLPixelBuffer::QGLPixelBuffer(const QSize &size, const QGLFormat &format, QGLWidget *shareWidget)
   : d_ptr(new QGLPixelBufferPrivate(this))
{
   Q_D(QGLPixelBuffer);
   d->common_init(size, format, shareWidget);
}

QGLPixelBuffer::QGLPixelBuffer(int width, int height, const QGLFormat &format, QGLWidget *shareWidget)
   : d_ptr(new QGLPixelBufferPrivate(this))
{
   Q_D(QGLPixelBuffer);
   d->common_init(QSize(width, height), format, shareWidget);
}

QGLPixelBuffer::~QGLPixelBuffer()
{
   Q_D(QGLPixelBuffer);

   // defined in qpaintengine_opengl.cpp
   QGLContext *current = const_cast<QGLContext *>(QGLContext::currentContext());
   if (current != d->qctx) {
      makeCurrent();
   }

   d->cleanup();

   if (current && current != d->qctx) {
      current->makeCurrent();
   }
}

bool QGLPixelBuffer::makeCurrent()
{
   Q_D(QGLPixelBuffer);
   if (d->invalid) {
      return false;
   }

   d->qctx->makeCurrent();

   if (!d->fbo) {
      QOpenGLFramebufferObjectFormat format;

      if (d->req_format.stencil()) {
         format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
      } else if (d->req_format.depth()) {
         format.setAttachment(QOpenGLFramebufferObject::Depth);
      }

      if (d->req_format.sampleBuffers()) {
         format.setSamples(d->req_format.samples());
      }

      d->fbo = new QOpenGLFramebufferObject(d->req_size, format);
      d->fbo->bind();
      d->glDevice.setFbo(d->fbo->handle());
      QOpenGLContext::currentContext()->functions()->glViewport(0, 0, d->req_size.width(), d->req_size.height());
   }
   return true;
}

bool QGLPixelBuffer::doneCurrent()
{
   Q_D(QGLPixelBuffer);
   if (d->invalid) {
      return false;
   }
   d->qctx->doneCurrent();
   return true;
}

QGLContext *QGLPixelBuffer::context() const
{
   Q_D(const QGLPixelBuffer);
   return d->qctx;
}

void QGLPixelBuffer::updateDynamicTexture(GLuint texture_id) const
{
   Q_D(const QGLPixelBuffer);
   if (d->invalid || !d->fbo) {
      return;
   }

   const QGLContext *ctx = QGLContext::currentContext();
   if (!ctx) {
      return;
   }

#undef glBindFramebuffer

#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif

   QOpenGLExtensions extensions(ctx->contextHandle());

   ctx->d_ptr->refreshCurrentFbo();

   if (d->blit_fbo) {
      QOpenGLFramebufferObject::blitFramebuffer(d->blit_fbo, d->fbo);
      extensions.glBindFramebuffer(GL_READ_FRAMEBUFFER, d->blit_fbo->handle());
   }

   extensions.glBindTexture(GL_TEXTURE_2D, texture_id);

#ifndef QT_OPENGL_ES
   GLenum format = ctx->contextHandle()->isOpenGLES() ? GL_RGBA : GL_RGBA8;
   extensions.glCopyTexImage2D(GL_TEXTURE_2D, 0, format, 0, 0, d->req_size.width(), d->req_size.height(), 0);
#else
   extensions.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, d->req_size.width(), d->req_size.height(), 0);
#endif

   if (d->blit_fbo) {
      extensions.glBindFramebuffer(GL_READ_FRAMEBUFFER, ctx->d_func()->current_fbo);
   }
}

QSize QGLPixelBuffer::size() const
{
   Q_D(const QGLPixelBuffer);
   return d->req_size;
}

QImage QGLPixelBuffer::toImage() const
{
   Q_D(const QGLPixelBuffer);
   if (d->invalid) {
      return QImage();
   }

   const_cast<QGLPixelBuffer *>(this)->makeCurrent();
   if (d->fbo) {
      d->fbo->bind();
   }

   return cs_glRead_frameBuffer(d->req_size, d->format.alpha(), true);
}

Qt::HANDLE QGLPixelBuffer::handle() const
{
   Q_D(const QGLPixelBuffer);
   if (d->invalid) {
      return nullptr;
   }
   return (Qt::HANDLE) d->pbuf;
}

bool QGLPixelBuffer::isValid() const
{
   Q_D(const QGLPixelBuffer);
   return !d->invalid;
}

static QGLEngineThreadStorage<QGL2PaintEngineEx> *qt_buffer_2_engine()
{
   static QGLEngineThreadStorage<QGL2PaintEngineEx> retval;
   return &retval;
}

QPaintEngine *QGLPixelBuffer::paintEngine() const
{
   return qt_buffer_2_engine()->engine();
}

int QGLPixelBuffer::metric(PaintDeviceMetric metric) const
{
   Q_D(const QGLPixelBuffer);

   float dpmx = qt_defaultDpiX() * 100. / 2.54;
   float dpmy = qt_defaultDpiY() * 100. / 2.54;
   int w = d->req_size.width();
   int h = d->req_size.height();

   switch (metric) {
      case PdmWidth:
         return w;

      case PdmHeight:
         return h;

      case PdmWidthMM:
         return qRound(w * 1000 / dpmx);

      case PdmHeightMM:
         return qRound(h * 1000 / dpmy);

      case PdmNumColors:
         return 0;

      case PdmDepth:
         return 32;//d->depth;

      case PdmDpiX:
         return qRound(dpmx * 0.0254);

      case PdmDpiY:
         return qRound(dpmy * 0.0254);

      case PdmPhysicalDpiX:
         return qRound(dpmx * 0.0254);

      case PdmPhysicalDpiY:
         return qRound(dpmy * 0.0254);

      case QPaintDevice::PdmDevicePixelRatio:
         return 1;

      case QPaintDevice::PdmDevicePixelRatioScaled:
         return QPaintDevice::devicePixelRatioFScale();

      default:
         qWarning("QGLPixelBuffer::metric(), Unhandled metric type: %d\n", metric);
         break;
   }

   return 0;
}

GLuint QGLPixelBuffer::bindTexture(const QImage &image, GLenum target)
{
   Q_D(QGLPixelBuffer);

#ifndef QT_OPENGL_ES
   GLenum format = QOpenGLContext::currentContext()->isOpenGLES() ? GL_RGBA : GL_RGBA8;
   return d->qctx->bindTexture(image, target, GLint(format));
#else
   return d->qctx->bindTexture(image, target, GL_RGBA);
#endif
}

GLuint QGLPixelBuffer::bindTexture(const QPixmap &pixmap, GLenum target)
{
   Q_D(QGLPixelBuffer);

#ifndef QT_OPENGL_ES
   GLenum format = QOpenGLContext::currentContext()->isOpenGLES() ? GL_RGBA : GL_RGBA8;
   return d->qctx->bindTexture(pixmap, target, GLint(format));
#else
   return d->qctx->bindTexture(pixmap, target, GL_RGBA);
#endif
}

GLuint QGLPixelBuffer::bindTexture(const QString &fileName)
{
   Q_D(QGLPixelBuffer);
   return d->qctx->bindTexture(fileName);
}

void QGLPixelBuffer::deleteTexture(GLuint texture_id)
{
   Q_D(QGLPixelBuffer);
   d->qctx->deleteTexture(texture_id);
}

void QGLPixelBuffer::drawTexture(const QRectF &target, GLuint textureId, GLenum textureTarget)
{
   Q_D(QGLPixelBuffer);
   d->qctx->drawTexture(target, textureId, textureTarget);
}

void QGLPixelBuffer::drawTexture(const QPointF &point, GLuint textureId, GLenum textureTarget)
{
   Q_D(QGLPixelBuffer);
   d->qctx->drawTexture(point, textureId, textureTarget);
}
QGLFormat QGLPixelBuffer::format() const
{
   Q_D(const QGLPixelBuffer);
   return d->format;
}

/*! \fn int QGLPixelBuffer::devType() const
    \internal
*/

bool QGLPixelBufferPrivate::init(const QSize &, const QGLFormat &f, QGLWidget *shareWidget)
{
   widget = new QGLWidget(f, nullptr, shareWidget);
   widget->resize(1, 1);
   qctx = const_cast<QGLContext *>(widget->context());
   return widget->isValid();
}

bool QGLPixelBufferPrivate::cleanup()
{
   delete fbo;
   fbo = nullptr;

   delete blit_fbo;
   blit_fbo = nullptr;

   delete widget;
   widget = nullptr;

   return true;
}

bool QGLPixelBuffer::bindToDynamicTexture(GLuint texture_id)
{
   (void) texture_id;
   return false;
}

void QGLPixelBuffer::releaseFromDynamicTexture()
{
}

GLuint QGLPixelBuffer::generateDynamicTexture() const
{
   Q_D(const QGLPixelBuffer);

   if (! d->fbo) {
      return 0;
   }

   if (d->fbo->format().samples() > 0
         && QOpenGLExtensions(QOpenGLContext::currentContext()).hasOpenGLExtension(QOpenGLExtensions::FramebufferBlit)) {

      if (!d->blit_fbo) {
         const_cast<QOpenGLFramebufferObject *&>(d->blit_fbo) = new QOpenGLFramebufferObject(d->req_size);
      }

   } else {
      return d->fbo->texture();
   }

   GLuint texture;
   QOpenGLFunctions *funcs = QOpenGLContext::currentContext()->functions();

   funcs->glGenTextures(1, &texture);
   funcs->glBindTexture(GL_TEXTURE_2D, texture);

   funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, d->req_size.width(), d->req_size.height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

   return texture;
}

bool QGLPixelBuffer::hasOpenGLPbuffers()
{
   return QGLFramebufferObject::hasOpenGLFramebufferObjects();
}
