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

#include <qwaylandreadbackeglcontext.h>
#include <qeglconvenience.h>
#include <QtOpenGL/QGLContext>
#include <qglextensions_p.h>
#include <qwaylandshmsurface.h>
#include <QtCore/QDebug>

static inline void qgl_byteSwapImage(QImage &img, GLenum pixel_type)
{
    const int width = img.width();
    const int height = img.height();

    if (pixel_type == GL_UNSIGNED_INT_8_8_8_8_REV
        || (pixel_type == GL_UNSIGNED_BYTE && QSysInfo::ByteOrder == QSysInfo::LittleEndian))
    {
        for (int i = 0; i < height; ++i) {
            uint *p = (uint *) img.scanLine(i);
            for (int x = 0; x < width; ++x)
                p[x] = ((p[x] << 16) & 0xff0000) | ((p[x] >> 16) & 0xff) | (p[x] & 0xff00ff00);
        }
    } else {
        for (int i = 0; i < height; ++i) {
            uint *p = (uint *) img.scanLine(i);
            for (int x = 0; x < width; ++x)
                p[x] = (p[x] << 8) | ((p[x] >> 24) & 0xff);
        }
    }
}

QWaylandReadbackEglContext::QWaylandReadbackEglContext(QWaylandReadbackEglIntegration *eglIntegration, QWaylandReadbackEglWindow *window)
    : mEglIntegration(eglIntegration)
    , mWindow(window)
    , mBuffer(0)
    , mPixmap(0)
    , mConfig(q_configFromQPlatformWindowFormat(eglIntegration->eglDisplay(),window->widget()->platformWindowFormat(),true,EGL_PIXMAP_BIT))
    , mPixmapSurface(EGL_NO_SURFACE)
{
    QVector<EGLint> eglContextAttrs;
    eglContextAttrs.append(EGL_CONTEXT_CLIENT_VERSION);
    eglContextAttrs.append(2);
    eglContextAttrs.append(EGL_NONE);

    mContext = eglCreateContext(eglIntegration->eglDisplay(),mConfig,0,eglContextAttrs.constData());

    geometryChanged();
}

QWaylandReadbackEglContext::~QWaylandReadbackEglContext()
{
    eglDestroyContext(mEglIntegration->eglDisplay(),mContext);
}

void QWaylandReadbackEglContext::makeCurrent()
{
    QPlatformGLContext::makeCurrent();

    mWindow->waitForFrameSync();

    eglMakeCurrent(mEglIntegration->eglDisplay(),mPixmapSurface,mPixmapSurface,mContext);
}

void QWaylandReadbackEglContext::doneCurrent()
{
    QPlatformGLContext::doneCurrent();
    eglMakeCurrent(mEglIntegration->eglDisplay(),EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
}

void QWaylandReadbackEglContext::swapBuffers()
{
    eglSwapBuffers(mEglIntegration->eglDisplay(),mPixmapSurface);

    if (QPlatformGLContext::currentContext() != this) {
        makeCurrent();
    }

    QSize size = mWindow->geometry().size();

    QImage img(size,QImage::Format_ARGB32);
    const uchar *constBits = img.bits();
    void *pixels = const_cast<uchar *>(constBits);

    glReadPixels(0,0, size.width(), size.height(), GL_RGBA,GL_UNSIGNED_BYTE, pixels);

    img = img.mirrored();
    qgl_byteSwapImage(img,GL_UNSIGNED_INT_8_8_8_8_REV);
    constBits = img.bits();

    const uchar *constDstBits = mBuffer->image()->bits();
    uchar *dstBits = const_cast<uchar *>(constDstBits);
    memcpy(dstBits,constBits,(img.width()*4) * img.height());


    mWindow->damage(QRegion(QRect(QPoint(0,0),size)));
}

void * QWaylandReadbackEglContext::getProcAddress(const QString &procName)
{
    return (void *) eglGetProcAddress(procName.toLatin1().data());
}

QPlatformWindowFormat QWaylandReadbackEglContext::platformWindowFormat() const
{
    return qt_qPlatformWindowFormatFromConfig(mEglIntegration->eglDisplay(),mConfig);
}

void QWaylandReadbackEglContext::geometryChanged()
{
    QSize size(mWindow->geometry().size());
    if (size.isEmpty()) {
        //QGLWidget wants a context for a window without geometry
        size = QSize(1,1);
    }

    mWindow->waitForFrameSync();

    delete mBuffer;
    if (mPixmap)
        XFreePixmap(mEglIntegration->xDisplay(),mPixmap);

    mBuffer = new QWaylandShmBuffer(mEglIntegration->waylandDisplay(),size,QImage::Format_ARGB32);
    mWindow->attach(mBuffer);
    mPixmap = XCreatePixmap(mEglIntegration->xDisplay(),mEglIntegration->rootWindow(),size.width(),size.height(),mEglIntegration->depth());
    XSync(mEglIntegration->xDisplay(),False);

    mPixmapSurface = eglCreatePixmapSurface(mEglIntegration->eglDisplay(),mConfig,mPixmap,0);
    if (mPixmapSurface == EGL_NO_SURFACE) {
        qDebug() << "Could not make egl surface out of pixmap :(";
    }
}
