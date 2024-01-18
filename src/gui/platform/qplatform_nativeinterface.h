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

#ifndef QPLATFORM_NATIVEINTERFACE_H
#define QPLATFORM_NATIVEINTERFACE_H

#include <qwindowdefs.h>
#include <qobject.h>
#include <qvariant.h>

class QBackingStore;
class QOpenGLContext;
class QPlatformWindow;
class QScreen;
class QWindow;

class Q_GUI_EXPORT QPlatformNativeInterface : public QObject
{
   GUI_CS_OBJECT(QPlatformNativeInterface)

 public:
   virtual void *nativeResourceForIntegration(const QByteArray &resource);
   virtual void *nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context);
   virtual void *nativeResourceForScreen(const QByteArray &resource, QScreen *screen);
   virtual void *nativeResourceForWindow(const QByteArray &resource, QWindow *window);
   virtual void *nativeResourceForBackingStore(const QByteArray &resource, QBackingStore *backingStore);

   using FP_Integration  = void *(*)();
   using FP_Context      = void *(*)(QOpenGLContext *);
   using FP_Screen       = void *(*)(QScreen *);
   using FP_Window       = void *(*)(QWindow *);
   using FP_BackingStore = void *(*)(QBackingStore *);

   using FP_Void         = void(*)();

   virtual FP_Integration nativeResourceFunctionForIntegration(const QByteArray &resource);
   virtual FP_Context nativeResourceFunctionForContext(const QByteArray &resource);
   virtual FP_Screen nativeResourceFunctionForScreen(const QByteArray &resource);
   virtual FP_Window nativeResourceFunctionForWindow(const QByteArray &resource);
   virtual FP_BackingStore nativeResourceFunctionForBackingStore(const QByteArray &resource);

   virtual FP_Void platformFunction(const QByteArray &function) const;

   virtual QVariantMap windowProperties(QPlatformWindow *window) const;
   virtual QVariant windowProperty(QPlatformWindow *window, const QString &name) const;
   virtual QVariant windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const;
   virtual void setWindowProperty(QPlatformWindow *window, const QString &name, const QVariant &value);

   GUI_CS_SIGNAL_1(Public, void windowPropertyChanged(QPlatformWindow *window, const QString &propertyName))
   GUI_CS_SIGNAL_2(windowPropertyChanged, window, propertyName)
};

#endif
