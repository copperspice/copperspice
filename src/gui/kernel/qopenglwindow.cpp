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

#include <qopenglwindow.h>
#include <qpaintdevicewindow_p.h>

#include <qopengl_framebufferobject.h>
#include <qopengl_paintdevice.h>
#include <qopenglfunctions.h>
#include <qmatrix4x4.h>
#include <qoffscreensurface.h>

#include <qopengl_textureblitter_p.h>
#include <qopengl_extensions_p.h>
#include <qopenglcontext_p.h>

// GLES2 builds will not have these constants with the suffixless names
#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif

class QOpenGLWindowPaintDevice : public QOpenGLPaintDevice
{
 public:
   QOpenGLWindowPaintDevice(QOpenGLWindow *window) : m_window(window) { }
   void ensureActiveTarget() override;

   QOpenGLWindow *m_window;
};

class QOpenGLWindowPrivate : public QPaintDeviceWindowPrivate
{
   Q_DECLARE_PUBLIC(QOpenGLWindow)

 public:
   QOpenGLWindowPrivate(QOpenGLContext *shareContext, QOpenGLWindow::UpdateBehavior updateBehavior)
      : updateBehavior(updateBehavior)
      , hasFboBlit(false)
      , shareContext(shareContext) {
      if (!shareContext) {
         this->shareContext = qt_gl_global_share_context();
      }
   }

   ~QOpenGLWindowPrivate();

   static QOpenGLWindowPrivate *get(QOpenGLWindow *w) {
      return w->d_func();
   }

   void bindFBO();
   void initialize();

   void beginPaint(const QRegion &region) override;
   void endPaint() override;
   void flush(const QRegion &region) override;

   QOpenGLWindow::UpdateBehavior updateBehavior;
   bool hasFboBlit;
   QScopedPointer<QOpenGLContext> context;
   QOpenGLContext *shareContext;
   QScopedPointer<QOpenGLFramebufferObject> fbo;
   QScopedPointer<QOpenGLWindowPaintDevice> paintDevice;
   QOpenGLTextureBlitter blitter;
   QColor backgroundColor;
   QScopedPointer<QOffscreenSurface> offscreenSurface;
};

QOpenGLWindowPrivate::~QOpenGLWindowPrivate()
{
   Q_Q(QOpenGLWindow);
   if (q->isValid()) {
      q->makeCurrent(); // this works even when the platformwindow is destroyed
      paintDevice.reset(nullptr);
      fbo.reset(nullptr);
      blitter.destroy();
      q->doneCurrent();
   }
}

void QOpenGLWindowPrivate::initialize()
{
   Q_Q(QOpenGLWindow);

   if (context) {
      return;
   }

   context.reset(new QOpenGLContext);
   context->setShareContext(shareContext);
   context->setFormat(q->requestedFormat());

   if (!context->create()) {
      qWarning("QOpenGLWindow::initialize() Failed to create context");
   }

   if (!context->makeCurrent(q)) {
      qWarning("QOpenGLWindow::initialize() Failed to make context current");
   }

   paintDevice.reset(new QOpenGLWindowPaintDevice(q));
   if (updateBehavior == QOpenGLWindow::PartialUpdateBlit) {
      hasFboBlit = QOpenGLFramebufferObject::hasOpenGLFramebufferBlit();
   }

   q->initializeGL();
}

void QOpenGLWindowPrivate::beginPaint(const QRegion &region)
{
   (void) region;

   Q_Q(QOpenGLWindow);

   initialize();
   context->makeCurrent(q);

   const int deviceWidth  = q->width()  * q->devicePixelRatio();
   const int deviceHeight = q->height() * q->devicePixelRatio();
   const QSize deviceSize(deviceWidth, deviceHeight);

   if (updateBehavior > QOpenGLWindow::NoPartialUpdate) {
      if (!fbo || fbo->size() != deviceSize) {
         QOpenGLFramebufferObjectFormat fboFormat;
         fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
         if (q->requestedFormat().samples() > 0) {
            if (updateBehavior != QOpenGLWindow::PartialUpdateBlend) {
               fboFormat.setSamples(q->requestedFormat().samples());
            } else {
               qWarning("QOpenGLWindow::beginPaint() PartialUpdateBlend does not support multisampling");
            }
         }
         fbo.reset(new QOpenGLFramebufferObject(deviceSize, fboFormat));
         markWindowAsDirty();
      }
   } else {
      markWindowAsDirty();
   }

   paintDevice->setSize(QSize(deviceWidth, deviceHeight));
   paintDevice->setDevicePixelRatio(q->devicePixelRatio());
   context->functions()->glViewport(0, 0, deviceWidth, deviceHeight);

   context->functions()->glBindFramebuffer(GL_FRAMEBUFFER, context->defaultFramebufferObject());

   q->paintUnderGL();

   if (updateBehavior > QOpenGLWindow::NoPartialUpdate) {
      fbo->bind();
   }
}

void QOpenGLWindowPrivate::endPaint()
{
   Q_Q(QOpenGLWindow);

   if (updateBehavior > QOpenGLWindow::NoPartialUpdate) {
      fbo->release();
   }

   context->functions()->glBindFramebuffer(GL_FRAMEBUFFER, context->defaultFramebufferObject());

   if (updateBehavior == QOpenGLWindow::PartialUpdateBlit && hasFboBlit) {
      const int deviceWidth = q->width() * q->devicePixelRatio();
      const int deviceHeight = q->height() * q->devicePixelRatio();
      QOpenGLExtensions extensions(context.data());
      extensions.glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->handle());
      extensions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context->defaultFramebufferObject());
      extensions.glBlitFramebuffer(0, 0, deviceWidth, deviceHeight,
         0, 0, deviceWidth, deviceHeight,
         GL_COLOR_BUFFER_BIT, GL_NEAREST);
   } else if (updateBehavior > QOpenGLWindow::NoPartialUpdate) {
      if (updateBehavior == QOpenGLWindow::PartialUpdateBlend) {
         context->functions()->glEnable(GL_BLEND);
         context->functions()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      if (!blitter.isCreated()) {
         blitter.create();
      }

      QRect windowRect(QPoint(0, 0), fbo->size());
      QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(windowRect, windowRect);
      blitter.bind();
      blitter.blit(fbo->texture(), target, QOpenGLTextureBlitter::OriginBottomLeft);
      blitter.release();

      if (updateBehavior == QOpenGLWindow::PartialUpdateBlend) {
         context->functions()->glDisable(GL_BLEND);
      }
   }

   q->paintOverGL();
}

void QOpenGLWindowPrivate::bindFBO()
{
   if (updateBehavior > QOpenGLWindow::NoPartialUpdate) {
      fbo->bind();
   } else {
      QOpenGLFramebufferObject::bindDefault();
   }
}

void QOpenGLWindowPrivate::flush(const QRegion &region)
{
   (void) region;

   Q_Q(QOpenGLWindow);

   context->swapBuffers(q);
   emit q->frameSwapped();
}

void QOpenGLWindowPaintDevice::ensureActiveTarget()
{
   QOpenGLWindowPrivate::get(m_window)->bindFBO();
}

QOpenGLWindow::QOpenGLWindow(QOpenGLWindow::UpdateBehavior updateBehavior, QWindow *parent)
   : QPaintDeviceWindow(*(new QOpenGLWindowPrivate(nullptr, updateBehavior)), parent)
{
   setSurfaceType(QSurface::OpenGLSurface);
}

QOpenGLWindow::QOpenGLWindow(QOpenGLContext *shareContext, UpdateBehavior updateBehavior, QWindow *parent)
   : QPaintDeviceWindow(*(new QOpenGLWindowPrivate(shareContext, updateBehavior)), parent)
{
   setSurfaceType(QSurface::OpenGLSurface);
}

QOpenGLWindow::~QOpenGLWindow()
{
   makeCurrent();
}

QOpenGLWindow::UpdateBehavior QOpenGLWindow::updateBehavior() const
{
   Q_D(const QOpenGLWindow);
   return d->updateBehavior;
}

bool QOpenGLWindow::isValid() const
{
   Q_D(const QOpenGLWindow);
   return d->context && d->context->isValid();
}

void QOpenGLWindow::makeCurrent()
{
   Q_D(QOpenGLWindow);

   if (!isValid()) {
      return;
   }

   // The platform window may be destroyed at this stage and therefore
   // makeCurrent() may not safely be called with 'this'.
   if (handle()) {
      d->context->makeCurrent(this);
   } else {
      if (!d->offscreenSurface) {
         d->offscreenSurface.reset(new QOffscreenSurface);
         d->offscreenSurface->setFormat(d->context->format());
         d->offscreenSurface->create();
      }
      d->context->makeCurrent(d->offscreenSurface.data());
   }

   d->bindFBO();
}

void QOpenGLWindow::doneCurrent()
{
   Q_D(QOpenGLWindow);

   if (!isValid()) {
      return;
   }

   d->context->doneCurrent();
}

/*!
  \return The QOpenGLContext used by this window or \c 0 if not yet initialized.
 */
QOpenGLContext *QOpenGLWindow::context() const
{
   Q_D(const QOpenGLWindow);
   return d->context.data();
}

/*!
  \return The QOpenGLContext requested to be shared with this window's QOpenGLContext.
*/
QOpenGLContext *QOpenGLWindow::shareContext() const
{
   Q_D(const QOpenGLWindow);
   return d->shareContext;
}

GLuint QOpenGLWindow::defaultFramebufferObject() const
{
   Q_D(const QOpenGLWindow);

   if (d->updateBehavior > NoPartialUpdate && d->fbo) {
      return d->fbo->handle();

   } else if (QOpenGLContext *ctx = QOpenGLContext::currentContext()) {
      return ctx->defaultFramebufferObject();

   } else {
      return 0;

   }
}

extern Q_GUI_EXPORT QImage qt_gl_read_framebuffer(const QSize &size, bool alpha_format, bool include_alpha);

QImage QOpenGLWindow::grabFramebuffer()
{
   if (! isValid()) {
      return QImage();
   }

   makeCurrent();

   return qt_gl_read_framebuffer(size() * devicePixelRatio(), false, false);
}

void QOpenGLWindow::initializeGL()
{
}

void QOpenGLWindow::resizeGL(int w, int h)
{
   (void) w;
   (void) h;
}

void QOpenGLWindow::paintGL()
{
}

void QOpenGLWindow::paintUnderGL()
{
}

void QOpenGLWindow::paintOverGL()
{
}

void QOpenGLWindow::paintEvent(QPaintEvent *event)
{
   (void) event;

   paintGL();
}

void QOpenGLWindow::resizeEvent(QResizeEvent *event)
{
   (void) event;

   Q_D(QOpenGLWindow);

   d->initialize();
   resizeGL(width(), height());
}

/*!
  \internal
 */
int QOpenGLWindow::metric(PaintDeviceMetric metric) const
{
   Q_D(const QOpenGLWindow);

   switch (metric) {
      case PdmDepth:
         if (d->paintDevice) {
            return d->paintDevice->depth();
         }
         break;
      default:
         break;
   }
   return QPaintDeviceWindow::metric(metric);
}

/*!
  \internal
 */
QPaintDevice *QOpenGLWindow::redirected(QPoint *) const
{
   Q_D(const QOpenGLWindow);
   if (QOpenGLContext::currentContext() == d->context.data()) {
      return d->paintDevice.data();
   }
   return nullptr;
}


