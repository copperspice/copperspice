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

#include <qxcbeglnativeinterfacehandler.h>

#include <qapplication_p.h>
#include "qxcbeglwindow.h"
#include "qxcbintegration.h"
#include "qxcbeglintegration.h"
#include "qxcbeglcontext.h"

static int resourceType(const QByteArray &key)
{
   static const QByteArray names[] = { // match QXcbEglNativeInterfaceHandler::ResourceType
      QByteArrayLiteral("egldisplay"),
      QByteArrayLiteral("eglcontext"),
      QByteArrayLiteral("eglconfig")
   };
   for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
      if (key == names[i]) {
         return i;
      }
   }

   if (key == QByteArrayLiteral("get_egl_context")) {
      return QXcbEglNativeInterfaceHandler::EglContext;
   }

   return sizeof(names) / sizeof(names[0]);
}

QXcbEglNativeInterfaceHandler::QXcbEglNativeInterfaceHandler(QXcbNativeInterface *nativeInterface)
   : QXcbNativeInterfaceHandler(nativeInterface)
{
}

QPlatformNativeInterface::NativeResourceForIntegrationFunction QXcbEglNativeInterfaceHandler::nativeResourceFunctionForIntegration(
   const QByteArray &resource) const
{
   switch (resourceType(resource)) {
      case EglDisplay:
         return eglDisplay;
      default:
         break;
   }
   return nullptr;
}

QPlatformNativeInterface::NativeResourceForContextFunction QXcbEglNativeInterfaceHandler::nativeResourceFunctionForContext(
   const QByteArray &resource) const
{
   switch (resourceType(resource)) {
      case EglContext:
         return eglContextForContext;
      case EglConfig:
         return eglConfigForContext;
      default:
         break;
   }
   return nullptr;
}

QPlatformNativeInterface::NativeResourceForWindowFunction QXcbEglNativeInterfaceHandler::nativeResourceFunctionForWindow(
   const QByteArray &resource) const
{
   switch (resourceType(resource)) {
      case EglDisplay:
         return eglDisplayForWindow;
      default:
         break;
   }
   return nullptr;
}

void *QXcbEglNativeInterfaceHandler::eglDisplay()
{
   QXcbIntegration *integration = QXcbIntegration::instance();
   QXcbEglIntegration *eglIntegration = static_cast<QXcbEglIntegration *>(integration->defaultConnection()->glIntegration());
   return eglIntegration->eglDisplay();
}

void *QXcbEglNativeInterfaceHandler::eglDisplayForWindow(QWindow *window)
{
   Q_ASSERT(window);
   if (window->supportsOpenGL() && window->handle() == nullptr) {
      return eglDisplay();
   } else if (window->supportsOpenGL()) {
      return static_cast<QXcbEglWindow *>(window->handle())->glIntegration()->eglDisplay();
   }
   return nullptr;
}

void *QXcbEglNativeInterfaceHandler::eglContextForContext(QOpenGLContext *context)
{
   Q_ASSERT(context);
   Q_ASSERT(context->handle());
   return static_cast<QXcbEglContext *>(context->handle())->eglContext();
}

void *QXcbEglNativeInterfaceHandler::eglConfigForContext(QOpenGLContext *context)
{
   Q_ASSERT(context);
   Q_ASSERT(context->handle());
   return static_cast<QXcbEglContext *>(context->handle())->eglConfig();
}

