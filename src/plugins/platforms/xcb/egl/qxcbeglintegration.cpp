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

#include "qxcbeglintegration.h"

#include "qxcbeglcontext.h"
#include <QOffscreenSurface>
#include "qxcbeglnativeinterfacehandler.h"

QXcbEglIntegration::QXcbEglIntegration()
   : m_connection(nullptr)
   , m_egl_display(EGL_NO_DISPLAY)
{
   qCDebug(QT_XCB_GLINTEGRATION) << "Xcb EGL gl-integration created";
}

QXcbEglIntegration::~QXcbEglIntegration()
{
   if (m_egl_display != EGL_NO_DISPLAY) {
      eglTerminate(m_egl_display);
   }
}

bool QXcbEglIntegration::initialize(QXcbConnection *connection)
{
   m_connection = connection;
   m_egl_display = eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(xlib_display()));

   EGLint major, minor;
   bool success = eglInitialize(m_egl_display, &major, &minor);
   if (!success) {
      m_egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      qCDebug(QT_XCB_GLINTEGRATION) << "Xcb EGL gl-integration retrying with display" << m_egl_display;
      success = eglInitialize(m_egl_display, &major, &minor);
   }

   m_native_interface_handler.reset(new QXcbEglNativeInterfaceHandler(connection->nativeInterface()));

   qCDebug(QT_XCB_GLINTEGRATION) << "Xcb EGL gl-integration successfully initialized";
   return success;
}

QXcbWindow *QXcbEglIntegration::createWindow(QWindow *window) const
{
   return new QXcbEglWindow(window, const_cast<QXcbEglIntegration *>(this));
}

QPlatformOpenGLContext *QXcbEglIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
   QXcbScreen *screen = static_cast<QXcbScreen *>(context->screen()->handle());
   QXcbEglContext *platformContext = new QXcbEglContext(context->format(),
      context->shareHandle(),
      eglDisplay(),
      screen->connection(),
      context->nativeHandle());
   context->setNativeHandle(platformContext->nativeHandle());
   return platformContext;
}

QPlatformOffscreenSurface *QXcbEglIntegration::createPlatformOffscreenSurface(QOffscreenSurface *surface) const
{
   return new QEGLPbuffer(eglDisplay(), surface->requestedFormat(), surface);
}

void *QXcbEglIntegration::xlib_display() const
{
#ifdef XCB_USE_XLIB
   return m_connection->xlib_display();
#else
   return EGL_DEFAULT_DISPLAY;
#endif
}

