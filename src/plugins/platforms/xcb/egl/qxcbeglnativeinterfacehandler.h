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

#ifndef QXCBEGLNATIVEINTERFACEHANDLER_H
#define QXCBEGLNATIVEINTERFACEHANDLER_H

#include "qxcbnativeinterfacehandler.h"

class QXcbEglNativeInterfaceHandler : public QXcbNativeInterfaceHandler
{
 public:
   enum ResourceType {
      EglDisplay,
      EglContext,
      EglConfig
   };

   QXcbEglNativeInterfaceHandler(QXcbNativeInterface *nativeInterface);

   QPlatformNativeInterface::NativeResourceForIntegrationFunction nativeResourceFunctionForIntegration(
      const QByteArray &resource) const override;
   QPlatformNativeInterface::NativeResourceForContextFunction nativeResourceFunctionForContext(const QByteArray &resource) const override;
   QPlatformNativeInterface::NativeResourceForWindowFunction nativeResourceFunctionForWindow(const QByteArray &resource) const override;

 private:
   static void *eglDisplay();
   static void *eglDisplayForWindow(QWindow *window);
   static void *eglContextForContext(QOpenGLContext *context);
   static void *eglConfigForContext(QOpenGLContext *context);
};

#endif //QXCBEGLNATIVEINTERFACEHANDLER_H
