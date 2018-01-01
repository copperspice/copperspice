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

#include "qwaylandxcompositeglxcontext.h"

#include "qwaylandxcompositeglxwindow.h"
#include "qwaylandxcompositebuffer.h"

#include "wayland-xcomposite-client-protocol.h"
#include <QtCore/QDebug>

#include <X11/extensions/Xcomposite.h>

QWaylandXCompositeGLXContext::QWaylandXCompositeGLXContext(QWaylandXCompositeGLXIntegration *glxIntegration, QWaylandXCompositeGLXWindow *window)
    : QPlatformGLContext()
    , mGlxIntegration(glxIntegration)
    , mWindow(window)
    , mBuffer(0)
    , mXWindow(0)
    , mConfig(qglx_findConfig(glxIntegration->xDisplay(),glxIntegration->screen(),window->widget()->platformWindowFormat()))
    , mWaitingForSyncCallback(false)
{
    XVisualInfo *visualInfo = glXGetVisualFromFBConfig(glxIntegration->xDisplay(),mConfig);
    mContext = glXCreateContext(glxIntegration->xDisplay(),visualInfo,0,TRUE);

    geometryChanged();
}

void QWaylandXCompositeGLXContext::makeCurrent()
{
    QPlatformGLContext::makeCurrent();
    glXMakeCurrent(mGlxIntegration->xDisplay(),mXWindow,mContext);
}

void QWaylandXCompositeGLXContext::doneCurrent()
{
    glXMakeCurrent(mGlxIntegration->xDisplay(),0,0);
    QPlatformGLContext::doneCurrent();
}

void QWaylandXCompositeGLXContext::swapBuffers()
{
    QSize size = mWindow->geometry().size();

    glXSwapBuffers(mGlxIntegration->xDisplay(),mXWindow);
    mWindow->damage(QRect(QPoint(0,0),size));
    mWindow->waitForFrameSync();
}

void * QWaylandXCompositeGLXContext::getProcAddress(const QString &procName)
{
    return (void *) glXGetProcAddress(reinterpret_cast<GLubyte *>(procName.toLatin1().data()));
}

QPlatformWindowFormat QWaylandXCompositeGLXContext::platformWindowFormat() const
{
    return qglx_platformWindowFromGLXFBConfig(mGlxIntegration->xDisplay(),mConfig,mContext);
}

void QWaylandXCompositeGLXContext::sync_function(void *data)
{
    QWaylandXCompositeGLXContext *that = static_cast<QWaylandXCompositeGLXContext *>(data);
    that->mWaitingForSyncCallback = false;
}

void QWaylandXCompositeGLXContext::waitForSync()
{
    wl_display_sync_callback(mGlxIntegration->waylandDisplay()->wl_display(),
                             QWaylandXCompositeGLXContext::sync_function,
                             this);
    mWaitingForSyncCallback = true;
    wl_display_sync(mGlxIntegration->waylandDisplay()->wl_display(),0);
    mGlxIntegration->waylandDisplay()->flushRequests();
    while (mWaitingForSyncCallback) {
        mGlxIntegration->waylandDisplay()->readEvents();
    }
}

void QWaylandXCompositeGLXContext::geometryChanged()
{
    QSize size(mWindow->geometry().size());
    if (size.isEmpty()) {
        //QGLWidget wants a context for a window without geometry
        size = QSize(1,1);
    }

    delete mBuffer;
    //XFreePixmap deletes the glxPixmap as well
    if (mXWindow) {
        XDestroyWindow(mGlxIntegration->xDisplay(),mXWindow);
    }

    XVisualInfo *visualInfo = glXGetVisualFromFBConfig(mGlxIntegration->xDisplay(),mConfig);
    Colormap cmap = XCreateColormap(mGlxIntegration->xDisplay(),mGlxIntegration->rootWindow(),visualInfo->visual,AllocNone);

    XSetWindowAttributes a;
    a.background_pixel = WhitePixel(mGlxIntegration->xDisplay(), mGlxIntegration->screen());
    a.border_pixel = BlackPixel(mGlxIntegration->xDisplay(), mGlxIntegration->screen());
    a.colormap = cmap;
    mXWindow = XCreateWindow(mGlxIntegration->xDisplay(), mGlxIntegration->rootWindow(),0, 0, size.width(), size.height(),
                             0, visualInfo->depth, InputOutput, visualInfo->visual,
                             CWBackPixel|CWBorderPixel|CWColormap, &a);

    XCompositeRedirectWindow(mGlxIntegration->xDisplay(), mXWindow, CompositeRedirectManual);
    XMapWindow(mGlxIntegration->xDisplay(), mXWindow);

    XSync(mGlxIntegration->xDisplay(),False);
    mBuffer = new QWaylandXCompositeBuffer(mGlxIntegration->waylandXComposite(),
                                           (uint32_t)mXWindow,
                                           size,
                                           mGlxIntegration->waylandDisplay()->argbVisual());
    mWindow->attach(mBuffer);
    waitForSync();
}
