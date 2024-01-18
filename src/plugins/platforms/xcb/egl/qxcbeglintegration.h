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

#ifndef QXCBEGLINTEGRATION_H
#define QXCBEGLINTEGRATION_H

#include "qxcbglintegration.h"
#include "qxcbeglwindow.h"

#include <QOpenGLContext>
#include <qplatform_screen.h>
#include <QScreen>

#include "qxcbscreen.h"
#include "qxcbeglinclude.h"

class QXcbEglNativeInterfaceHandler;

class QXcbEglIntegration : public QXcbGlIntegration
{
 public:
   QXcbEglIntegration();
   ~QXcbEglIntegration();

   bool initialize(QXcbConnection *connection) override;

   QXcbWindow *createWindow(QWindow *window) const override;
   QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
   QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const override;

   bool supportsThreadedOpenGL() const override {
      return true;
   }

   EGLDisplay eglDisplay() const {
      return m_egl_display;
   }
   void *xlib_display() const;

 private:
   QXcbConnection *m_connection;
   EGLDisplay m_egl_display;

   QScopedPointer<QXcbEglNativeInterfaceHandler> m_native_interface_handler;
};

#endif //QXCBEGLINTEGRATION_H
