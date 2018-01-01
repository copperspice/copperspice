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

#ifndef QWAYLANDREADBACKEGLGLCONTEXT_H
#define QWAYLANDREADBACKEGLGLCONTEXT_H

#include <QPlatformGLContext>
#include <QtGui/QWidget>

#include "qwaylandreadbackeglintegration.h"
#include "qwaylandreadbackeglwindow.h"

class QWaylandShmBuffer;

class QWaylandReadbackEglContext : public QPlatformGLContext
{
public:
    QWaylandReadbackEglContext(QWaylandReadbackEglIntegration *eglIntegration, QWaylandReadbackEglWindow *window);
    ~QWaylandReadbackEglContext();

    void makeCurrent();
    void doneCurrent();
    void swapBuffers();
    void* getProcAddress(const QString& procName);

    virtual QPlatformWindowFormat platformWindowFormat() const;

    void geometryChanged();

private:
    QWaylandReadbackEglIntegration *mEglIntegration;
    QWaylandReadbackEglWindow *mWindow;
    QWaylandShmBuffer *mBuffer;

    Pixmap mPixmap;

    EGLConfig mConfig;
    EGLContext mContext;
    EGLSurface mPixmapSurface;
};

#endif // QWAYLANDREADBACKEGLGLCONTEXT_H
