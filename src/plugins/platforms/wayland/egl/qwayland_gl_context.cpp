/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qwayland_gl_context.h>

#include <qdebug.h>
#include <qsurfaceformat.h>
#include <qwayland_egl_window.h>

#include <qapplication_p.h>
#include <qwayland_decorations_blitter_p.h>
#include <qwayland_egl_stateguard_p.h>
#include <qwayland_integration_p.h>

namespace QtWaylandClient {

QWaylandGLContext::QWaylandGLContext(EGLDisplay eglDisplay, QWaylandDisplay *display,
      const QSurfaceFormat &format, QPlatformOpenGLContext *share)
   : QPlatformOpenGLContext(), m_useNativeDefaultFbo(false), m_eglDisplay(eglDisplay), m_blitter(nullptr), m_display(display)
{
   QSurfaceFormat fmt = format;

   if (static_cast<QWaylandIntegration *>(QApplicationPrivate::platformIntegration())->display()->supportsWindowDecoration()) {
      fmt.setAlphaBufferSize(8);
   }

   // pending implementation

   m_shareEGLContext = share ? static_cast<QWaylandGLContext *>(share)->eglContext() : EGL_NO_CONTEXT;
   updateGLFormat();
}

QWaylandGLContext::~QWaylandGLContext()
{
   delete m_blitter;
   eglDestroyContext(m_eglDisplay, m_context);
}

GLuint QWaylandGLContext::defaultFramebufferObject(QPlatformSurface *surface) const
{
   if (m_useNativeDefaultFbo) {
      return 0;
   }

   return static_cast<QWaylandEglWindow *>(surface)->contentFBO();
}

void QWaylandGLContext::doneCurrent()
{
   eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

EGLConfig QWaylandGLContext::eglConfig() const
{
   return m_config;
}

void (*QWaylandGLContext::getProcAddress(const QByteArray &procName)) ()
{
   return eglGetProcAddress(procName.constData());
}

bool QWaylandGLContext::isSharing() const
{
   return m_shareEGLContext != EGL_NO_CONTEXT;
}

bool QWaylandGLContext::isValid() const
{
   return m_context != EGL_NO_CONTEXT;
}

bool QWaylandGLContext::makeCurrent(QPlatformSurface *surface)
{
   // constructor calls eglBindAPI() with the correct value
   // their api says: eglBindAPI() defines the rendering API in the thread it is called from
   // makeCurrent() can be called from a different thread, not necessarily the one where
   // *this* context was created

   // make sure to call eglBindAPI() in the correct thread
   if (eglQueryAPI() != m_api) {
      eglBindAPI(m_api);
   }

   QWaylandEglWindow *window = static_cast <QWaylandEglWindow *> (surface);
   EGLSurface eglSurface = window->eglSurface();

   if ( (eglSurface != EGL_NO_SURFACE) && (eglGetCurrentContext() == m_context) && (eglSurface == eglGetCurrentSurface(EGL_DRAW))) {
      return true;
   }

   window->setCanResize(false);

   // Core profiles mandate the use of VAOs when rendering.
   // A VAO is needed in DecorationsBlitter, however this would require a QOpenGLFunctions_3_2_Core,
   // which would break when using a lower version context.
   // Instead, disable decorations for core profiles until subsurfaces are used

   if (m_format.profile() != QSurfaceFormat::CoreProfile && ! window->decoration()) {
      window->createDecoration();
   }

   if (eglSurface == EGL_NO_SURFACE) {
      window->updateSurface(true);
      eglSurface = window->eglSurface();
   }

   if (! eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_context)) {
      qWarning("QWaylandGLContext::makeCurrent() Egl Error: %x\n", eglGetError());
      window->setCanResize(true);
      return false;
   }

   window->bindContentFBO();

   return true;
}

void QWaylandGLContext::swapBuffers(QPlatformSurface *surface)
{
   QWaylandEglWindow *window = static_cast<QWaylandEglWindow *>(surface);

   EGLSurface eglSurface = window->eglSurface();

   if (window->decoration()) {
      makeCurrent(surface);

      // need to save & restore all states
      // applications are usually not prepared for random context state changes in a call to swapBuffers()
      EGL_StateGuard eglStateGuard;

      if (m_blitter == nullptr) {
         m_blitter = new DecorationsBlitter(this);
      }

      m_blitter->blit(window);
   }

   eglSwapBuffers(m_eglDisplay, eglSurface);

   window->setCanResize(true);
}

void QWaylandGLContext::updateGLFormat()
{
   // save & restore state to prevent QOpenGLContext::currentContext() from becoming
   // unusable after QOpenGLContext::create()

   EGLDisplay prevDisplay = eglGetCurrentDisplay();

   if (prevDisplay == EGL_NO_DISPLAY) {
      // no context is current
      prevDisplay = m_eglDisplay;
   }

   EGLContext prevContext     = eglGetCurrentContext();
   EGLSurface prevSurfaceDraw = eglGetCurrentSurface(EGL_DRAW);
   EGLSurface prevSurfaceRead = eglGetCurrentSurface(EGL_READ);

   wl_surface *wlSurface    = m_display->createSurface(nullptr);
   wl_egl_window *eglWindow = wl_egl_window_create(wlSurface, 1, 1);

   EGLSurface eglSurface = eglCreateWindowSurface(m_eglDisplay, m_config, eglWindow, nullptr);

   if (eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, m_context)) {
      if (m_format.renderableType() == QSurfaceFormat::OpenGL || m_format.renderableType() == QSurfaceFormat::OpenGLES) {
         const GLubyte *versionStr = glGetString(GL_VERSION);

         if (versionStr != nullptr) {
            QByteArray version = QByteArray(reinterpret_cast<const char *>(versionStr));
            int major;
            int minor;

            if (QPlatformOpenGLContext::parseOpenGLVersion(version, major, minor)) {
               m_format.setMajorVersion(major);
               m_format.setMinorVersion(minor);
            }
         }

         m_format.setProfile(QSurfaceFormat::NoProfile);
         m_format.setOptions(QSurfaceFormat::FormatOptions());

         if (m_format.renderableType() == QSurfaceFormat::OpenGL) {
            // check profile and options

            if (m_format.majorVersion() < 3) {
               m_format.setOption(QSurfaceFormat::DeprecatedFunctions);
            } else {
               GLint value = 0;
               glGetIntegerv(GL_CONTEXT_FLAGS, &value);

               if (! (value & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)) {
                  m_format.setOption(QSurfaceFormat::DeprecatedFunctions);
               }

               if (value & GL_CONTEXT_FLAG_DEBUG_BIT) {
                  m_format.setOption(QSurfaceFormat::DebugContext);
               }

               if (m_format.version() >= std::make_pair(3, 2)) {
                  value = 0;
                  glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &value);

                  if (value & GL_CONTEXT_CORE_PROFILE_BIT) {
                     m_format.setProfile(QSurfaceFormat::CoreProfile);
                  } else if (value & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
                     m_format.setProfile(QSurfaceFormat::CompatibilityProfile);
                  }
               }
            }
         }
      }

      eglMakeCurrent(prevDisplay, prevSurfaceDraw, prevSurfaceRead, prevContext);
   }

   eglDestroySurface(m_eglDisplay, eglSurface);
   wl_egl_window_destroy(eglWindow);
   wl_surface_destroy(wlSurface);
}

}
