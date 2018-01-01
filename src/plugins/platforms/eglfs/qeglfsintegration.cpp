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

#include "qeglfsintegration.h"

#include "qeglfswindow.h"
#include "qeglfswindowsurface.h"

#include "qgenericunixfontdatabase.h"

#include <QtGui/QPlatformWindow>
#include <QtGui/QPlatformWindowFormat>
#include <QtOpenGL/private/qpixmapdata_gl_p.h>

#include <EGL/egl.h>

QT_BEGIN_NAMESPACE

QEglFSIntegration::QEglFSIntegration()
    : mFontDb(new QGenericUnixFontDatabase())
{
    m_primaryScreen = new QEglFSScreen(EGL_DEFAULT_DISPLAY);

    mScreens.append(m_primaryScreen);
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglIntegration\n");
#endif
}

bool QEglFSIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPixmapData *QEglFSIntegration::createPixmapData(QPixmapData::PixelType type) const
{
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglIntegration::createPixmapData %d\n", type);
#endif
    return new QGLPixmapData(type);
}

QPlatformWindow *QEglFSIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglIntegration::createPlatformWindow %p\n",widget);
#endif
    return new QEglFSWindow(widget, m_primaryScreen);
}


QWindowSurface *QEglFSIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);

#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglIntegration::createWindowSurface %p\n",widget);
#endif
    return new QEglFSWindowSurface(m_primaryScreen,widget);
}

QPlatformFontDatabase *QEglFSIntegration::fontDatabase() const
{
    return mFontDb;
}

QT_END_NAMESPACE
