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

#ifndef QWAYLAND_WINDOWMANAGER_INTEGRATION_H
#define QWAYLAND_WINDOWMANAGER_INTEGRATION_H

#include <qobject.h>
#include <qscopedpointer.h>

#include <qwayland-windowmanager.h>
#include <wayland-client.h>

#include <qgenericunix_services_p.h>

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;

class QWaylandWindowManagerIntegrationPrivate;

class Q_WAYLAND_CLIENT_EXPORT QWaylandWindowManagerIntegration : public QObject,
      public QGenericUnixServices, public QtWayland::qt_windowmanager
{
   CS_OBJECT(QWaylandWindowManagerIntegration)

 public:
   explicit QWaylandWindowManagerIntegration(QWaylandDisplay *waylandDisplay);
   virtual ~QWaylandWindowManagerIntegration();

   bool openUrl(const QUrl &url) override;
   bool openDocument(const QUrl &url) override;

   bool showIsFullScreen() const;

 private:
   Q_DECLARE_PRIVATE(QWaylandWindowManagerIntegration)

   static void wlHandleListenerGlobal(void *data, wl_registry *registry, uint32_t id,
         const QString &interface, uint32_t version);

   void windowmanager_hints(int32_t showIsFullScreen) override;
   void windowmanager_quit() override;

   void openUrl_helper(const QUrl &url);

   QScopedPointer<QWaylandWindowManagerIntegrationPrivate> d_ptr;
};

}

#endif
