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

#include <qwayland_egl_clientbuffer_integration.h>

#include <qdebug.h>

#include <qwayland_display_p.h>
#include <qwayland_window_p.h>

#include <wayland-client.h>

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
   // pending implementation
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
   // pending implementation

   return nullptr;
}

QPlatformOpenGLContext *QWaylandEglClientBufferIntegration::createPlatformOpenGLContext(const QSurfaceFormat &glFormat,
      QPlatformOpenGLContext *share) const
{
   // pending implementation

   return nullptr;
}

}
