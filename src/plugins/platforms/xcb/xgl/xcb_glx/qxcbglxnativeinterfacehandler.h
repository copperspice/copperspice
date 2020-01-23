/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QXCBGLXNATIVEINTERFACEHANDLER_H
#define QXCBGLXNATIVEINTERFACEHANDLER_H

#include "qxcbnativeinterfacehandler.h"

class QXcbGlxNativeInterfaceHandler : public QXcbNativeInterfaceHandler
{
 public:
   enum ResourceType {
      GLXConfig,
      GLXContext,
   };

   QXcbGlxNativeInterfaceHandler(QXcbNativeInterface *nativeInterface);
   QPlatformNativeInterface::NativeResourceForContextFunction nativeResourceFunctionForContext(const QByteArray &resource) const override;

 private:
   static void *glxContextForContext(QOpenGLContext *context);
   static void *glxConfigForContext(QOpenGLContext *context);
};

#endif //QXCBGLXNATIVEINTERFACEHANDLER_H
