/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qwayland_native_interface_p.h>

#include <qscreen.h>

#include <qapplication_p.h>
#include <qwayland_clientbuffer_integration_p.h>
#include <qwayland_display_p.h>
#include <qwayland_integration_p.h>
#include <qwayland_screen_p.h>
#include <qwayland_window_p.h>

namespace QtWaylandClient {

QWaylandNativeInterface::QWaylandNativeInterface(QWaylandIntegration *integration)
   : m_integration(integration)
{
}

void *QWaylandNativeInterface::nativeResourceForIntegration(const QByteArray &resourceString)
{
   QByteArray lowerCaseResource = resourceString.toLower();

   if (lowerCaseResource == "display" || lowerCaseResource == "wl_display" || lowerCaseResource == "nativedisplay") {
      return m_integration->display()->wl_display();
   }

   // pending implementation

   return nullptr;
}

void *QWaylandNativeInterface::nativeResourceForWindow(const QByteArray &resourceString, QWindow *window)
{
   QByteArray lowerCaseResource = resourceString.toLower();

   if (lowerCaseResource == "display") {
      return m_integration->display()->wl_display();
   }

   // pending implementation

   return nullptr;
}

void *QWaylandNativeInterface::nativeResourceForScreen(const QByteArray &resourceString, QScreen *screen)
{
   QByteArray lowerCaseResource = resourceString.toLower();

   if (lowerCaseResource == "output") {
      return ((QWaylandScreen *)screen->handle())->output();
   }

   return nullptr;
}

#ifndef QT_NO_OPENGL
void *QWaylandNativeInterface::nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context)
{
   QByteArray lowerCaseResource = resource.toLower();

   // pending implementation

   return nullptr;
}
#endif

QVariantMap QWaylandNativeInterface::windowProperties(QPlatformWindow *window) const
{
   QWaylandWindow *waylandWindow = static_cast<QWaylandWindow *>(window);
   return waylandWindow->properties();
}

QVariant QWaylandNativeInterface::windowProperty(QPlatformWindow *window, const QString &name) const
{
   QWaylandWindow *waylandWindow = static_cast<QWaylandWindow *>(window);
   return waylandWindow->property(name);
}

QVariant QWaylandNativeInterface::windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const
{
   QWaylandWindow *waylandWindow = static_cast<QWaylandWindow *>(window);
   return waylandWindow->property(name, defaultValue);
}

void QWaylandNativeInterface::setWindowProperty(QPlatformWindow *window, const QString &name, const QVariant &value)
{
   QWaylandWindow *wlWindow = static_cast<QWaylandWindow *>(window);
   wlWindow->sendProperty(name, value);
}

void QWaylandNativeInterface::emitWindowPropertyChanged(QPlatformWindow *window, const QString &name)
{
   emit windowPropertyChanged(window, name);
}

}
