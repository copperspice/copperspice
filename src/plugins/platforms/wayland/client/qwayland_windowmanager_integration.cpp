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

#include <qwayland_windowmanager_integration_p.h>

#include <qevent.h>
#include <qguiapplication.h>
#include <qhash.h>
#include <qplatform_nativeinterface.h>
#include <qplatform_window.h>
#include <qtevents.h>
#include <qurl.h>

#include <qwayland_display_p.h>
#include <stdint.h>

namespace QtWaylandClient {

class QWaylandWindowManagerIntegrationPrivate
{
 public:
   QWaylandWindowManagerIntegrationPrivate(QWaylandDisplay *waylandDisplay);

   bool m_blockPropertyUpdates;
   bool m_showIsFullScreen;

   QWaylandDisplay *m_waylandDisplay;
   QHash<QWindow *, QVariantMap> m_queuedProperties;
};

QWaylandWindowManagerIntegrationPrivate::QWaylandWindowManagerIntegrationPrivate(QWaylandDisplay *display)
   : m_blockPropertyUpdates(false), m_showIsFullScreen(false), m_waylandDisplay(display)
{
}

QWaylandWindowManagerIntegration::QWaylandWindowManagerIntegration(QWaylandDisplay *display)
   : d_ptr(new QWaylandWindowManagerIntegrationPrivate(display))
{
   display->addRegistryListener(&wlHandleListenerGlobal, this);
}

QWaylandWindowManagerIntegration::~QWaylandWindowManagerIntegration()
{
}

bool QWaylandWindowManagerIntegration::showIsFullScreen() const
{
   Q_D(const QWaylandWindowManagerIntegration);
   return d->m_showIsFullScreen;
}

void QWaylandWindowManagerIntegration::wlHandleListenerGlobal(void *data, wl_registry *registry, uint32_t id,
      const QString &interface, uint32_t version)
{
   (void) version;

   if (interface == "qt_windowmanager") {
      static_cast<QWaylandWindowManagerIntegration *>(data)->init(registry, id, 1);
   }
}

void QWaylandWindowManagerIntegration::windowmanager_hints(int32_t showIsFullScreen)
{
   Q_D(QWaylandWindowManagerIntegration);
   d->m_showIsFullScreen = showIsFullScreen;
}

void QWaylandWindowManagerIntegration::windowmanager_quit()
{
   QApplication::quit();
}

void QWaylandWindowManagerIntegration::openUrl_helper(const QUrl &url)
{
   Q_ASSERT(isInitialized());

   QByteArray data = url.toString().toUtf8();

   static const int chunkSize = 128;

   while (! data.isEmpty()) {
      QByteArray chunk = data.left(chunkSize);
      data = data.mid(chunkSize);
      open_url(! data.isEmpty(), QString::fromUtf8(chunk));
   }
}

bool QWaylandWindowManagerIntegration::openUrl(const QUrl &url)
{
   if (isInitialized()) {
      openUrl_helper(url);
      return true;
   }

   return QGenericUnixServices::openUrl(url);
}

bool QWaylandWindowManagerIntegration::openDocument(const QUrl &url)
{
   if (isInitialized()) {
      openUrl_helper(url);
      return true;
   }

   return QGenericUnixServices::openDocument(url);
}

}
