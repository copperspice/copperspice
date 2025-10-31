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
#include <qwayland_integration_p.h>

namespace QtWaylandClient {

QWaylandGLContext::QWaylandGLContext(EGLDisplay eglDisplay, QWaylandDisplay *display,
      const QSurfaceFormat &format, QPlatformOpenGLContext *share)
   : QPlatformOpenGLContext(), m_useNativeDefaultFbo(false), m_blitter(nullptr), m_display(display)
{
   QSurfaceFormat fmt = format;

   if (static_cast<QWaylandIntegration *>(QApplicationPrivate::platformIntegration())->display()->supportsWindowDecoration()) {
      fmt.setAlphaBufferSize(8);
   }
   // pending implementation
}

QWaylandGLContext::~QWaylandGLContext()
{
   delete m_blitter;

   // pending implementation
}

bool QWaylandGLContext::makeCurrent(QPlatformSurface *surface)
{
   // pending implementation
   return true;
}

void QWaylandGLContext::doneCurrent()
{
   // pending implementation
}

void QWaylandGLContext::swapBuffers(QPlatformSurface *surface)
{
   QWaylandEglWindow *window = static_cast<QWaylandEglWindow *>(surface);

   EGLSurface eglSurface = window->eglSurface();

   if (window->decoration()) {
      makeCurrent(surface);

      // need to save & restore all states
      // applications are usually not prepared for random context state changes in a call to swapBuffers()


      if (m_blitter == nullptr) {
         m_blitter = new DecorationsBlitter(this);
      }

      m_blitter->blit(window);
   }

   eglSwapBuffers(m_eglDisplay, eglSurface);

   window->setCanResize(true);
}

GLuint QWaylandGLContext::defaultFramebufferObject(QPlatformSurface *surface) const
{
   if (m_useNativeDefaultFbo) {
      return 0;
   }

   return static_cast<QWaylandEglWindow *>(surface)->contentFBO();
}

void (*QWaylandGLContext::getProcAddress(const QByteArray &procName)) ()
{
   return eglGetProcAddress(procName.constData());
}

}
