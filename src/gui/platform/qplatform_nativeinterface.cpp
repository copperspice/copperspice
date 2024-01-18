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

#include <qplatform_nativeinterface.h>

void *QPlatformNativeInterface::nativeResourceForIntegration(const QByteArray &resource)
{
   (void) resource;

   return nullptr;
}

void *QPlatformNativeInterface::nativeResourceForScreen(const QByteArray &resource, QScreen *screen)
{
   (void) resource;
   (void) screen;

   return nullptr;
}


void *QPlatformNativeInterface::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
   (void) resource;
   (void) window;

   return nullptr;
}

void *QPlatformNativeInterface::nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context)
{
   (void) resource;
   (void) context;

   return nullptr;
}

void *QPlatformNativeInterface::nativeResourceForBackingStore(const QByteArray &resource, QBackingStore *backingStore)
{
   (void) resource;
   (void) backingStore;

   return nullptr;
}

QPlatformNativeInterface::FP_Integration QPlatformNativeInterface::nativeResourceFunctionForIntegration(const QByteArray &resource)
{
   (void) resource;

   return nullptr;
}

QPlatformNativeInterface::FP_Context QPlatformNativeInterface::nativeResourceFunctionForContext(const QByteArray &resource)
{
   (void) resource;

   return nullptr;
}

QPlatformNativeInterface::FP_Screen QPlatformNativeInterface::nativeResourceFunctionForScreen(const QByteArray &resource)
{
   (void) resource;

   return nullptr;
}

QPlatformNativeInterface::FP_Window QPlatformNativeInterface::nativeResourceFunctionForWindow(const QByteArray &resource)
{
   (void) resource;

   return nullptr;
}

QPlatformNativeInterface::FP_BackingStore QPlatformNativeInterface::nativeResourceFunctionForBackingStore(const QByteArray &resource)
{
   (void) resource;

   return nullptr;
}

QPlatformNativeInterface::FP_Void QPlatformNativeInterface::platformFunction(const QByteArray &function) const
{
   (void) function;

   return nullptr;
}

QVariantMap QPlatformNativeInterface::windowProperties(QPlatformWindow *window) const
{
   (void) window;

   return QVariantMap();
}

QVariant QPlatformNativeInterface::windowProperty(QPlatformWindow *window, const QString &name) const
{
   (void) window;
   (void) name;

   return QVariant();
}

QVariant QPlatformNativeInterface::windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const
{
   (void) window;
   (void) name;
   (void) defaultValue;

   return QVariant();
}

void QPlatformNativeInterface::setWindowProperty(QPlatformWindow *window, const QString &name, const QVariant &value)
{
   (void) window;
   (void) name;
   (void) value;
}

