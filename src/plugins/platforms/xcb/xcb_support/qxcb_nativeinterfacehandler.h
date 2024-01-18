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

#ifndef QXCB_NATIVEINTERFACEHANDLER_H
#define QXCB_NATIVEINTERFACEHANDLER_H

#include <qbytearray.h>
#include <qplatform_nativeinterface.h>
#include <qxcb_export.h>

class QXcbNativeInterface;

class Q_XCB_EXPORT QXcbNativeInterfaceHandler
{
 public:
   using FP_Void = void(*)();

   QXcbNativeInterfaceHandler(QXcbNativeInterface *nativeInterface);
   virtual ~QXcbNativeInterfaceHandler();

   virtual QPlatformNativeInterface::FP_Integration nativeResourceFunctionForIntegration(const QByteArray &resource) const;
   virtual QPlatformNativeInterface::FP_Context nativeResourceFunctionForContext(const QByteArray &resource) const;
   virtual QPlatformNativeInterface::FP_Screen nativeResourceFunctionForScreen(const QByteArray &resource) const;
   virtual QPlatformNativeInterface::FP_Window nativeResourceFunctionForWindow(const QByteArray &resource) const;
   virtual QPlatformNativeInterface::FP_BackingStore nativeResourceFunctionForBackingStore(const QByteArray &resource) const;

   virtual FP_Void platformFunction(const QByteArray &function) const;

 protected:
   QXcbNativeInterface *m_native_interface;
};

#endif //QXCBNATIVEINTERFACEHANDLER_H
