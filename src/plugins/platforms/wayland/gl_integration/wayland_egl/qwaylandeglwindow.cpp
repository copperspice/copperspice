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

#include "qwaylandeglwindow.h"

#include "qwaylandscreen.h"
#include "qwaylandglcontext.h"

QWaylandEglWindow::QWaylandEglWindow(QWidget *window)
    : QWaylandWindow(window)
    , mGLContext(0)
    , mWaylandEglWindow(0)
{
    mEglIntegration = static_cast<QWaylandEglIntegration *>(mDisplay->eglIntegration());
    //super creates a new surface
    newSurfaceCreated();
}

QWaylandEglWindow::~QWaylandEglWindow()
{
    delete mGLContext;
}

QWaylandWindow::WindowType QWaylandEglWindow::windowType() const
{
    return QWaylandWindow::Egl;
}

void QWaylandEglWindow::setGeometry(const QRect &rect)
{
    QWaylandWindow::setGeometry(rect);
    if (mWaylandEglWindow) {
        wl_egl_window_resize(mWaylandEglWindow,rect.width(),rect.height(),0,0);
    }
}

void QWaylandEglWindow::setParent(const QPlatformWindow *parent)
{
    const QWaylandWindow *wParent = static_cast<const QWaylandWindow *>(parent);

    mParentWindow = wParent;
}

QPlatformGLContext * QWaylandEglWindow::glContext() const
{
    if (!mGLContext) {
        QWaylandEglWindow *that = const_cast<QWaylandEglWindow *>(this);
        that->mGLContext = new QWaylandGLContext(mEglIntegration->eglDisplay(),widget()->platformWindowFormat());

        EGLNativeWindowType window(reinterpret_cast<EGLNativeWindowType>(mWaylandEglWindow));
        EGLSurface surface = eglCreateWindowSurface(mEglIntegration->eglDisplay(),mGLContext->eglConfig(),window,NULL);
        that->mGLContext->setEglSurface(surface);
    }

    return mGLContext;
}

void QWaylandEglWindow::newSurfaceCreated()
{
    if (mWaylandEglWindow) {
        wl_egl_window_destroy(mWaylandEglWindow);
    }
    wl_visual *visual = QWaylandScreen::waylandScreenFromWidget(widget())->visual();
    QSize size = geometry().size();
    if (!size.isValid())
        size = QSize(0,0);

    mWaylandEglWindow = wl_egl_window_create(mSurface,size.width(),size.height(),visual);
    if (mGLContext) {
        EGLNativeWindowType window(reinterpret_cast<EGLNativeWindowType>(mWaylandEglWindow));
        EGLSurface surface = eglCreateWindowSurface(mEglIntegration->eglDisplay(),mGLContext->eglConfig(),window,NULL);
        mGLContext->setEglSurface(surface);
    }
}
