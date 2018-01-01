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

#ifndef QEGLSCREEN_H
#define QEGLSCREEN_H

#include <QPlatformScreen>

#include <QtCore/QTextStream>

#include <EGL/egl.h>

QT_BEGIN_NAMESPACE

class QPlatformGLContext;

class QEglFSScreen : public QPlatformScreen //huh: FullScreenScreen ;) just to follow namespace
{
public:
    QEglFSScreen(EGLNativeDisplayType display);
    ~QEglFSScreen() {}

    QRect geometry() const;
    int depth() const;
    QImage::Format format() const;

    QPlatformGLContext *platformContext() const;

private:
    void createAndSetPlatformContext() const;
    void createAndSetPlatformContext();

    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    QPlatformGLContext *m_platformContext;
    EGLDisplay m_dpy;
    EGLSurface m_surface;
};

QT_END_NAMESPACE
#endif // QEGLSCREEN_H
