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

#include <qxcb_nativeinterfacehandler.h>
#include <qxcb_nativeinterface.h>

QXcbNativeInterfaceHandler::QXcbNativeInterfaceHandler(QXcbNativeInterface *nativeInterface)
   : m_native_interface(nativeInterface)
{
   m_native_interface->addHandler(this);
}

QXcbNativeInterfaceHandler::~QXcbNativeInterfaceHandler()
{
   m_native_interface->removeHandler(this);
}

QPlatformNativeInterface::FP_Integration QXcbNativeInterfaceHandler::nativeResourceFunctionForIntegration(
   const QByteArray &resource) const
{
   (void) resource;
   return nullptr;
}

QPlatformNativeInterface::FP_Context QXcbNativeInterfaceHandler::nativeResourceFunctionForContext(const QByteArray &resource) const
{
   (void) resource;
   return nullptr;
}

QPlatformNativeInterface::FP_Screen QXcbNativeInterfaceHandler::nativeResourceFunctionForScreen(const QByteArray &resource) const
{
   (void) resource;
   return nullptr;
}

QPlatformNativeInterface::FP_Window QXcbNativeInterfaceHandler::nativeResourceFunctionForWindow(const QByteArray &resource) const
{
   (void) resource;
   return nullptr;
}

QPlatformNativeInterface::FP_BackingStore QXcbNativeInterfaceHandler::nativeResourceFunctionForBackingStore(
   const QByteArray &resource) const
{
   (void) resource;
   return nullptr;
}

QXcbNativeInterfaceHandler::FP_Void QXcbNativeInterfaceHandler::platformFunction(const QByteArray &function) const
{
   (void) function;
   return nullptr;
}

