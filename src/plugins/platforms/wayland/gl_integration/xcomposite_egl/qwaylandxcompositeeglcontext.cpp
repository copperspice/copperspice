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

#include "qwaylandxcompositeeglcontext.h"

#include "qwaylandxcompositeeglwindow.h"
#include "qwaylandxcompositebuffer.h"

#include "wayland-xcomposite-client-protocol.h"
#include <QtCore/QDebug>

#include "qeglconvenience.h"
#include "qxlibeglintegration.h"

#include <X11/extensions/Xcomposite.h>

QWaylandXCompositeEGLContext::QWaylandXCompositeEGLContext(QWaylandXCompositeEGLIntegration *glxIntegration, QWaylandXCompositeEGLWindow *window)
    : QPlatformGLContext()
    , mEglIntegration(glxIntegration)
    , mWindow(window)
    , mBuffer(0)
    , mXWindow(0)
    , mConfig(q_configFromQPlatformWindowFormat(glxIntegration->eglDisplay(),window->widget()->platformWindowFormat(),true,EGL_WINDOW_BIT))
    , mWaitingForSync(false)
{
    QVector<EGLint> eglContextAttrs;
    eglContextAttrs.append(EGL_CONTEXT_CLIENT_VERSION); eglContextAttrs.append(2);
    eglContextAttrs.append(EGL_NONE);

    mContext = eglCreateContext(glxIntegration->eglDisplay(),mConfig,EGL_NO_CONTEXT,eglContextAttrs.constData());
    if (mContext == EGL_NO_CONTEXT) {
        qFatal("failed to find context");
    }

    geometryChanged();
}

void QWaylandXCompositeEGLContext::makeCurrent()
{
    QPlatformGLContext::makeCurrent();

    eglMakeCurrent(mEglIntegration->eglDisplay(),mEglWindowSurface,mEglWindowSurface,mContext);
}

void QWaylandXCompositeEGLContext::doneCurrent()
{
    QPlatformGLContext::doneCurrent();
    eglMakeCurrent(mEglIntegration->eglDisplay(),EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
}

void QWaylandXCompositeEGLContext::swapBuffers()
{
    QSize size = mWindow->geometry().size();

    eglSwapBuffers(mEglIntegration->eglDisplay(),mEglWindowSurface);
    mWindow->damage(QRect(QPoint(0,0),size));
    mWindow->waitForFrameSync();
}

void * QWaylandXCompositeEGLContext::getProcAddress(const QString &procName)
{
    return (void *)eglGetProcAddress(qPrintable(procName));
}

QPlatformWindowFormat QWaylandXCompositeEGLContext::platformWindowFormat() const
{
    return qt_qPlatformWindowFormatFromConfig(mEglIntegration->eglDisplay(),mConfig);
}

void QWaylandXCompositeEGLContext::sync_function(void *data)
{
    QWaylandXCompositeEGLContext *that = static_cast<QWaylandXCompositeEGLContext *>(data);
    that->mWaitingForSync = false;
}

void QWaylandXCompositeEGLContext::geometryChanged()
{
    QSize size(mWindow->geometry().size());
    if (size.isEmpty()) {
        //QGLWidget wants a context for a window without geometry
        size = QSize(1,1);
    }

    delete mBuffer;
    //XFreePixmap deletes the glxPixmap as well
    if (mXWindow) {
        XDestroyWindow(mEglIntegration->xDisplay(),mXWindow);
    }

    VisualID visualId = QXlibEglIntegration::getCompatibleVisualId(mEglIntegration->xDisplay(),mEglIntegration->eglDisplay(),mConfig);

    XVisualInfo visualInfoTemplate;
    memset(&visualInfoTemplate, 0, sizeof(XVisualInfo));
    visualInfoTemplate.visualid = visualId;

    int matchingCount = 0;
    XVisualInfo *visualInfo = XGetVisualInfo(mEglIntegration->xDisplay(), VisualIDMask, &visualInfoTemplate, &matchingCount);

    Colormap cmap = XCreateColormap(mEglIntegration->xDisplay(),mEglIntegration->rootWindow(),visualInfo->visual,AllocNone);

    XSetWindowAttributes a;
    a.colormap = cmap;
    mXWindow = XCreateWindow(mEglIntegration->xDisplay(), mEglIntegration->rootWindow(),0, 0, size.width(), size.height(),
                             0, visualInfo->depth, InputOutput, visualInfo->visual,
                             CWColormap, &a);

    XCompositeRedirectWindow(mEglIntegration->xDisplay(), mXWindow, CompositeRedirectManual);
    XMapWindow(mEglIntegration->xDisplay(), mXWindow);

    mEglWindowSurface = eglCreateWindowSurface(mEglIntegration->eglDisplay(),mConfig,mXWindow,0);
    if (mEglWindowSurface == EGL_NO_SURFACE) {
        qFatal("Could not make eglsurface");
    }

    XSync(mEglIntegration->xDisplay(),False);
    mBuffer = new QWaylandXCompositeBuffer(mEglIntegration->waylandXComposite(),
                                           (uint32_t)mXWindow,
                                           size,
                                           mEglIntegration->waylandDisplay()->argbVisual());
    mWindow->attach(mBuffer);
    wl_display_sync_callback(mEglIntegration->waylandDisplay()->wl_display(),
                             QWaylandXCompositeEGLContext::sync_function,
                             this);

    mWaitingForSync = true;
    wl_display_sync(mEglIntegration->waylandDisplay()->wl_display(),0);
    mEglIntegration->waylandDisplay()->flushRequests();
    while (mWaitingForSync) {
        mEglIntegration->waylandDisplay()->readEvents();
    }
}
