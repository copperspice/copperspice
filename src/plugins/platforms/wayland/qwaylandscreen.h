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

#ifndef QWAYLANDSCREEN_H
#define QWAYLANDSCREEN_H

#include <QtGui/QPlatformScreen>

class QWaylandDisplay;
class QWaylandCursor;
struct wl_visual;

class QWaylandScreen : public QPlatformScreen
{
public:
    QWaylandScreen(QWaylandDisplay *waylandDisplay, struct wl_output *output, QRect geometry);
    ~QWaylandScreen();

    QWaylandDisplay *display() const;

    QRect geometry() const;
    int depth() const;
    QImage::Format format() const;

    wl_visual *visual() const;

    static QWaylandScreen *waylandScreenFromWidget(QWidget *widget);

private:
    QWaylandDisplay *mWaylandDisplay;
    struct wl_output *mOutput;
    QRect mGeometry;
    int mDepth;
    QImage::Format mFormat;
    QSize mPhysicalSize;
    QWaylandCursor *mWaylandCursor;
};

#endif // QWAYLANDSCREEN_H
