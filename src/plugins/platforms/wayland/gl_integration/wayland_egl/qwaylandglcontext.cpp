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

#include "qwaylandglcontext.h"

#include "qwaylanddisplay.h"
#include "qwaylandwindow.h"

#include "../../../eglconvenience/qeglconvenience.h"

#include <QtGui/QPlatformGLContext>
#include <QtGui/QPlatformWindowFormat>
#include <QtCore/QMutex>

QWaylandGLContext::QWaylandGLContext(EGLDisplay eglDisplay, const QPlatformWindowFormat &format)
    : QPlatformGLContext()
    , mEglDisplay(eglDisplay)
    , mSurface(EGL_NO_SURFACE)
    , mConfig(q_configFromQPlatformWindowFormat(mEglDisplay,format,true))
    , mFormat(qt_qPlatformWindowFormatFromConfig(mEglDisplay,mConfig))
{
    QPlatformGLContext *sharePlatformContext = 0;
    sharePlatformContext = format.sharedGLContext();
    mFormat.setSharedContext(sharePlatformContext);
    EGLContext shareEGLContext = EGL_NO_CONTEXT;
    if (sharePlatformContext)
        shareEGLContext = static_cast<const QWaylandGLContext*>(sharePlatformContext)->mContext;

    eglBindAPI(EGL_OPENGL_ES_API);

    QVector<EGLint> eglContextAttrs;
    eglContextAttrs.append(EGL_CONTEXT_CLIENT_VERSION);
    eglContextAttrs.append(2);
    eglContextAttrs.append(EGL_NONE);

    mContext = eglCreateContext(mEglDisplay, mConfig,
                                shareEGLContext, eglContextAttrs.constData());
}

QWaylandGLContext::QWaylandGLContext()
    : QPlatformGLContext()
    , mEglDisplay(0)
    , mContext(EGL_NO_CONTEXT)
    , mSurface(EGL_NO_SURFACE)
    , mConfig(0)
{ }

QWaylandGLContext::~QWaylandGLContext()
{
    eglDestroyContext(mEglDisplay,mContext);
}

void QWaylandGLContext::makeCurrent()
{
    QPlatformGLContext::makeCurrent();
    if (mSurface == EGL_NO_SURFACE) {
        qWarning("makeCurrent with EGL_NO_SURFACE");
    }
    eglMakeCurrent(mEglDisplay, mSurface, mSurface, mContext);
}

void QWaylandGLContext::doneCurrent()
{
    QPlatformGLContext::doneCurrent();
    eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void QWaylandGLContext::swapBuffers()
{
    eglSwapBuffers(mEglDisplay,mSurface);
}

void *QWaylandGLContext::getProcAddress(const QString &string)
{
    return (void *) eglGetProcAddress(string.toLatin1().data());
}

void QWaylandGLContext::setEglSurface(EGLSurface surface)
{
    bool wasCurrent = false;
    if (QPlatformGLContext::currentContext() == this) {
        wasCurrent = true;
        doneCurrent();
    }
    mSurface = surface;
    if (wasCurrent) {
        makeCurrent();
    }
}

EGLConfig QWaylandGLContext::eglConfig() const
{
    return mConfig;
}

