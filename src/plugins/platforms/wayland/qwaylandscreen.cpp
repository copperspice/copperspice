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

#include "qwaylandscreen.h"

#include "qwaylanddisplay.h"
#include "qwaylandcursor.h"

QWaylandScreen::QWaylandScreen(QWaylandDisplay *waylandDisplay, struct wl_output *output, QRect geometry)
    : QPlatformScreen()
    , mWaylandDisplay(waylandDisplay)
    , mOutput(output)
    , mGeometry(geometry)
    , mDepth(32)
    , mFormat(QImage::Format_ARGB32_Premultiplied)
    , mWaylandCursor(new QWaylandCursor(this))
{
    moveToThread(waylandDisplay->thread());
}

QWaylandScreen::~QWaylandScreen()
{
    delete mWaylandCursor;
}

QWaylandDisplay * QWaylandScreen::display() const
{
    return mWaylandDisplay;
}

QRect QWaylandScreen::geometry() const
{
    return mGeometry;
}

int QWaylandScreen::depth() const
{
    return mDepth;
}

QImage::Format QWaylandScreen::format() const
{
    return mFormat;
}

QWaylandScreen * QWaylandScreen::waylandScreenFromWidget(QWidget *widget)
{
    QPlatformScreen *platformScreen = QPlatformScreen::platformScreenForWidget(widget);
    return static_cast<QWaylandScreen *>(platformScreen);
}

wl_visual * QWaylandScreen::visual() const
{
    struct wl_visual *visual;

    switch (format()) {
    case QImage::Format_ARGB32:
        visual = mWaylandDisplay->argbVisual();
        break;
    case QImage::Format_ARGB32_Premultiplied:
        visual = mWaylandDisplay->argbPremultipliedVisual();
        break;
    default:
        qDebug("unsupported buffer format %d requested\n", format());
        visual = mWaylandDisplay->argbVisual();
        break;
    }
    return visual;
}
