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

#ifndef QWAYLAND_CLIENTBUFFER_INTEGRATION_H
#define QWAYLAND_CLIENTBUFFER_INTEGRATION_H

#include <qglobal.h>

class QPlatformOpenGLContext;
class QSurfaceFormat;
class QWindow;

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandClientBufferIntegration
{
 public:
   enum NativeResource {
      EglDisplay,
      EglConfig,
      EglContext
   };

   QWaylandClientBufferIntegration();
   virtual ~QWaylandClientBufferIntegration();

   virtual void initialize(QWaylandDisplay *display) = 0;

   virtual bool isValid() const {
      return true;
   }

   virtual bool supportsThreadedOpenGL() const {
      return false;
   }

   virtual bool supportsWindowDecoration() const {
      return false;
   }

   virtual QWaylandWindow *createEglWindow(QWindow *window) = 0;
   virtual QPlatformOpenGLContext *createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const = 0;

   virtual void *nativeResource(NativeResource /*resource*/) {
      return nullptr;
   }

   virtual void *nativeResourceForContext(NativeResource /*resource*/, QPlatformOpenGLContext */*context*/) {
      return nullptr;
   }
};

}

#endif
