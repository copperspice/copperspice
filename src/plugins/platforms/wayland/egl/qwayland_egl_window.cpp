/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qwayland_egl_window.h>

#include <qopengl_framebufferobject.h>
#include <qwindow.h>

#include <qwayland_display_p.h>
#include <qwayland_egl_config_p.h>
#include <qwayland_screen_p.h>

namespace QtWaylandClient {

QWaylandEglWindow::QWaylandEglWindow(QWindow *window)
   : QWaylandWindow(window), m_waylandEglWindow(nullptr), m_eglSurface(nullptr),
     m_clientBufferIntegration(static_cast<QWaylandEglClientBufferIntegration *>(m_display->clientBufferIntegration())),
     m_resize(false), m_contentFBO(nullptr)
{
   // unused: m_parentWindow(nullptr)

   QSurfaceFormat fmt = window->requestedFormat();

   if (m_display->supportsWindowDecoration()) {
      fmt.setAlphaBufferSize(8);
   }

   m_eglConfig = q_configFromGLFormat(m_clientBufferIntegration->eglDisplay(), fmt);
   m_format    = q_glFormatFromConfig(m_clientBufferIntegration->eglDisplay(), m_eglConfig);

   // Do not create anything here. Platform window may belong to a RasterGLSurface window
   // which could have pure raster content. In this case, if the window never becomes
   // current, creating a wl_egl_window and EGL surface should be avoided
}

QWaylandEglWindow::~QWaylandEglWindow()
{
   if (m_eglSurface != nullptr) {
      eglDestroySurface(m_clientBufferIntegration->eglDisplay(), m_eglSurface);
      m_eglSurface = nullptr;
   }

   if (m_waylandEglWindow != nullptr) {
      wl_egl_window_destroy(m_waylandEglWindow);
   }

   delete m_contentFBO;
}

void QWaylandEglWindow::bindContentFBO()
{
   if (decoration() != nullptr) {
      contentFBO();
      m_contentFBO->bind();
   }
}

GLuint QWaylandEglWindow::contentFBO() const
{
   if (decoration() == nullptr) {
      return 0;
   }

   if (m_resize || ! m_contentFBO) {
      QOpenGLFramebufferObject *old = m_contentFBO;

      QSize fboSize = geometry().size() * scale();
      m_contentFBO  = new QOpenGLFramebufferObject(fboSize.width(), fboSize.height(), QOpenGLFramebufferObject::CombinedDepthStencil);

      delete old;
      m_resize = false;
   }

   return m_contentFBO->handle();
}

GLuint QWaylandEglWindow::contentTexture() const
{
   return m_contentFBO->texture();
}

QRect QWaylandEglWindow::contentsRect() const
{
   QRect r    = geometry();
   QMargins m = frameMargins();

   return QRect(m.left(), m.bottom(), r.width(), r.height());
}

EGLSurface QWaylandEglWindow::eglSurface() const
{
   return m_eglSurface;
}

QSurfaceFormat QWaylandEglWindow::format() const
{
   return m_format;
}

void QWaylandEglWindow::invalidateSurface()
{
   if (m_eglSurface) {
      eglDestroySurface(m_clientBufferIntegration->eglDisplay(), m_eglSurface);
      m_eglSurface = nullptr;
   }
}

void QWaylandEglWindow::setGeometry(const QRect &rect)
{
   QWaylandWindow::setGeometry(rect);

   // If the surface was invalidated through invalidateSurface() and we are now getting a resize,
   // then do not want to create it again. Instead resize the wl_egl_window and the EGLSurface
   // will be created the next time makeCurrent is called.

   updateSurface(false);
}

void QWaylandEglWindow::setVisible(bool visible)
{
   QWaylandWindow::setVisible(visible);

   if (! visible) {
      invalidateSurface();
   }
}

void QWaylandEglWindow::updateSurface(bool create)
{
   QMargins margins = frameMargins();
   QRect rect       = geometry();

   QSize sizeWithMargins = (rect.size() + QSize(margins.left() + margins.right(), margins.top() + margins.bottom())) * scale();

   // wl_egl_windows must have both width and height > 0
   // egl on Mesa returns a nullptr if an invalid wl_egl_window is created
   // not all EGL implementations will do this.

   // handle resizing a valid window to 0x0 by destroying it

   if (sizeWithMargins.isEmpty()) {
      if (m_eglSurface) {
         eglDestroySurface(m_clientBufferIntegration->eglDisplay(), m_eglSurface);
         m_eglSurface = nullptr;
      }

      if (m_waylandEglWindow) {
         wl_egl_window_destroy(m_waylandEglWindow);
         m_waylandEglWindow = nullptr;
      }

      m_offset = QPoint();

   } else {
      if (m_waylandEglWindow != nullptr) {
         int current_width, current_height;
         wl_egl_window_get_attached_size(m_waylandEglWindow, &current_width, &current_height);

         if (current_width != sizeWithMargins.width() || current_height != sizeWithMargins.height()) {
            wl_egl_window_resize(m_waylandEglWindow, sizeWithMargins.width(), sizeWithMargins.height(), m_offset.x(), m_offset.y());
            m_offset = QPoint();

            m_resize = true;
         }

      } else if (create) {
         m_waylandEglWindow = wl_egl_window_create(object(), sizeWithMargins.width(), sizeWithMargins.height());
      }

      if (m_eglSurface == nullptr && create) {
         EGLNativeWindowType eglw = (EGLNativeWindowType)m_waylandEglWindow;
         m_eglSurface = eglCreateWindowSurface(m_clientBufferIntegration->eglDisplay(), m_eglConfig, eglw, nullptr);
      }
   }
}

QWaylandWindow::WindowType QWaylandEglWindow::windowType() const
{
   return QWaylandWindow::Egl;
}

}
