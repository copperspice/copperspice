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

#include "qwaylandxcompositeglxintegration.h"

#include "qwaylandxcompositeglxwindow.h"

#include <QtCore/QDebug>

#include "wayland-xcomposite-client-protocol.h"

QWaylandGLIntegration * QWaylandGLIntegration::createGLIntegration(QWaylandDisplay *waylandDisplay)
{
    return new QWaylandXCompositeGLXIntegration(waylandDisplay);
}

QWaylandXCompositeGLXIntegration::QWaylandXCompositeGLXIntegration(QWaylandDisplay * waylandDispaly)
    : QWaylandGLIntegration()
    , mWaylandDisplay(waylandDispaly)
{
    qDebug() << "Using XComposite-GLX";
    wl_display_add_global_listener(waylandDispaly->wl_display(), QWaylandXCompositeGLXIntegration::wlDisplayHandleGlobal,
                                   this);
}

QWaylandXCompositeGLXIntegration::~QWaylandXCompositeGLXIntegration()
{
    XCloseDisplay(mDisplay);
}

void QWaylandXCompositeGLXIntegration::initialize()
{
}

QWaylandWindow * QWaylandXCompositeGLXIntegration::createEglWindow(QWidget *widget)
{
    return new QWaylandXCompositeGLXWindow(widget,this);
}

Display * QWaylandXCompositeGLXIntegration::xDisplay() const
{
    return mDisplay;
}

int QWaylandXCompositeGLXIntegration::screen() const
{
    return mScreen;
}

Window QWaylandXCompositeGLXIntegration::rootWindow() const
{
    return mRootWindow;
}

QWaylandDisplay * QWaylandXCompositeGLXIntegration::waylandDisplay() const
{
    return mWaylandDisplay;
}
wl_xcomposite * QWaylandXCompositeGLXIntegration::waylandXComposite() const
{
    return mWaylandComposite;
}

const struct wl_xcomposite_listener QWaylandXCompositeGLXIntegration::xcomposite_listener = {
    QWaylandXCompositeGLXIntegration::rootInformation
};

void QWaylandXCompositeGLXIntegration::wlDisplayHandleGlobal(wl_display *display, uint32_t id, const char *interface, uint32_t version, void *data)
{
    Q_UNUSED(version);
    if (strcmp(interface, "wl_xcomposite") == 0) {
        QWaylandXCompositeGLXIntegration *integration = static_cast<QWaylandXCompositeGLXIntegration *>(data);
        integration->mWaylandComposite = wl_xcomposite_create(display,id,1);
        wl_xcomposite_add_listener(integration->mWaylandComposite,&xcomposite_listener,integration);
    }

}

void QWaylandXCompositeGLXIntegration::rootInformation(void *data, wl_xcomposite *xcomposite, const char *display_name, uint32_t root_window)
{
    Q_UNUSED(xcomposite);
    QWaylandXCompositeGLXIntegration *integration = static_cast<QWaylandXCompositeGLXIntegration *>(data);

    integration->mDisplay = XOpenDisplay(display_name);
    integration->mRootWindow = (Window) root_window;
    integration->mScreen = XDefaultScreen(integration->mDisplay);
}

