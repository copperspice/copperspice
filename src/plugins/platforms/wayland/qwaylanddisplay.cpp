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

#include "qwaylanddisplay.h"

#include "qwaylandwindow.h"
#include "qwaylandscreen.h"
#include "qwaylandcursor.h"
#include "qwaylandinputdevice.h"
#include "qwaylandclipboard.h"

#ifdef QT_WAYLAND_GL_SUPPORT
#include "gl_integration/qwaylandglintegration.h"
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
#include "windowmanager_integration/qwaylandwindowmanagerintegration.h"
#endif

#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/QApplication>
#include <QtGui/private/qapplication_p.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

struct wl_surface *QWaylandDisplay::createSurface(void *handle)
{
    struct wl_surface * surface = wl_compositor_create_surface(mCompositor);
    wl_surface_set_user_data(surface, handle);
    return surface;
}

struct wl_buffer *QWaylandDisplay::createShmBuffer(int fd,
                                                   int width, int height,
                                                   uint32_t stride,
                                                   struct wl_visual *visual)
{
    return wl_shm_create_buffer(mShm, fd, width, height, stride, visual);
}

struct wl_visual *QWaylandDisplay::rgbVisual()
{
    return rgb_visual;
}

struct wl_visual *QWaylandDisplay::argbVisual()
{
    return argb_visual;
}

struct wl_visual *QWaylandDisplay::argbPremultipliedVisual()
{
    return premultiplied_argb_visual;
}

#ifdef QT_WAYLAND_GL_SUPPORT
QWaylandGLIntegration * QWaylandDisplay::eglIntegration()
{
    return mEglIntegration;
}
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
QWaylandWindowManagerIntegration *QWaylandDisplay::windowManagerIntegration()
{
    return mWindowManagerIntegration;
}
#endif

void QWaylandDisplay::shellHandleConfigure(void *data, struct wl_shell *shell,
                                           uint32_t time, uint32_t edges,
                                           struct wl_surface *surface,
                                           int32_t width, int32_t height)
{
    Q_UNUSED(data);
    Q_UNUSED(shell);
    Q_UNUSED(time);
    Q_UNUSED(edges);
    QWaylandWindow *ww = (QWaylandWindow *) wl_surface_get_user_data(surface);

    ww->configure(time, edges, 0, 0, width, height);
}

const struct wl_shell_listener QWaylandDisplay::shellListener = {
    QWaylandDisplay::shellHandleConfigure,
};

QWaylandDisplay::QWaylandDisplay(void)
    : argb_visual(0), premultiplied_argb_visual(0), rgb_visual(0)
{
    mDisplay = wl_display_connect(NULL);
    if (mDisplay == NULL) {
        qErrnoWarning(errno, "Failed to create display");
        qFatal("No wayland connection available.");
    }

    wl_display_add_global_listener(mDisplay, QWaylandDisplay::displayHandleGlobal, this);

#ifdef QT_WAYLAND_GL_SUPPORT
    mEglIntegration = QWaylandGLIntegration::createGLIntegration(this);
#endif

#ifdef QT_WAYLAND_WINDOWMANAGER_SUPPORT
    mWindowManagerIntegration = QWaylandWindowManagerIntegration::createIntegration(this);
#endif

    blockingReadEvents();

    qRegisterMetaType<uint32_t>("uint32_t");

#ifdef QT_WAYLAND_GL_SUPPORT
    mEglIntegration->initialize();
#endif

    connect(QAbstractEventDispatcher::instance(), SIGNAL(aboutToBlock()), this, SLOT(flushRequests()));

    mFd = wl_display_get_fd(mDisplay, sourceUpdate, this);

    mReadNotifier = new QSocketNotifier(mFd, QSocketNotifier::Read, this);
    connect(mReadNotifier, SIGNAL(activated(int)), this, SLOT(readEvents()));

    waitForScreens();
}

QWaylandDisplay::~QWaylandDisplay(void)
{
    close(mFd);
#ifdef QT_WAYLAND_GL_SUPPORT
    delete mEglIntegration;
#endif
    wl_display_destroy(mDisplay);
}

void QWaylandDisplay::createNewScreen(struct wl_output *output, QRect geometry)
{
    QWaylandScreen *waylandScreen = new QWaylandScreen(this,output,geometry);
    mScreens.append(waylandScreen);
}

void QWaylandDisplay::syncCallback(wl_display_sync_func_t func, void *data)
{
    wl_display_sync_callback(mDisplay, func, data);
}

void QWaylandDisplay::frameCallback(wl_display_frame_func_t func, struct wl_surface *surface, void *data)
{
    wl_display_frame_callback(mDisplay, surface, func, data);
}

void QWaylandDisplay::flushRequests()
{
    if (mSocketMask & WL_DISPLAY_WRITABLE)
        wl_display_iterate(mDisplay, WL_DISPLAY_WRITABLE);
}

void QWaylandDisplay::readEvents()
{
// verify that there is still data on the socket
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(mFd, &fds);
    fd_set nds;
    FD_ZERO(&nds);
    fd_set rs = fds;
    fd_set ws = nds;
    fd_set es = nds;
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    int ret = ::select(mFd+1, &rs, &ws, &es, &timeout );

    if (ret <= 0) {
        //qDebug("QWaylandDisplay::readEvents() No data... blocking avoided");
        return;
    }

    wl_display_iterate(mDisplay, WL_DISPLAY_READABLE);
}

void QWaylandDisplay::blockingReadEvents()
{
    wl_display_iterate(mDisplay, WL_DISPLAY_READABLE);
}

int QWaylandDisplay::sourceUpdate(uint32_t mask, void *data)
{
    QWaylandDisplay *waylandDisplay = static_cast<QWaylandDisplay *>(data);
    waylandDisplay->mSocketMask = mask;

    return 0;
}

void QWaylandDisplay::outputHandleGeometry(void *data,
                                           wl_output *output,
                                           int32_t x, int32_t y,
                                           int32_t physicalWidth,
                                           int32_t physicalHeight,
                                           int subpixel,
                                           const char *make, const char *model)
{
    QWaylandDisplay *waylandDisplay = static_cast<QWaylandDisplay *>(data);
    QRect outputRect = QRect(x, y, physicalWidth, physicalHeight);
    waylandDisplay->createNewScreen(output,outputRect);
}

void QWaylandDisplay::mode(void *data,
             struct wl_output *wl_output,
             uint32_t flags,
             int width,
             int height,
             int refresh)
{
    Q_UNUSED(data);
    Q_UNUSED(wl_output);
    Q_UNUSED(flags);
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(refresh);
}

const struct wl_output_listener QWaylandDisplay::outputListener = {
    QWaylandDisplay::outputHandleGeometry,
    QWaylandDisplay::mode
};

const struct wl_compositor_listener QWaylandDisplay::compositorListener = {
    QWaylandDisplay::handleVisual,
};


void QWaylandDisplay::waitForScreens()
{
    flushRequests();
    while (mScreens.isEmpty())
        blockingReadEvents();
}

void QWaylandDisplay::displayHandleGlobal(struct wl_display *display,
                                          uint32_t id,
                                          const char *interface,
                                          uint32_t version,
                                          void *data)
{
    Q_UNUSED(display);
    QWaylandDisplay *that = static_cast<QWaylandDisplay *>(data);
    that->displayHandleGlobal(id, QByteArray(interface), version);
}

void QWaylandDisplay::displayHandleGlobal(uint32_t id,
                                          const QByteArray &interface,
                                          uint32_t version)
{
    Q_UNUSED(version);
    if (interface == "wl_output") {
        struct wl_output *output = wl_output_create(mDisplay, id, 1);
        wl_output_add_listener(output, &outputListener, this);
    } else if (interface == "wl_compositor") {
        mCompositor = wl_compositor_create(mDisplay, id, 1);
        wl_compositor_add_listener(mCompositor,
                                   &compositorListener, this);
    } else if (interface == "wl_shm") {
        mShm = wl_shm_create(mDisplay, id, 1);
    } else if (interface == "wl_shell"){
        mShell = wl_shell_create(mDisplay, id, 1);
        wl_shell_add_listener(mShell, &shellListener, this);
    } else if (interface == "wl_input_device") {
        QWaylandInputDevice *inputDevice =
            new QWaylandInputDevice(mDisplay, id);
        mInputDevices.append(inputDevice);
    } else if (interface == "wl_selection_offer") {
        QPlatformIntegration *plat = QApplicationPrivate::platformIntegration();
        QWaylandClipboard *clipboard = static_cast<QWaylandClipboard *>(plat->clipboard());
        clipboard->createSelectionOffer(id);
    }
}

void QWaylandDisplay::handleVisual(void *data,
                                   struct wl_compositor *compositor,
                                   uint32_t id, uint32_t token)
{
    QWaylandDisplay *self = static_cast<QWaylandDisplay *>(data);

    switch (token) {
    case WL_COMPOSITOR_VISUAL_ARGB32:
        self->argb_visual = wl_visual_create(self->mDisplay, id, 1);
        break;
    case WL_COMPOSITOR_VISUAL_PREMULTIPLIED_ARGB32:
        self->premultiplied_argb_visual =
            wl_visual_create(self->mDisplay, id, 1);
        break;
    case WL_COMPOSITOR_VISUAL_XRGB32:
        self->rgb_visual = wl_visual_create(self->mDisplay, id, 1);
        break;
    }
}
