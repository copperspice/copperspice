/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef QWAYLAND_NATIVE_INTERFACE_H
#define QWAYLAND_NATIVE_INTERFACE_H

#include <qhash.h>
#include <qplatform_nativeinterface.h>
#include <qvariant.h>
#include <qplatform_window.h>

namespace QtWaylandClient {

class QWaylandIntegration;

class Q_WAYLAND_CLIENT_EXPORT QWaylandNativeInterface : public QPlatformNativeInterface
{
 public:
   QWaylandNativeInterface(QWaylandIntegration *integration);

   void *nativeResourceForIntegration(const QByteArray &resource) override;
   void *nativeResourceForWindow(const QByteArray &resourceString, QWindow *window) override;
   void *nativeResourceForScreen(const QByteArray &resourceString, QScreen *screen) override;

#ifndef QT_NO_OPENGL
   void *nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context) override;
#endif

   void setWindowProperty(QPlatformWindow *window, const QString &name, const QVariant &value) override;

   QVariantMap windowProperties(QPlatformWindow *window) const override;
   QVariant windowProperty(QPlatformWindow *window, const QString &name) const override;
   QVariant windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const override;

   void emitWindowPropertyChanged(QPlatformWindow *window, const QString &name);

 private:
   QWaylandIntegration *m_integration;
   QHash<QPlatformWindow *, QVariantMap> m_windowProperties;
};

}

#endif
