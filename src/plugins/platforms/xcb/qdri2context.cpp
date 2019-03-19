/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include "qdri2context.h"

#include "qxcbwindow.h"
#include "qxcbconnection.h"

#include <QtCore/QDebug>
#include <QtGui/QWidget>

#include <xcb/dri2.h>
#include <xcb/xfixes.h>

#define MESA_EGL_NO_X11_HEADERS
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class QDri2ContextPrivate
{
public:
    QDri2ContextPrivate(QXcbWindow *window)
        : qXcbWindow(window)
        , windowFormat(window->widget()->platformWindowFormat())
        , image(0)
    {
    }

    xcb_window_t xcbWindow() { return qXcbWindow->window(); }
    xcb_connection_t *xcbConnection() { return qXcbWindow->xcb_connection(); }

    QXcbWindow *qXcbWindow;
    QPlatformWindowFormat windowFormat;

    EGLContext eglContext;

    EGLImageKHR image;

    GLuint fbo;
    GLuint rbo;
    GLuint depth;

    QSize size;
};

QDri2Context::QDri2Context(QXcbWindow *window)
    : d_ptr(new QDri2ContextPrivate(window))
{
    Q_D(QDri2Context);

    static const EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };

    eglBindAPI(EGL_OPENGL_ES_API);

    EGLContext shareContext = EGL_NO_CONTEXT;
    if (window->widget()->platformWindowFormat().sharedGLContext()) {
        QDri2Context *context = static_cast<QDri2Context *>(window->widget()->platformWindowFormat().sharedGLContext());
        shareContext = context->d_func()->eglContext;
    }
    d->eglContext = eglCreateContext(EGL_DISPLAY_FROM_XCB(d->qXcbWindow), NULL,
                                               shareContext, contextAttribs);

    if (d->eglContext == EGL_NO_CONTEXT) {
        qDebug() << "No eglContext!" << eglGetError();
    }

    EGLBoolean makeCurrentSuccess = eglMakeCurrent(EGL_DISPLAY_FROM_XCB(d->qXcbWindow),EGL_NO_SURFACE,EGL_NO_SURFACE,d->eglContext);
    if (!makeCurrentSuccess) {
        qDebug() << "eglMakeCurrent failed!" << eglGetError();
    }

    xcb_dri2_create_drawable (d->xcbConnection(), d->xcbWindow());

    glGenFramebuffers(1,&d->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,d->fbo);
    glActiveTexture(GL_TEXTURE0);

    glGenRenderbuffers(1, &d->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, d->rbo);

    glGenRenderbuffers(1,&d->depth);
    glBindRenderbuffer(GL_RENDERBUFFER, d->depth);

    resize(d->qXcbWindow->widget()->geometry().size());

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, d->rbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERER,d->depth);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERER,d->depth);

    //restore the old current context
    const QPlatformGLContext *currentContext = QPlatformGLContext::currentContext();
    if (currentContext)
        const_cast<QPlatformGLContext*>(currentContext)->makeCurrent();
}

QDri2Context::~QDri2Context()
{
    //cleanup
}

void QDri2Context::makeCurrent()
{
    QPlatformGLContext::makeCurrent();
    Q_D(QDri2Context);

    eglMakeCurrent(EGL_DISPLAY_FROM_XCB(d->qXcbWindow),EGL_NO_SURFACE,EGL_NO_SURFACE,d->eglContext);
    glBindFramebuffer(GL_FRAMEBUFFER,d->fbo);

}

void QDri2Context::doneCurrent()
{
    QPlatformGLContext::doneCurrent();
    Q_D(QDri2Context);
    eglMakeCurrent(EGL_DISPLAY_FROM_XCB(d->qXcbWindow),EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
}

void QDri2Context::swapBuffers()
{
    Q_D(QDri2Context);
    xcb_rectangle_t rectangle;
    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.width = d->qXcbWindow->widget()->geometry().width();
    rectangle.height = d->qXcbWindow->widget()->geometry().height();

    xcb_xfixes_region_t xfixesRegion = xcb_generate_id(d->xcbConnection());
    xcb_xfixes_create_region(d->xcbConnection(), xfixesRegion,
                             1, &rectangle);

    xcb_dri2_copy_region_cookie_t cookie = xcb_dri2_copy_region_unchecked(d->xcbConnection(),
                                                                          d->qXcbWindow->window(),
                                                                          xfixesRegion,
                                                                          XCB_DRI2_ATTACHMENT_BUFFER_FRONT_LEFT,
                                                                          XCB_DRI2_ATTACHMENT_BUFFER_BACK_LEFT);

    xcb_dri2_copy_region_reply_t *reply = xcb_dri2_copy_region_reply(d->xcbConnection(),cookie,NULL);

    //cleanup
    delete reply;
    xcb_xfixes_destroy_region(d->xcbConnection(), xfixesRegion);

}

void * QDri2Context::getProcAddress(const QString &procName)
{
    return (void *)eglGetProcAddress(qPrintable(procName));
}

void QDri2Context::resize(const QSize &size)
{
    Q_D(QDri2Context);
    d->size= size;

    glBindFramebuffer(GL_FRAMEBUFFER,d->fbo);

    xcb_dri2_dri2_buffer_t *backBfr = backBuffer();

    if (d->image) {
        qDebug() << "destroing image";
        eglDestroyImageKHR(EGL_DISPLAY_FROM_XCB(d->qXcbWindow),d->image);
    }

    EGLint imgAttribs[] = {
        EGL_WIDTH,                      d->size.width(),
        EGL_HEIGHT,                     d->size.height(),
        EGL_DRM_BUFFER_STRIDE_MESA,     backBfr->pitch /4,
        EGL_DRM_BUFFER_FORMAT_MESA,     EGL_DRM_BUFFER_FORMAT_ARGB32_MESA,
        EGL_NONE
    };

    d->image = eglCreateImageKHR(EGL_DISPLAY_FROM_XCB(d->qXcbWindow),
                                EGL_NO_CONTEXT,
                                EGL_DRM_BUFFER_MESA,
                                (EGLClientBuffer) backBfr->name,
                                imgAttribs);

    glBindRenderbuffer(GL_RENDERBUFFER, d->rbo);
    glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER,
                                           d->image);

    glBindRenderbuffer(GL_RENDERBUFFER, d->depth);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8_OES,d->size.width(), d->size.height());

}

QPlatformWindowFormat QDri2Context::platformWindowFormat() const
{
    Q_D(const QDri2Context);
    return d->windowFormat;
}

xcb_dri2_dri2_buffer_t * QDri2Context::backBuffer()
{
    Q_D(QDri2Context);

    unsigned int backBufferAttachment = XCB_DRI2_ATTACHMENT_BUFFER_BACK_LEFT;
    xcb_dri2_get_buffers_cookie_t cookie = xcb_dri2_get_buffers_unchecked (d->xcbConnection(),
                                                                           d->xcbWindow(),
                                                                           1, 1, &backBufferAttachment);

    xcb_dri2_get_buffers_reply_t *reply = xcb_dri2_get_buffers_reply (d->xcbConnection(), cookie, NULL);
    if (!reply) {
        qDebug() << "failed to get buffers reply";
        return 0;
    }

    xcb_dri2_dri2_buffer_t *buffers = xcb_dri2_get_buffers_buffers (reply);
    if (!buffers) {
        qDebug() << "failed to get buffers";
        return 0;
    }

    Q_ASSERT(reply->count == 1);

    delete reply;

    return buffers;
}

void * QDri2Context::eglContext() const
{
    Q_D(const QDri2Context);
    return d->eglContext;
}
