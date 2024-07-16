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

#include <qopenglwidget.h>

#include <qapplication.h>
#include <qoffscreensurface.h>
#include <qopengl_framebufferobject.h>
#include <qopengl_paintdevice.h>
#include <qopenglcontext.h>
#include <qopenglfunctions.h>
#include <qplatform_window.h>
#include <qplatform_integration.h>
#include <qscreen.h>
#include <qwindow.h>

#include <qguiapplication_p.h>
#include <qfont_p.h>
#include <qopengl_extensions_p.h>
#include <qopengl_paintdevice_p.h>
#include <qopenglcontext_p.h>
#include <qwidget_p.h>

class QOpenGLWidgetPaintDevicePrivate : public QOpenGLPaintDevicePrivate
{
 public:
   QOpenGLWidgetPaintDevicePrivate(QOpenGLWidget *widget)
      : QOpenGLPaintDevicePrivate(QSize()), w(widget)
   { }

   void beginPaint() override;

   QOpenGLWidget *w;
};

class QOpenGLWidgetPaintDevice : public QOpenGLPaintDevice
{
 public:
   QOpenGLWidgetPaintDevice(QOpenGLWidget *widget)
      : QOpenGLPaintDevice(*new QOpenGLWidgetPaintDevicePrivate(widget))
   { }

   void ensureActiveTarget() override;
};

class QOpenGLWidgetPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QOpenGLWidget)

 public:
   QOpenGLWidgetPrivate()
      : context(nullptr), fbo(nullptr), resolvedFbo(nullptr), surface(nullptr), initialized(false),
        fakeHidden(false), inBackingStorePaint(false), hasBeenComposed(false), flushPending(false),
        paintDevice(nullptr), updateBehavior(QOpenGLWidget::NoPartialUpdate), requestedSamples(0), inPaintGL(false)
   {
      requestedFormat = QSurfaceFormat::defaultFormat();
   }

   void reset();
   void recreateFbo();

   GLuint textureId() const override;

   void initialize();
   void invokeUserPaint();
   void render();

   void invalidateFbo();

   QImage grabFramebuffer() override;
   void beginBackingStorePainting() override {
      inBackingStorePaint = true;
   }

   void endBackingStorePainting() override {
      inBackingStorePaint = false;
   }

   void beginCompose() override;
   void endCompose() override;
   void initializeViewportFramebuffer() override;
   void resizeViewportFramebuffer() override;
   void resolveSamples() override;

   QOpenGLContext *context;
   QOpenGLFramebufferObject *fbo;
   QOpenGLFramebufferObject *resolvedFbo;
   QOffscreenSurface *surface;
   bool initialized;
   bool fakeHidden;
   bool inBackingStorePaint;
   bool hasBeenComposed;
   bool flushPending;
   QOpenGLPaintDevice *paintDevice;
   QSurfaceFormat requestedFormat;
   QOpenGLWidget::UpdateBehavior updateBehavior;
   int requestedSamples;
   bool inPaintGL;
};

void QOpenGLWidgetPaintDevicePrivate::beginPaint()
{
   // autoFillBackground is and must be false by default. Otherwise we would clear on
   // every QPainter begin() which is not desirable. This is only for legacy use cases,
   // like using QOpenGLWidget as the viewport of a graphics view, that expect clearing
   // with the palette's background color.

   if (w->autoFillBackground()) {
      QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

      if (w->format().hasAlpha()) {
         f->glClearColor(0, 0, 0, 0);
      } else {
         QColor c = w->palette().brush(w->backgroundRole()).color();
         float alpha = c.alphaF();
         f->glClearColor(c.redF() * alpha, c.greenF() * alpha, c.blueF() * alpha, alpha);
      }

      f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   }
}

void QOpenGLWidgetPaintDevice::ensureActiveTarget()
{
   QOpenGLWidgetPaintDevicePrivate *d = static_cast<QOpenGLWidgetPaintDevicePrivate *>(d_ptr.data());
   QOpenGLWidgetPrivate *wd = static_cast<QOpenGLWidgetPrivate *>(QWidgetPrivate::get(d->w));

   if (! wd->initialized) {
      return;
   }

   if (QOpenGLContext::currentContext() != wd->context) {
      d->w->makeCurrent();

   } else {
      wd->fbo->bind();
   }

   // When used as a viewport, drawing is done via opening a QPainter on the widget
   // without going through paintEvent(). We will have to make sure a glFlush() is done
   // before the texture is accessed also in this case.
   wd->flushPending = true;
}

GLuint QOpenGLWidgetPrivate::textureId() const
{
   return resolvedFbo ? resolvedFbo->texture() : (fbo ? fbo->texture() : 0);
}

void QOpenGLWidgetPrivate::reset()
{
   Q_Q(QOpenGLWidget);

   // Destroy the OpenGL resources first. These need the context to be current.
   if (initialized) {
      q->makeCurrent();
   }

   delete paintDevice;
   paintDevice = nullptr;

   delete fbo;
   fbo = nullptr;

   delete resolvedFbo;
   resolvedFbo = nullptr;

   if (initialized) {
      q->doneCurrent();
   }

   // Delete the context first, then the surface. Slots connected to
   // the context's aboutToBeDestroyed() may still call makeCurrent()
   // to perform some cleanup.
   delete context;
   context = nullptr;

   delete surface;
   surface = nullptr;

   initialized = fakeHidden = inBackingStorePaint = false;
}

void QOpenGLWidgetPrivate::recreateFbo()
{
   Q_Q(QOpenGLWidget);

   emit q->aboutToResize();

   context->makeCurrent(surface);

   delete fbo;
   fbo = nullptr;

   delete resolvedFbo;
   resolvedFbo = nullptr;

   int samples = requestedSamples;
   QOpenGLExtensions *extfuncs = static_cast<QOpenGLExtensions *>(context->functions());
   if (!extfuncs->hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample)) {
      samples = 0;
   }

   QOpenGLFramebufferObjectFormat format;
   format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
   format.setSamples(samples);

   const QSize deviceSize = q->size() * q->devicePixelRatioF();
   fbo = new QOpenGLFramebufferObject(deviceSize, format);
   if (samples > 0) {
      resolvedFbo = new QOpenGLFramebufferObject(deviceSize);
   }

   fbo->bind();
   context->functions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   paintDevice->setSize(deviceSize);
   paintDevice->setDevicePixelRatio(q->devicePixelRatioF());

   emit q->resized();
}

void QOpenGLWidgetPrivate::beginCompose()
{
   Q_Q(QOpenGLWidget);

   if (flushPending) {
      flushPending = false;
      q->makeCurrent();

      static_cast<QOpenGLExtensions *>(context->functions())->flushShared();
   }

   hasBeenComposed = true;
   emit q->aboutToCompose();
}

void QOpenGLWidgetPrivate::endCompose()
{
   Q_Q(QOpenGLWidget);
   emit q->frameSwapped();
}

void QOpenGLWidgetPrivate::initialize()
{
   Q_Q(QOpenGLWidget);

   if (initialized) {
      return;
   }

   // Get our toplevel's context with which we will share in order to make the
   // texture usable by the underlying window's backingstore.
   QWidget *tlw = q->window();
   QOpenGLContext *shareContext = get(tlw)->shareContext();

   if (! shareContext) {
      qWarning("QOpenGLWidget:initialize() Unable to use QOpenGLWidget without a context");
      return;
   }

   // Do not include the sample count. Requesting a multisampled context is not necessary
   // since we render into an FBO, never to an actual surface. What's more, attempting to
   // create a pbuffer with a multisampled config crashes certain implementations. Just
   // avoid the entire hassle, the result is the same.
   requestedSamples = requestedFormat.samples();
   requestedFormat.setSamples(0);

   QScopedPointer<QOpenGLContext> ctx(new QOpenGLContext);
   ctx->setShareContext(shareContext);
   ctx->setFormat(requestedFormat);
   ctx->setScreen(shareContext->screen());

   if (! ctx->create()) {
      qWarning("QOpenGLWidget::initialize() Failed to create an OpenGL context");
      return;
   }

   // Propagate settings that make sense only for the tlw.
   QSurfaceFormat tlwFormat = tlw->windowHandle()->format();

   if (requestedFormat.swapInterval() != tlwFormat.swapInterval()) {
      // Most platforms will pick up the changed swap interval on the next
      // makeCurrent or swapBuffers.
      tlwFormat.setSwapInterval(requestedFormat.swapInterval());
      tlw->windowHandle()->setFormat(tlwFormat);
   }

   if (requestedFormat.swapBehavior() != tlwFormat.swapBehavior()) {
      tlwFormat.setSwapBehavior(requestedFormat.swapBehavior());
      tlw->windowHandle()->setFormat(tlwFormat);
   }

   // The top-level window's surface is not good enough since it causes way too
   // much trouble with regards to the QSurfaceFormat for example. So just like
   // in QQuickWidget, use a dedicated QOffscreenSurface.
   surface = new QOffscreenSurface;
   surface->setFormat(ctx->format());
   surface->setScreen(ctx->screen());
   surface->create();

   if (! ctx->makeCurrent(surface)) {
      qWarning("QOpenGLWidget::initialize() Failed to make the OpenGL context current");
      return;
   }

   paintDevice = new QOpenGLWidgetPaintDevice(q);
   paintDevice->setSize(q->size() * q->devicePixelRatioF());
   paintDevice->setDevicePixelRatio(q->devicePixelRatioF());

   context = ctx.take();
   initialized = true;

   q->initializeGL();
}

void QOpenGLWidgetPrivate::resolveSamples()
{
   Q_Q(QOpenGLWidget);

   if (resolvedFbo) {
      q->makeCurrent();

      QRect rect(QPoint(0, 0), fbo->size());
      QOpenGLFramebufferObject::blitFramebuffer(resolvedFbo, rect, fbo, rect);
      flushPending = true;
   }
}

void QOpenGLWidgetPrivate::invokeUserPaint()
{
   Q_Q(QOpenGLWidget);

   QOpenGLContext *ctx = QOpenGLContext::currentContext();
   Q_ASSERT(ctx && fbo);

   QOpenGLFunctions *f = ctx->functions();
   QOpenGLContextPrivate::get(ctx)->defaultFboRedirect = fbo->handle();

   f->glViewport(0, 0, q->width() * q->devicePixelRatioF(), q->height() * q->devicePixelRatioF());
   inPaintGL = true;

   q->paintGL();

   inPaintGL    = false;
   flushPending = true;

   QOpenGLContextPrivate::get(ctx)->defaultFboRedirect = 0;
}

void QOpenGLWidgetPrivate::render()
{
   Q_Q(QOpenGLWidget);

   if (fakeHidden || ! initialized) {
      return;
   }

   q->makeCurrent();

   if (updateBehavior == QOpenGLWidget::NoPartialUpdate && hasBeenComposed) {
      invalidateFbo();
      hasBeenComposed = false;
   }

   invokeUserPaint();
}

void QOpenGLWidgetPrivate::invalidateFbo()
{
   QOpenGLExtensions *f = static_cast<QOpenGLExtensions *>(QOpenGLContext::currentContext()->functions());
   if (f->hasOpenGLExtension(QOpenGLExtensions::DiscardFramebuffer)) {
      const int gl_color_attachment0  = 0x8CE0;  // GL_COLOR_ATTACHMENT0
      const int gl_depth_attachment   = 0x8D00;  // GL_DEPTH_ATTACHMENT
      const int gl_stencil_attachment = 0x8D20;  // GL_STENCIL_ATTACHMENT

      const GLenum attachments[] = {
         gl_color_attachment0, gl_depth_attachment, gl_stencil_attachment
      };

      f->glDiscardFramebufferEXT(GL_FRAMEBUFFER, sizeof attachments / sizeof * attachments, attachments);

   } else {
      f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   }
}

extern Q_GUI_EXPORT QImage qt_gl_read_framebuffer(const QSize &size, bool alpha_format, bool include_alpha);

QImage QOpenGLWidgetPrivate::grabFramebuffer()
{
   Q_Q(QOpenGLWidget);

   if (! initialized) {
      return QImage();
   }

   if (! inPaintGL) {
      render();
   }

   if (resolvedFbo) {
      resolveSamples();
      resolvedFbo->bind();

   } else {
      q->makeCurrent();
   }

   QImage res = qt_gl_read_framebuffer(q->size() * q->devicePixelRatioF(), false, false);
   res.setDevicePixelRatio(q->devicePixelRatioF());

   // While we give no guarantees of what is going to be left bound, prefer the
   // multisample fbo instead of the resolved one. Clients may continue to
   // render straight after calling this function.
   if (resolvedFbo) {
      q->makeCurrent();
   }

   return res;
}

void QOpenGLWidgetPrivate::initializeViewportFramebuffer()
{
   Q_Q(QOpenGLWidget);

   // Legacy behavior for compatibility with QGLWidget when used as a graphics view
   // viewport: enable clearing on each painter begin.

   q->setAutoFillBackground(true);
}

void QOpenGLWidgetPrivate::resizeViewportFramebuffer()
{
   Q_Q(QOpenGLWidget);

   if (! initialized) {
      return;
   }

   if (! fbo || q->size() * q->devicePixelRatioF() != fbo->size()) {
      recreateFbo();
      q->update();
   }
}

QOpenGLWidget::QOpenGLWidget(QWidget *parent, Qt::WindowFlags flags)
   : QWidget(*(new QOpenGLWidgetPrivate), parent, flags)
{
   Q_D(QOpenGLWidget);

   if (QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::RasterGLSurface)) {
      d->setRenderToTexture();
   } else {
      qWarning("QOpenGLWidget() Class not supported on this platform");
   }
}

QOpenGLWidget::~QOpenGLWidget()
{
   Q_D(QOpenGLWidget);
   d->reset();
}

void QOpenGLWidget::setUpdateBehavior(UpdateBehavior updateBehavior)
{
   Q_D(QOpenGLWidget);
   d->updateBehavior = updateBehavior;
}

QOpenGLWidget::UpdateBehavior QOpenGLWidget::updateBehavior() const
{
   Q_D(const QOpenGLWidget);
   return d->updateBehavior;
}

void QOpenGLWidget::setFormat(const QSurfaceFormat &format)
{
   Q_D(QOpenGLWidget);

   if (d->initialized) {
      (void) format;

      qWarning("QOpenGLWidget::setFormat() Already initialized, setting the format again has no effect");
      return;
   }

   d->requestedFormat = format;
}

QSurfaceFormat QOpenGLWidget::format() const
{
   Q_D(const QOpenGLWidget);
   return d->initialized ? d->context->format() : d->requestedFormat;
}

bool QOpenGLWidget::isValid() const
{
   Q_D(const QOpenGLWidget);
   return d->initialized && d->context->isValid();
}

void QOpenGLWidget::makeCurrent()
{
   Q_D(QOpenGLWidget);

   if (! d->initialized) {
      return;
   }

   d->context->makeCurrent(d->surface);

   if (d->fbo) {
      // there may not be one if we are in reset()
      d->fbo->bind();
   }
}

void QOpenGLWidget::doneCurrent()
{
   Q_D(QOpenGLWidget);

   if (! d->initialized) {
      return;
   }

   d->context->doneCurrent();
}

QOpenGLContext *QOpenGLWidget::context() const
{
   Q_D(const QOpenGLWidget);
   return d->context;
}

GLuint QOpenGLWidget::defaultFramebufferObject() const
{
   Q_D(const QOpenGLWidget);
   return d->fbo ? d->fbo->handle() : 0;
}

void QOpenGLWidget::initializeGL()
{
}

void QOpenGLWidget::resizeGL(int w, int h)
{
   (void) w;
   (void) h;
}

void QOpenGLWidget::paintGL()
{ }

void QOpenGLWidget::resizeEvent(QResizeEvent *e)
{
   Q_D(QOpenGLWidget);

   if (e->size().isEmpty()) {
      d->fakeHidden = true;
      return;
   }
   d->fakeHidden = false;

   d->initialize();
   if (!d->initialized) {
      return;
   }

   d->recreateFbo();
   resizeGL(width(), height());
   d->sendPaintEvent(QRect(QPoint(0, 0), size()));
}

void QOpenGLWidget::paintEvent(QPaintEvent *e)
{
   (void) e;

   Q_D(QOpenGLWidget);

   if (! d->initialized) {
      return;
   }

   if (updatesEnabled()) {
      d->render();
   }
}

QImage QOpenGLWidget::grabFramebuffer()
{
   Q_D(QOpenGLWidget);
   return d->grabFramebuffer();
}

int QOpenGLWidget::metric(QPaintDevice::PaintDeviceMetric metric) const
{
   Q_D(const QOpenGLWidget);

   if (d->inBackingStorePaint) {
      return QWidget::metric(metric);
   }

   QWidget *tlw = window();
   QWindow *window = tlw ? tlw->windowHandle() : nullptr;
   QScreen *screen = tlw && tlw->windowHandle() ? tlw->windowHandle()->screen() : nullptr;

   if (! screen && QGuiApplication::primaryScreen()) {
      screen = QGuiApplication::primaryScreen();
   }

   const float dpmx = qt_defaultDpiX() * 100. / 2.54;
   const float dpmy = qt_defaultDpiY() * 100. / 2.54;

   switch (metric) {

      case PdmWidth:
         return width();

      case PdmHeight:
         return height();

      case PdmDepth:
         return 32;

      case PdmWidthMM:
         if (screen) {
            return width() * screen->physicalSize().width() / screen->geometry().width();
         } else {
            return width() * 1000 / dpmx;
         }

      case PdmHeightMM:
         if (screen) {
            return height() * screen->physicalSize().height() / screen->geometry().height();
         } else {
            return height() * 1000 / dpmy;
         }

      case PdmNumColors:
         return 0;

      case PdmDpiX:
         if (screen) {
            return qRound(screen->logicalDotsPerInchX());
         } else {
            return qRound(dpmx * 0.0254);
         }

      case PdmDpiY:
         if (screen) {
            return qRound(screen->logicalDotsPerInchY());
         } else {
            return qRound(dpmy * 0.0254);
         }

      case PdmPhysicalDpiX:
         if (screen) {
            return qRound(screen->physicalDotsPerInchX());
         } else {
            return qRound(dpmx * 0.0254);
         }

      case PdmPhysicalDpiY:
         if (screen) {
            return qRound(screen->physicalDotsPerInchY());
         } else {
            return qRound(dpmy * 0.0254);
         }

      case PdmDevicePixelRatio:
         if (window) {
            return int(window->devicePixelRatio());
         } else {
            return 1.0;
         }

      case PdmDevicePixelRatioScaled:
         if (window) {
            return int(window->devicePixelRatio() * devicePixelRatioFScale());
         } else {
            return 1.0;
         }

      default:
         qWarning("QOpenGLWidget::metric() Unknown metric %d", metric);
         return 0;
   }
}

// internal
QPaintDevice *QOpenGLWidget::redirected(QPoint *p) const
{
   Q_D(const QOpenGLWidget);
   if (d->inBackingStorePaint) {
      return QWidget::redirected(p);
   }

   return d->paintDevice;
}

// internal
QPaintEngine *QOpenGLWidget::paintEngine() const
{
   Q_D(const QOpenGLWidget);
   // QWidget needs to "punch a hole" into the backingstore. This needs the
   // normal paint engine and device, not the GL one. So in this mode, behave
   // like a normal widget.
   if (d->inBackingStorePaint) {
      return QWidget::paintEngine();
   }

   if (! d->initialized) {
      return nullptr;
   }

   return d->paintDevice->paintEngine();
}

// internal
bool QOpenGLWidget::event(QEvent *e)
{
   Q_D(QOpenGLWidget);

   switch (e->type()) {
      case QEvent::WindowChangeInternal:
         if (qGuiApp->testAttribute(Qt::AA_ShareOpenGLContexts)) {
            break;
         }

         if (d->initialized) {
            d->reset();
         }
         [[fallthrough]];

      case QEvent::Show: // reparenting may not lead to a resize so reinitalize on Show too
         if (! d->initialized && !size().isEmpty() && window() && window()->windowHandle()) {
            d->initialize();

            if (d->initialized) {
               d->recreateFbo();
            }
         }
         break;

      case QEvent::ScreenChangeInternal:
         if (d->initialized && d->paintDevice->devicePixelRatioF() != devicePixelRatioF()) {
            d->recreateFbo();
         }
         break;

      default:
         break;
   }

   return QWidget::event(e);
}
