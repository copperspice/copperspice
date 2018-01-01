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

#ifndef PVREGLSCREEN_H
#define PVREGLSCREEN_H

#include <QScreen>
#include <QGLScreen>
#include "pvrqwsdrawable.h"

class PvrEglScreen;

class PvrEglScreenSurfaceFunctions : public QGLScreenSurfaceFunctions
{
public:
    PvrEglScreenSurfaceFunctions(PvrEglScreen *s, int screenNum)
        : screen(s), displayId(screenNum) {}

    bool createNativeWindow(QWidget *widget, EGLNativeWindowType *native);

private:
    PvrEglScreen *screen;
    int displayId;
};

class PvrEglScreen : public QGLScreen
{
public:
    PvrEglScreen(int displayId);
    ~PvrEglScreen();

    bool initDevice();
    bool connect(const QString &displaySpec);
    void disconnect();
    void shutdownDevice();
    void setMode(int, int, int) {}

    void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
    void solidFill(const QColor &color, const QRegion &region);

    bool chooseContext(QGLContext *context, const QGLContext *shareContext);
    bool hasOpenGL();

    QWSWindowSurface* createSurface(QWidget *widget) const;
    QWSWindowSurface* createSurface(const QString &key) const;

    int transformation() const;

private:
    void sync();
    void openTty();
    void closeTty();

    int fd;
    int ttyfd, oldKdMode;
    QString ttyDevice;
    bool doGraphicsMode;
    mutable const QScreen *parent;
};

#endif
