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

#ifndef QXCB_GL_INTEGRATION_H

#include <qxcb_export.h>
#include <qxcb_window.h>

class QPlatformOffscreenSurface;
class QOffscreenSurface;
class QXcbNativeInterfaceHandler;

class Q_XCB_EXPORT QXcbGlIntegration
{
 public:
   QXcbGlIntegration();
   virtual ~QXcbGlIntegration();

   virtual bool initialize(QXcbConnection *connection) = 0;

   virtual bool supportsThreadedOpenGL() const {
      return false;
   }
   virtual bool supportsSwitchableWidgetComposition()  const {
      return true;
   }
   virtual bool handleXcbEvent(xcb_generic_event_t *event, uint responseType);

   virtual QXcbWindow *createWindow(QWindow *window) const = 0;

#ifndef QT_NO_OPENGL
   virtual QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const = 0;
#endif

   virtual QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const = 0;

   virtual QXcbNativeInterfaceHandler *nativeInterfaceHandler() const  {
      return nullptr;
   }
};

#endif
