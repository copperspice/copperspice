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

#ifndef QWAYLANDEGLWINDOW_H
#define QWAYLANDEGLWINDOW_H

#include "qwaylandwindow.h"
#include "qwaylandeglinclude.h"
#include "qwaylandeglintegration.h"

class QWaylandGLContext;

class QWaylandEglWindow : public QWaylandWindow
{
public:
    QWaylandEglWindow(QWidget *window);
    ~QWaylandEglWindow();
    WindowType windowType() const;
    void setGeometry(const QRect &rect);
    void setParent(const QPlatformWindow *parent);
    QPlatformGLContext *glContext() const;
protected:
    void newSurfaceCreated();
private:
    QWaylandEglIntegration *mEglIntegration;
    QWaylandGLContext *mGLContext;
    struct wl_egl_window *mWaylandEglWindow;

    const QWaylandWindow *mParentWindow;
};

#endif // QWAYLANDEGLWINDOW_H
