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

#ifndef QWINDOWSNATIVEINTERFACE_H
#define QWINDOWSNATIVEINTERFACE_H

#include <qfont.h>
#include <qplatform_nativeinterface.h>

class QWindowsNativeInterface : public QPlatformNativeInterface
{
   CS_OBJECT(QWindowsNativeInterface)

   CS_PROPERTY_READ(asyncExpose, asyncExpose)
   CS_PROPERTY_WRITE(asyncExpose, setAsyncExpose)

   CS_PROPERTY_READ(gpu, gpu)
   CS_PROPERTY_STORED(gpu, false)

 public:
   using FP_Void = void(*)();

   void *nativeResourceForIntegration(const QByteArray &resource) override;

#ifndef QT_NO_OPENGL
   void *nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context) override;
#endif

   void *nativeResourceForWindow(const QByteArray &resource, QWindow *window) override;

   CS_INVOKABLE_METHOD_1(Public, void *createMessageWindow(const QString &classNameTemplate, const QString &windowName, void *eventProc) const )
   CS_INVOKABLE_METHOD_2(createMessageWindow)

   CS_INVOKABLE_METHOD_1(Public, QString registerWindowClass(const QString &classNameIn, void *eventProc) const)
   CS_INVOKABLE_METHOD_2(registerWindowClass)

   CS_INVOKABLE_METHOD_1(Public, void beep())
   CS_INVOKABLE_METHOD_2(beep)

   CS_INVOKABLE_METHOD_1(Public, void registerWindowsMime(void *mimeIn))
   CS_INVOKABLE_METHOD_2(registerWindowsMime)

   CS_INVOKABLE_METHOD_1(Public, void unregisterWindowsMime(void *mime))
   CS_INVOKABLE_METHOD_2(unregisterWindowsMime)

   CS_INVOKABLE_METHOD_1(Public, int registerMimeType(const QString &mimeType))
   CS_INVOKABLE_METHOD_2(registerMimeType)

   CS_INVOKABLE_METHOD_1(Public, QFont logFontToQFont(const void *logFont, int verticalDpi))
   CS_INVOKABLE_METHOD_2(logFontToQFont)

   bool asyncExpose() const;
   void setAsyncExpose(bool value);

   QVariant gpu() const;

   QVariantMap windowProperties(QPlatformWindow *window) const override;
   QVariant windowProperty(QPlatformWindow *window, const QString &name) const override;
   QVariant windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const override;
   void setWindowProperty(QPlatformWindow *window, const QString &name, const QVariant &value) override;

   FP_Void platformFunction(const QByteArray &function) const override;
};

#endif