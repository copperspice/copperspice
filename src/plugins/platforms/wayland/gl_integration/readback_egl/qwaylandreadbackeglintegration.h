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

#ifndef QWAYLANDREADBACKEGLINTEGRATION_H
#define QWAYLANDREADBACKEGLINTEGRATION_H

#include "gl_integration/qwaylandglintegration.h"

#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QtGui/QWidget>

#include <X11/Xlib.h>

#include <EGL/egl.h>

class QWaylandReadbackEglIntegration : public QWaylandGLIntegration
{
public:
    QWaylandReadbackEglIntegration(QWaylandDisplay *display);
    ~QWaylandReadbackEglIntegration();

    void initialize();
    QWaylandWindow *createEglWindow(QWidget *widget);

    QWaylandDisplay *waylandDisplay() const;
    Display *xDisplay() const;
    Window rootWindow() const;
    int depth() const;

    EGLDisplay eglDisplay();

private:
    QWaylandDisplay *mWaylandDisplay;
    Display *mDisplay;
    int mScreen;
    Window mRootWindow;
    EGLDisplay mEglDisplay;

};

#endif // QWAYLANDREADBACKEGLINTEGRATION_H
