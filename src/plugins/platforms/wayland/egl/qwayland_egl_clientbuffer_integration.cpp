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

#include <qwayland_egl_clientbuffer_integration.h>

#include <qdebug.h>
#include <qwayland_egl_window.h>
#include <qwayland_gl_context.h>

#include <wayland-client.h>

#include <qwayland_display_p.h>
#include <qwayland_window_p.h>

namespace QtWaylandClient {

QWaylandEglClientBufferIntegration::QWaylandEglClientBufferIntegration()
   : m_display(nullptr), m_eglDisplay(EGL_NO_DISPLAY), m_supportsThreading(false)
{
#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug("Using Wayland EGL");
#endif
}

QWaylandEglClientBufferIntegration::~QWaylandEglClientBufferIntegration()
{
   eglTerminate(m_eglDisplay);
}

void QWaylandEglClientBufferIntegration::initialize(QWaylandDisplay *display)
{
   QByteArray eglPlatform = qgetenv("EGL_PLATFORM");

   if (eglPlatform.isEmpty()) {
      setenv("EGL_PLATFORM", "wayland", true);
   }

   m_display = display;

   EGLint major;
   EGLint minor;

   m_eglDisplay = eglGetDisplay((EGLNativeDisplayType) display->wl_display());

   if (m_eglDisplay == EGL_NO_DISPLAY) {
      qWarning("EGL not available");
      return;
   }

   if (! eglInitialize(m_eglDisplay, &major, &minor)) {
      qWarning("Failed to initialize EGL display");
      m_eglDisplay = EGL_NO_DISPLAY;
      return;
   }

   m_supportsThreading = true;

   if (! qgetenv("QT_OPENGL_NO_SANITY_CHECK").isEmpty()) {
      return;
   }

}

bool QWaylandEglClientBufferIntegration::isValid() const
{
   return m_eglDisplay != EGL_NO_DISPLAY;
}

bool QWaylandEglClientBufferIntegration::supportsThreadedOpenGL() const
{
   return m_supportsThreading;
}

bool QWaylandEglClientBufferIntegration::supportsWindowDecoration() const
{
   return true;
}

QWaylandWindow *QWaylandEglClientBufferIntegration::createEglWindow(QWindow *window)
{
   return new QWaylandEglWindow(window);
}

QPlatformOpenGLContext *QWaylandEglClientBufferIntegration::createPlatformOpenGLContext(const QSurfaceFormat &glFormat,
      QPlatformOpenGLContext *share) const
{
   return new QWaylandGLContext(m_eglDisplay, m_display, glFormat, share);
}

void *QWaylandEglClientBufferIntegration::nativeResource(NativeResource resource)
{
   switch (resource) {
      case EglDisplay:
         return m_eglDisplay;

      default:
         break;
   }

   return nullptr;
}

void *QWaylandEglClientBufferIntegration::nativeResourceForContext(NativeResource resource, QPlatformOpenGLContext *context)
{
   Q_ASSERT(context != nullptr);

   switch (resource) {
      case EglConfig:
         return static_cast<QWaylandGLContext *>(context)->eglConfig();

      case EglContext:
         return static_cast<QWaylandGLContext *>(context)->eglContext();

      case EglDisplay:
         return m_eglDisplay;

      default:
         break;
   }

   return nullptr;
}

EGLDisplay QWaylandEglClientBufferIntegration::eglDisplay() const
{
   return m_eglDisplay;
}

}
