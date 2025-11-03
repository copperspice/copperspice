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

#ifndef QWAYLAND_EGL_INTEGRATION_H
#define QWAYLAND_EGL_INTEGRATION_H

#include <qwayland_clientbuffer_integration_p.h>

#include <qwayland_egl_forward.h>

class QWindow;

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;

class QWaylandEglClientBufferIntegration : public QWaylandClientBufferIntegration
{
 public:
   QWaylandEglClientBufferIntegration();
   ~QWaylandEglClientBufferIntegration();

   void initialize(QWaylandDisplay *display) override;
   bool isValid() const override;
   bool supportsThreadedOpenGL() const override;
   bool supportsWindowDecoration() const override;

   QWaylandWindow *createEglWindow(QWindow *window) override;
   QPlatformOpenGLContext *createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const override;

   EGLDisplay eglDisplay() const;

 private:
   QWaylandDisplay *m_display;

   EGLDisplay m_eglDisplay;
   bool m_supportsThreading;
};

}

#endif
