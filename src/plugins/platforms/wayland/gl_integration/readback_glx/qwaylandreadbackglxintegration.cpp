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

#include "qwaylandreadbackglxintegration.h"

#include "qwaylandreadbackglxwindow.h"

#include <QtCore/QDebug>

QWaylandReadbackGlxIntegration::QWaylandReadbackGlxIntegration(QWaylandDisplay * waylandDispaly)
    : QWaylandGLIntegration()
    , mWaylandDisplay(waylandDispaly)
{
    qDebug() << "Using Readback-GLX";
    char *display_name = getenv("DISPLAY");
    mDisplay = XOpenDisplay(display_name);
    mScreen = XDefaultScreen(mDisplay);
    mRootWindow = XDefaultRootWindow(mDisplay);
    XSync(mDisplay, False);
}

QWaylandReadbackGlxIntegration::~QWaylandReadbackGlxIntegration()
{
    XCloseDisplay(mDisplay);
}

void QWaylandReadbackGlxIntegration::initialize()
{
}

QWaylandWindow * QWaylandReadbackGlxIntegration::createEglWindow(QWidget *widget)
{
    return new QWaylandReadbackGlxWindow(widget,this);
}

QWaylandGLIntegration * QWaylandGLIntegration::createGLIntegration(QWaylandDisplay *waylandDisplay)
{
    return new QWaylandReadbackGlxIntegration(waylandDisplay);
}

Display * QWaylandReadbackGlxIntegration::xDisplay() const
{
    return mDisplay;
}

int QWaylandReadbackGlxIntegration::screen() const
{
    return mScreen;
}

Window QWaylandReadbackGlxIntegration::rootWindow() const
{
    return mRootWindow;
}

QWaylandDisplay * QWaylandReadbackGlxIntegration::waylandDisplay() const
{
    return mWaylandDisplay;
}
