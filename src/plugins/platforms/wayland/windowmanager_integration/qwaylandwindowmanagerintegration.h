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

#ifndef QWAYLANDWINDOWMANAGERINTEGRATION_H
#define QWAYLANDWINDOWMANAGERINTEGRATION_H

#include <QObject>
#include "wayland-client.h"
#include "qwaylanddisplay.h"

class QWaylandWindowManagerIntegration
{
public:
    explicit QWaylandWindowManagerIntegration(QWaylandDisplay *waylandDisplay);
    virtual ~QWaylandWindowManagerIntegration();
    static QWaylandWindowManagerIntegration *createIntegration(QWaylandDisplay *waylandDisplay);
    struct wl_windowmanager *windowManager() const;

    void mapSurfaceToProcess(struct wl_surface *surface, long long processId);
    void mapClientToProcess(long long processId);
    void authenticateWithToken(const QByteArray &token = QByteArray());

private:
    static void wlHandleListenerGlobal(wl_display *display, uint32_t id,
                                       const char *interface, uint32_t version, void *data);

private:
    QWaylandDisplay *mWaylandDisplay;
    struct wl_windowmanager *mWaylandWindowManager;
};

#endif // QWAYLANDWINDOWMANAGERINTEGRATION_H
