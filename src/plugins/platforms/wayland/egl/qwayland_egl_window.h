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

#ifndef QWAYLAND_EGL_WINDOW_H
#define QWAYLAND_EGL_WINDOW_H

#include <qwayland_egl_clientbuffer_integration.h>
#include <qwayland_egl_forward.h>

#include <qwayland_window_p.h>

class QOpenGLFramebufferObject;

namespace QtWaylandClient {

class QWaylandGLContext;

class QWaylandEglWindow : public QWaylandWindow
{
 public:
   QWaylandEglWindow(QWindow *window);
   ~QWaylandEglWindow();

   void bindContentFBO();

   GLuint contentFBO() const;
   GLuint contentTexture() const;

   QRect contentsRect() const;

   EGLSurface eglSurface() const;
   QSurfaceFormat format() const override;

   void invalidateSurface() override;

   void setGeometry(const QRect &rect) override;
   void setVisible(bool visible) override;

   void updateSurface(bool create);

   WindowType windowType() const override;

 private:
   struct wl_egl_window *m_waylandEglWindow;

   EGLSurface m_eglSurface;
   EGLConfig m_eglConfig;

   QWaylandEglClientBufferIntegration *m_clientBufferIntegration;
   // unused:  const QWaylandWindow *m_parentWindow;

   mutable bool m_resize;
   mutable QOpenGLFramebufferObject *m_contentFBO;

   QSurfaceFormat m_format;
};

}

#endif
