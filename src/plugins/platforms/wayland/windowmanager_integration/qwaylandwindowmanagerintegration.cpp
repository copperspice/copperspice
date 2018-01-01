/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qwaylandwindowmanagerintegration.h"
#include "qwaylandwindowmanager-client-protocol.h"

#include <stdint.h>

QWaylandWindowManagerIntegration *QWaylandWindowManagerIntegration::createIntegration(QWaylandDisplay *waylandDisplay)
{
    return new QWaylandWindowManagerIntegration(waylandDisplay);
}

QWaylandWindowManagerIntegration::QWaylandWindowManagerIntegration(QWaylandDisplay *waylandDisplay)
    : mWaylandDisplay(waylandDisplay)
    , mWaylandWindowManager(0)
{
    wl_display_add_global_listener(mWaylandDisplay->wl_display(),
                                   QWaylandWindowManagerIntegration::wlHandleListenerGlobal,
                                   this);
}

QWaylandWindowManagerIntegration::~QWaylandWindowManagerIntegration()
{

}

struct wl_windowmanager *QWaylandWindowManagerIntegration::windowManager() const
{
    return mWaylandWindowManager;
}

void QWaylandWindowManagerIntegration::wlHandleListenerGlobal(wl_display *display, uint32_t id, const char *interface, uint32_t version, void *data)
{
    if (strcmp(interface, "wl_windowmanager") == 0) {
        QWaylandWindowManagerIntegration *integration = static_cast<QWaylandWindowManagerIntegration *>(data);
        integration->mWaylandWindowManager = wl_windowmanager_create(display, id);
    }
}

void QWaylandWindowManagerIntegration::mapClientToProcess(long long processId)
{
    if (mWaylandWindowManager)
        wl_windowmanager_map_client_to_process(mWaylandWindowManager, (uint32_t) processId);
}

void QWaylandWindowManagerIntegration::authenticateWithToken(const QByteArray &token)
{
    QByteArray authToken = token;
    if (authToken.isEmpty())
        authToken = qgetenv("WL_AUTHENTICATION_TOKEN");
    if (mWaylandWindowManager)
        wl_windowmanager_authenticate_with_token(mWaylandWindowManager, authToken.constData());
}
