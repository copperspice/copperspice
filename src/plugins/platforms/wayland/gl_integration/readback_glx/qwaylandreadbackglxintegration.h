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

#ifndef QWAYLANDREADBACKGLXINTEGRATION_H
#define QWAYLANDREADBACKGLXINTEGRATION_H

#include "gl_integration/qwaylandglintegration.h"

#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QtGui/QWidget>

#include <X11/Xlib.h>

class QWaylandReadbackGlxIntegration : public QWaylandGLIntegration
{
public:
    QWaylandReadbackGlxIntegration(QWaylandDisplay * waylandDispaly);
    ~QWaylandReadbackGlxIntegration();

    void initialize();

    QWaylandWindow *createEglWindow(QWidget *widget);

    QWaylandDisplay *waylandDisplay() const;

    Display *xDisplay() const;
    int screen() const;
    Window rootWindow() const;

private:
    QWaylandDisplay *mWaylandDisplay;

    Display *mDisplay;
    int mScreen;
    Window mRootWindow;

};

#endif // QWAYLANDREADBACKGLXINTEGRATION_H
