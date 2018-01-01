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

#include "qwaylandeglintegration.h"

#include "gl_integration/qwaylandglintegration.h"

#include "qwaylandeglwindow.h"

#include <QtCore/QDebug>

QWaylandEglIntegration::QWaylandEglIntegration(struct wl_display *waylandDisplay)
    : mWaylandDisplay(waylandDisplay)
{
    qDebug() << "Using Wayland-EGL";
}


QWaylandEglIntegration::~QWaylandEglIntegration()
{
    eglTerminate(mEglDisplay);
}

void QWaylandEglIntegration::initialize()
{
    EGLint major,minor;
    mEglDisplay = eglGetDisplay(mWaylandDisplay);
    if (mEglDisplay == NULL) {
        qWarning("EGL not available");
    } else {
        if (!eglInitialize(mEglDisplay, &major, &minor)) {
            qWarning("failed to initialize EGL display");
            return;
        }
    }
}

QWaylandWindow *QWaylandEglIntegration::createEglWindow(QWidget *window)
{
    return new QWaylandEglWindow(window);
}

EGLDisplay QWaylandEglIntegration::eglDisplay() const
{
    return mEglDisplay;
}

QWaylandGLIntegration *QWaylandGLIntegration::createGLIntegration(QWaylandDisplay *waylandDisplay)
{
    return new QWaylandEglIntegration(waylandDisplay->wl_display());
}
