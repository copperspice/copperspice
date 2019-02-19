/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qxcbglxnativeinterfacehandler.h"

#include "qglxintegration.h"
#include <QOpenGLContext>

static int resourceType(const QByteArray &key)
{
   static const QByteArray names[] = { // match QXcbGlxNativeInterfaceHandler::ResourceType
      QByteArrayLiteral("glxconfig"),
      QByteArrayLiteral("glxcontext"),
   };
   for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
      if (key == names[i]) {
         return i;
      }
   }

   return sizeof(names) / sizeof(names[0]);
}

QXcbGlxNativeInterfaceHandler::QXcbGlxNativeInterfaceHandler(QXcbNativeInterface *nativeInterface)
   : QXcbNativeInterfaceHandler(nativeInterface)
{
}

QPlatformNativeInterface::NativeResourceForContextFunction QXcbGlxNativeInterfaceHandler::nativeResourceFunctionForContext(
   const QByteArray &resource) const
{
   switch (resourceType(resource)) {
      case GLXConfig:
         return glxConfigForContext;
      case GLXContext:
         return glxContextForContext;
      default:
         break;
   }
   return nullptr;
}

void *QXcbGlxNativeInterfaceHandler::glxContextForContext(QOpenGLContext *context)
{
   Q_ASSERT(context);
   QGLXContext *glxPlatformContext = static_cast<QGLXContext *>(context->handle());
   return glxPlatformContext->glxContext();
}

void *QXcbGlxNativeInterfaceHandler::glxConfigForContext(QOpenGLContext *context)
{
   Q_ASSERT(context);
   QGLXContext *glxPlatformContext = static_cast<QGLXContext *>(context->handle());
   return glxPlatformContext->glxConfig();

}

