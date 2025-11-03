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

#ifndef QWAYLAND_GL_CONTEXT_H
#define QWAYLAND_GL_CONTEXT_H

#include <qplatform_openglcontext.h>
#include <qplatform_surface.h>
#include <qsurfaceformat.h>
#include <qwayland_egl_forward.h>

#include <qwayland_display_p.h>

namespace QtWaylandClient {

class DecorationsBlitter;

class QWaylandGLContext : public QPlatformOpenGLContext
{
 public:
   QWaylandGLContext(EGLDisplay eglDisplay, QWaylandDisplay *display, const QSurfaceFormat &format, QPlatformOpenGLContext *share);
   ~QWaylandGLContext();

   GLuint defaultFramebufferObject(QPlatformSurface *surface) const override;
   void doneCurrent() override;

   EGLConfig eglConfig() const;
   EGLContext eglContext() const {
      return m_context;
   }
   QSurfaceFormat format() const override {
      return m_format;
   }

   void (*getProcAddress(const QByteArray &procName)) () override;

   bool isSharing() const override;
   bool isValid() const override;

   bool makeCurrent(QPlatformSurface *surface) override;

   void setNativeDefaultFbo(bool value) {
      m_useNativeDefaultFbo = value;
   }

   void swapBuffers(QPlatformSurface *surface) override;

 private:
   bool m_useNativeDefaultFbo;
   uint m_api;

   EGLDisplay m_eglDisplay;
   EGLContext m_context;
   EGLContext m_shareEGLContext;
   EGLConfig  m_config;

   DecorationsBlitter *m_blitter;

   QSurfaceFormat m_format;
   QWaylandDisplay *m_display;
};

}

#endif
