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

#include "qwaylandreadbackeglintegration.h"

#include <QDebug>

#include "qwaylandreadbackeglwindow.h"

QWaylandReadbackEglIntegration::QWaylandReadbackEglIntegration(QWaylandDisplay *display)
    : QWaylandGLIntegration()
    , mWaylandDisplay(display)
{
    qDebug() << "Using Readback-EGL";
    char *display_name = getenv("DISPLAY");
    mDisplay = XOpenDisplay(display_name);
    mScreen = XDefaultScreen(mDisplay);
    mRootWindow = XDefaultRootWindow(mDisplay);
    XSync(mDisplay, False);
}

QWaylandReadbackEglIntegration::~QWaylandReadbackEglIntegration()
{
    XCloseDisplay(mDisplay);
}


QWaylandGLIntegration *QWaylandGLIntegration::createGLIntegration(QWaylandDisplay *waylandDisplay)
{
    return new QWaylandReadbackEglIntegration(waylandDisplay);
}

void QWaylandReadbackEglIntegration::initialize()
{
    eglBindAPI(EGL_OPENGL_ES_API);
    mEglDisplay = eglGetDisplay(mDisplay);
    EGLint major, minor;
    EGLBoolean initialized = eglInitialize(mEglDisplay,&major,&minor);
    if (initialized) {
        qDebug() << "EGL initialized successfully" << major << "," << minor;
    } else {
        qDebug() << "EGL could not initialized. All EGL and GL operations will fail";
    }
}

QWaylandWindow * QWaylandReadbackEglIntegration::createEglWindow(QWidget *widget)
{
    return new QWaylandReadbackEglWindow(widget,this);
}

EGLDisplay QWaylandReadbackEglIntegration::eglDisplay()
{
    return mEglDisplay;
}

Window QWaylandReadbackEglIntegration::rootWindow() const
{
    return mRootWindow;
}

int QWaylandReadbackEglIntegration::depth() const
{
    return XDefaultDepth(mDisplay,mScreen);
}

Display * QWaylandReadbackEglIntegration::xDisplay() const
{
    return mDisplay;
}

QWaylandDisplay * QWaylandReadbackEglIntegration::waylandDisplay() const
{
    return mWaylandDisplay;
}
