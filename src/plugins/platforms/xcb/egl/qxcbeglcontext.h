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

#ifndef QXCBEGLCONTEXT_H
#define QXCBEGLCONTEXT_H

#include "qxcbeglwindow.h"
#include <qeglplatformcontext_p.h>
#include <qeglpbuffer_p.h>
#include <QEGLNativeContext>

//####todo remove the noops (looks like their where there in the initial commit)
class QXcbEglContext : public QEGLPlatformContext
{
 public:
   QXcbEglContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share,
      EGLDisplay display, QXcbConnection *c, const QVariant &nativeHandle)
      : QEGLPlatformContext(glFormat, share, display, 0, nativeHandle)
      , m_connection(c) {
      Q_XCB_NOOP(m_connection);
   }

   void swapBuffers(QPlatformSurface *surface) {
      Q_XCB_NOOP(m_connection);
      QEGLPlatformContext::swapBuffers(surface);
      Q_XCB_NOOP(m_connection);
   }

   bool makeCurrent(QPlatformSurface *surface) {
      Q_XCB_NOOP(m_connection);
      bool ret = QEGLPlatformContext::makeCurrent(surface);
      Q_XCB_NOOP(m_connection);
      return ret;
   }

   void doneCurrent() {
      Q_XCB_NOOP(m_connection);
      QEGLPlatformContext::doneCurrent();
      Q_XCB_NOOP(m_connection);
   }

   EGLSurface eglSurfaceForPlatformSurface(QPlatformSurface *surface) {
      if (surface->surface()->surfaceClass() == QSurface::Window) {
         return static_cast<QXcbEglWindow *>(surface)->eglSurface();
      } else {
         return static_cast<QEGLPbuffer *>(surface)->pbuffer();
      }
   }

   QVariant nativeHandle() const {
      return QVariant::fromValue<QEGLNativeContext>(QEGLNativeContext(eglContext(), eglDisplay()));
   }

 private:
   QXcbConnection *m_connection;
};

#endif //QXCBEGLCONTEXT_H

