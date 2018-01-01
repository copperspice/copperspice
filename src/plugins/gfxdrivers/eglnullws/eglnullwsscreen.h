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

#ifndef EGLNULLWSSCREEN
#define EGLNULLWSSCREEN

#include <QGLScreen>

class EGLNullWSScreen : public QGLScreen
{
public:
    EGLNullWSScreen(int displayId);
    ~EGLNullWSScreen();

    bool initDevice();
    bool connect(const QString &displaySpec);
    void disconnect();
    void shutdownDevice();

    void setMode(int width, int height, int depth);
    void blank(bool on);

    void exposeRegion(QRegion r, int changing);

    QWSWindowSurface* createSurface(QWidget *widget) const;
    QWSWindowSurface* createSurface(const QString &key) const;

    bool hasOpenGL() { return true; }
};

#endif // EGLNULLWSSCREEN
