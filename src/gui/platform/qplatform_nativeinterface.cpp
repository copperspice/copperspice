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

#include <qplatform_nativeinterface.h>

void *QPlatformNativeInterface::nativeResourceForIntegration(const QByteArray &resource)
{
   return 0;
}

void *QPlatformNativeInterface::nativeResourceForScreen(const QByteArray &resource, QScreen *screen)
{
   return 0;
}


void *QPlatformNativeInterface::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
   return 0;
}

void *QPlatformNativeInterface::nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context)
{
   return 0;
}


void *QPlatformNativeInterface::nativeResourceForBackingStore(const QByteArray &resource, QBackingStore *backingStore)
{
   return 0;
}

QPlatformNativeInterface::FP_Integration QPlatformNativeInterface::nativeResourceFunctionForIntegration(const QByteArray &resource)
{
   return 0;
}

QPlatformNativeInterface::FP_Context QPlatformNativeInterface::nativeResourceFunctionForContext(const QByteArray &resource)
{
   return 0;
}

QPlatformNativeInterface::FP_Screen QPlatformNativeInterface::nativeResourceFunctionForScreen(const QByteArray &resource)
{
   return 0;
}

QPlatformNativeInterface::FP_Window QPlatformNativeInterface::nativeResourceFunctionForWindow(const QByteArray &resource)
{
   return 0;
}


QPlatformNativeInterface::FP_BackingStore QPlatformNativeInterface::nativeResourceFunctionForBackingStore(const QByteArray &resource)
{
   return 0;
}

QPlatformNativeInterface::FP_Void QPlatformNativeInterface::platformFunction(const QByteArray &function) const
{
   return nullptr;
}


QVariantMap QPlatformNativeInterface::windowProperties(QPlatformWindow *window) const
{
   return QVariantMap();
}

QVariant QPlatformNativeInterface::windowProperty(QPlatformWindow *window, const QString &name) const
{
   return QVariant();
}

QVariant QPlatformNativeInterface::windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const
{
   return QVariant();
}

void QPlatformNativeInterface::setWindowProperty(QPlatformWindow *window, const QString &name, const QVariant &value)
{
}

