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

#include <qxcbintegration.h>
#include <qxcbconnection.h>
#include <qxcbscreen.h>
#include <qxcbwindow.h>
#include <qxcbwindowsurface.h>
#include <qxcbnativeinterface.h>

#include <xcb/xcb.h>
#include <qpixmap_raster_p.h>
#include <qgenericunixfontdatabase.h>

#include <stdio.h>

#ifdef XCB_USE_EGL
#include <EGL/egl.h>
#endif

QXcbIntegration::QXcbIntegration()
    : m_connection(new QXcbConnection)
{
    foreach (QXcbScreen *screen, m_connection->screens())
        m_screens << screen;

    m_fontDatabase = new QGenericUnixFontDatabase();
    m_nativeInterface = new QXcbNativeInterface;
}

QXcbIntegration::~QXcbIntegration()
{
    delete m_connection;
}

bool QXcbIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    case OpenGL: return hasOpenGL();
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPixmapData *QXcbIntegration::createPixmapData(QPixmapData::PixelType type) const
{
    return new QRasterPixmapData(type);
}

QPlatformWindow *QXcbIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
    return new QXcbWindow(widget);
}

QWindowSurface *QXcbIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
    return new QXcbWindowSurface(widget);
}

QList<QPlatformScreen *> QXcbIntegration::screens() const
{
    return m_screens;
}

void QXcbIntegration::moveToScreen(QWidget *window, int screen)
{
    Q_UNUSED(window);
    Q_UNUSED(screen);
}

bool QXcbIntegration::isVirtualDesktop()
{
    return false;
}

QPlatformFontDatabase *QXcbIntegration::fontDatabase() const
{
    return m_fontDatabase;
}

QPixmap QXcbIntegration::grabWindow(WId window, int x, int y, int width, int height) const
{
    Q_UNUSED(window);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);
    return QPixmap();
}


bool QXcbIntegration::hasOpenGL() const
{
#if defined(XCB_USE_GLX)
    return true;
#elif defined(XCB_USE_EGL)
    return m_connection->hasEgl();
#elif defined(XCB_USE_DRI2)
    if (m_connection->hasSupportForDri2()) {
        return true;
    }
#endif
    return false;
}

QPlatformNativeInterface * QXcbIntegration::nativeInterface() const
{
    return m_nativeInterface;
}
