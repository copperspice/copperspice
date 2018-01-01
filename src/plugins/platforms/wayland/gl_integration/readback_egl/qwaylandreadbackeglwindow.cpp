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

#include "qwaylandreadbackeglwindow.h"

#include "qwaylandreadbackeglcontext.h"

QWaylandReadbackEglWindow::QWaylandReadbackEglWindow(QWidget *window, QWaylandReadbackEglIntegration *eglIntegration)
    : QWaylandShmWindow(window)
    , mEglIntegration(eglIntegration)
    , mContext(0)
{
}

QWaylandWindow::WindowType QWaylandReadbackEglWindow::windowType() const
{
    //We'r lying, maybe we should add a type, but for now it will do
    //since this is primarly used by the windowsurface.
    return QWaylandWindow::Egl;
}

QPlatformGLContext *QWaylandReadbackEglWindow::glContext() const
{
    if (!mContext) {
        QWaylandReadbackEglWindow *that = const_cast<QWaylandReadbackEglWindow *>(this);
        that->mContext = new QWaylandReadbackEglContext(mEglIntegration,that);
    }
    return mContext;
}

void QWaylandReadbackEglWindow::setGeometry(const QRect &rect)
{
    QPlatformWindow::setGeometry(rect);

    if (mContext)
        mContext->geometryChanged();
}

