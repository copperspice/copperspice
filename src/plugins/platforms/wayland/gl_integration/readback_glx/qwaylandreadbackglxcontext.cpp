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

#include "qwaylandreadbackglxcontext.h"

#include "qwaylandshmsurface.h"
#include "qwaylandreadbackglxwindow.h"

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

QWaylandReadbackGlxContext::QWaylandReadbackGlxContext(QWaylandReadbackGlxIntegration *glxIntegration, QWaylandReadbackGlxWindow *window)
    : QPlatformGLContext()
    , mGlxIntegration(glxIntegration)
    , mWindow(window)
    , mBuffer(0)
    , mPixmap(0)
    , mConfig(qglx_findConfig(glxIntegration->xDisplay(),glxIntegration->screen(),window->widget()->platformWindowFormat(),GLX_PIXMAP_BIT))
    , mGlxPixmap(0)
{
    XVisualInfo *visualInfo = glXGetVisualFromFBConfig(glxIntegration->xDisplay(),mConfig);
    mContext = glXCreateContext(glxIntegration->xDisplay(),visualInfo,0,TRUE);

    geometryChanged();
}

void QWaylandReadbackGlxContext::makeCurrent()
{
    QPlatformGLContext::makeCurrent();

    glXMakeCurrent(mGlxIntegration->xDisplay(),mGlxPixmap,mContext);
}

void QWaylandReadbackGlxContext::doneCurrent()
{
    QPlatformGLContext::doneCurrent();
}

void QWaylandReadbackGlxContext::swapBuffers()
{
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
    mWindow->waitForFrameSync();

}

void * QWaylandReadbackGlxContext::getProcAddress(const QString &procName)
{
    return (void *) glXGetProcAddress(reinterpret_cast<GLubyte *>(procName.toLatin1().data()));
}

QPlatformWindowFormat QWaylandReadbackGlxContext::platformWindowFormat() const
{
    return qglx_platformWindowFromGLXFBConfig(mGlxIntegration->xDisplay(),mConfig,mContext);
}

void QWaylandReadbackGlxContext::geometryChanged()
{
    QSize size(mWindow->geometry().size());
    if (size.isEmpty()) {
        //QGLWidget wants a context for a window without geometry
        size = QSize(1,1);
    }

    mWindow->waitForFrameSync();

    delete mBuffer;
    //XFreePixmap deletes the glxPixmap as well
    if (mPixmap) {
        XFreePixmap(mGlxIntegration->xDisplay(),mPixmap);
    }

    mBuffer = new QWaylandShmBuffer(mGlxIntegration->waylandDisplay(),size,QImage::Format_ARGB32);
    mWindow->attach(mBuffer);
    int depth = XDefaultDepth(mGlxIntegration->xDisplay(),mGlxIntegration->screen());
    mPixmap = XCreatePixmap(mGlxIntegration->xDisplay(),mGlxIntegration->rootWindow(),size.width(),size.height(),depth);
    XSync(mGlxIntegration->xDisplay(),False);

    mGlxPixmap = glXCreatePixmap(mGlxIntegration->xDisplay(),mConfig,mPixmap,0);

    if (!mGlxPixmap) {
        qDebug() << "Could not make egl surface out of pixmap :(";
    }
}
