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

#include "qwaylandintegration.h"

#include "qwaylanddisplay.h"
#include "qwaylandshmsurface.h"
#include "qwaylandshmwindow.h"
#include "qwaylandnativeinterface.h"
#include "qwaylandclipboard.h"

#include "qgenericunixfontdatabase.h"

#include <QtGui/QWindowSystemInterface>
#include <QtGui/QPlatformCursor>
#include <QtGui/QPlatformWindowFormat>

#include <qpixmap_raster_p.h>

#ifdef QT_WAYLAND_GL_SUPPORT
#include "gl_integration/qwaylandglintegration.h"
#include "gl_integration/qwaylandglwindowsurface.h"
#include <qpixmapdata_gl_p.h>
#endif

QWaylandIntegration::QWaylandIntegration(bool useOpenGL)
    : mFontDb(new QGenericUnixFontDatabase())
    , mDisplay(new QWaylandDisplay())
    , mUseOpenGL(useOpenGL)
    , mNativeInterface(new QWaylandNativeInterface)
    , mClipboard(0)
{
}

QPlatformNativeInterface * QWaylandIntegration::nativeInterface() const
{
    return mNativeInterface;
}

QList<QPlatformScreen *>
QWaylandIntegration::screens() const
{
    return mDisplay->screens();
}

bool QWaylandIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    case OpenGL: return hasOpenGL();
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPixmapData *QWaylandIntegration::createPixmapData(QPixmapData::PixelType type) const
{
#ifdef QT_WAYLAND_GL_SUPPORT
    if (mUseOpenGL)
        return new QGLPixmapData(type);
#endif
    return new QRasterPixmapData(type);
}

QPlatformWindow *QWaylandIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
#ifdef QT_WAYLAND_GL_SUPPORT
    bool useOpenGL = mUseOpenGL || (widget->platformWindowFormat().windowApi() == QPlatformWindowFormat::OpenGL);
    if (useOpenGL)
        return mDisplay->eglIntegration()->createEglWindow(widget);
#endif
    return new QWaylandShmWindow(widget);
}

QWindowSurface *QWaylandIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
    Q_UNUSED(winId);
#ifdef QT_WAYLAND_GL_SUPPORT
    bool useOpenGL = mUseOpenGL || (widget->platformWindowFormat().windowApi() == QPlatformWindowFormat::OpenGL);
    if (useOpenGL)
        return new QWaylandGLWindowSurface(widget);
#endif
    return new QWaylandShmWindowSurface(widget);
}

QPlatformFontDatabase *QWaylandIntegration::fontDatabase() const
{
    return mFontDb;
}

bool QWaylandIntegration::hasOpenGL() const
{
#ifdef QT_WAYLAND_GL_SUPPORT
    return true;
#else
    return false;
#endif
}

QPlatformClipboard *QWaylandIntegration::clipboard() const
{
    if (!mClipboard)
        mClipboard = new QWaylandClipboard(mDisplay);
    return mClipboard;
}
