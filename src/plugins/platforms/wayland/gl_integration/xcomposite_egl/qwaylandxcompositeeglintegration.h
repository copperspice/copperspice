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

#ifndef QWAYLANDXCOMPOSITEEGLINTEGRATION_H
#define QWAYLANDXCOMPOSITEEGLINTEGRATION_H

#include "gl_integration/qwaylandglintegration.h"
#include "wayland-client.h"

#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QtGui/QWidget>

#include <QWaitCondition>

#include <X11/Xlib.h>
#include <EGL/egl.h>

struct wl_xcomposite;

class QWaylandXCompositeEGLIntegration : public QWaylandGLIntegration
{
public:
    QWaylandXCompositeEGLIntegration(QWaylandDisplay * waylandDispaly);
    ~QWaylandXCompositeEGLIntegration();

    void initialize();

    QWaylandWindow *createEglWindow(QWidget *widget);

    QWaylandDisplay *waylandDisplay() const;
    struct wl_xcomposite *waylandXComposite() const;

    Display *xDisplay() const;
    EGLDisplay eglDisplay() const;
    int screen() const;
    Window rootWindow() const;

private:
    QWaylandDisplay *mWaylandDisplay;
    struct wl_xcomposite *mWaylandComposite;

    Display *mDisplay;
    EGLDisplay mEglDisplay;
    int mScreen;
    Window mRootWindow;

    static void wlDisplayHandleGlobal(struct wl_display *display, uint32_t id,
                             const char *interface, uint32_t version, void *data);

    static const struct wl_xcomposite_listener xcomposite_listener;
    static void rootInformation(void *data,
                 struct wl_xcomposite *xcomposite,
                 const char *display_name,
                 uint32_t root_window);
};

#endif // QWAYLANDXCOMPOSITEEGLINTEGRATION_H
