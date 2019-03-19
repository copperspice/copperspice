/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qglpaintdevice_p.h>
#include <qgl_p.h>
#include <qglpixelbuffer_p.h>
#include <qglframebufferobject_p.h>

#ifdef Q_WS_X11
#include <qpixmapdata_x11gl_p.h>
#endif

#if !defined(QT_OPENGL_ES_1)
#include <qpixmapdata_gl_p.h>
#include <qwindowsurface_gl_p.h>
#endif

QT_BEGIN_NAMESPACE

QGLPaintDevice::QGLPaintDevice()
   : m_thisFBO(0)
{
}

QGLPaintDevice::~QGLPaintDevice()
{
}

int QGLPaintDevice::metric(QPaintDevice::PaintDeviceMetric metric) const
{
   switch (metric) {
      case PdmWidth:
         return size().width();
      case PdmHeight:
         return size().height();
      case PdmDepth: {
         const QGLFormat f = format();
         return f.redBufferSize() + f.greenBufferSize() + f.blueBufferSize() + f.alphaBufferSize();
      }
      default:
         qWarning("QGLPaintDevice::metric() - metric %d not known", metric);
         return 0;
   }
}

void QGLPaintDevice::beginPaint()
{
   // Make sure our context is the current one:
   QGLContext *ctx = context();
   if (ctx != QGLContext::currentContext()) {
      ctx->makeCurrent();
   }

   // Record the currently bound FBO so we can restore it again
   // in endPaint() and bind this device's FBO
   //
   // Note: m_thisFBO could be zero if the paint device is not
   // backed by an FBO (e.g. window back buffer).  But there could
   // be a previous FBO bound to the context which we need to
   // explicitly unbind.  Otherwise the painting will go into
   // the previous FBO instead of to the window.
   m_previousFBO = ctx->d_func()->current_fbo;

   if (m_previousFBO != m_thisFBO) {
      ctx->d_ptr->current_fbo = m_thisFBO;
      glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_thisFBO);
   }

   // Set the default fbo for the context to m_thisFBO so that
   // if some raw GL code between beginNativePainting() and
   // endNativePainting() calls QGLFramebufferObject::release(),
   // painting will revert to the window surface's fbo.
   ctx->d_ptr->default_fbo = m_thisFBO;
}

void QGLPaintDevice::ensureActiveTarget()
{
   QGLContext *ctx = context();
   if (ctx != QGLContext::currentContext()) {
      ctx->makeCurrent();
   }

   if (ctx->d_ptr->current_fbo != m_thisFBO) {
      ctx->d_ptr->current_fbo = m_thisFBO;
      glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_thisFBO);
   }

   ctx->d_ptr->default_fbo = m_thisFBO;
}

void QGLPaintDevice::endPaint()
{
   // Make sure the FBO bound at beginPaint is re-bound again here:
   QGLContext *ctx = context();
   if (m_previousFBO != ctx->d_func()->current_fbo) {
      ctx->d_ptr->current_fbo = m_previousFBO;
      glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_previousFBO);
   }

   ctx->d_ptr->default_fbo = 0;
}

QGLFormat QGLPaintDevice::format() const
{
   return context()->format();
}

bool QGLPaintDevice::alphaRequested() const
{
   return context()->d_func()->reqFormat.alpha();
}

bool QGLPaintDevice::isFlipped() const
{
   return false;
}

////////////////// QGLWidgetGLPaintDevice //////////////////

QGLWidgetGLPaintDevice::QGLWidgetGLPaintDevice()
{
}

QPaintEngine *QGLWidgetGLPaintDevice::paintEngine() const
{
   return glWidget->paintEngine();
}

void QGLWidgetGLPaintDevice::setWidget(QGLWidget *w)
{
   glWidget = w;
}

void QGLWidgetGLPaintDevice::beginPaint()
{
   QGLPaintDevice::beginPaint();
   if (!glWidget->d_func()->disable_clear_on_painter_begin && glWidget->autoFillBackground()) {
      if (glWidget->testAttribute(Qt::WA_TranslucentBackground)) {
         glClearColor(0.0, 0.0, 0.0, 0.0);
      } else {
         const QColor &c = glWidget->palette().brush(glWidget->backgroundRole()).color();
         float alpha = c.alphaF();
         glClearColor(c.redF() * alpha, c.greenF() * alpha, c.blueF() * alpha, alpha);
      }
      if (context()->d_func()->workaround_needsFullClearOnEveryFrame) {
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      } else {
         glClear(GL_COLOR_BUFFER_BIT);
      }
   }
}

void QGLWidgetGLPaintDevice::endPaint()
{
   if (glWidget->autoBufferSwap()) {
      glWidget->swapBuffers();
   }
   QGLPaintDevice::endPaint();
}


QSize QGLWidgetGLPaintDevice::size() const
{
   return glWidget->size();
}

QGLContext *QGLWidgetGLPaintDevice::context() const
{
   return const_cast<QGLContext *>(glWidget->context());
}

// returns the QGLPaintDevice for the given QPaintDevice
QGLPaintDevice *QGLPaintDevice::getDevice(QPaintDevice *pd)
{
   QGLPaintDevice *glpd = 0;

   switch (pd->devType()) {
      case QInternal::Widget:
         // Should not be called on a non-gl widget:
         Q_ASSERT(qobject_cast<QGLWidget *>(static_cast<QWidget *>(pd)));
         glpd = &(static_cast<QGLWidget *>(pd)->d_func()->glDevice);
         break;
      case QInternal::Pbuffer:
         glpd = &(static_cast<QGLPixelBuffer *>(pd)->d_func()->glDevice);
         break;
      case QInternal::FramebufferObject:
         glpd = &(static_cast<QGLFramebufferObject *>(pd)->d_func()->glDevice);
         break;
      case QInternal::Pixmap: {
#if !defined(QT_OPENGL_ES_1)
         QPixmapData *pmd = static_cast<QPixmap *>(pd)->pixmapData();
         if (pmd->classId() == QPixmapData::OpenGLClass) {
            glpd = static_cast<QGLPixmapData *>(pmd)->glDevice();
         }
#ifdef Q_WS_X11
         else if (pmd->classId() == QPixmapData::X11Class) {
            glpd = static_cast<QX11GLPixmapData *>(pmd);
         }
#endif
         else {
            qWarning("Pixmap type not supported for GL rendering");
         }
#else
         qWarning("Pixmap render targets not supported on OpenGL ES 1.x");
#endif
         break;
      }
      default:
         qWarning("QGLPaintDevice::getDevice() - Unknown device type %d", pd->devType());
         break;
   }

   return glpd;
}

QT_END_NAMESPACE
