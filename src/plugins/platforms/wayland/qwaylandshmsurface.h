/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QWINDOWSURFACE_WAYLAND_H
#define QWINDOWSURFACE_WAYLAND_H

#include <qwaylandbuffer.h>
#include <qwindowsurface_p.h>
#include <QtGui/QPlatformWindow>

QT_BEGIN_NAMESPACE

class QWaylandDisplay;

class QWaylandShmBuffer : public QWaylandBuffer {
public:
    QWaylandShmBuffer(QWaylandDisplay *display,
		   const QSize &size, QImage::Format format);
    ~QWaylandShmBuffer();
    QSize size() const { return mImage.size(); }
    QImage *image() { return &mImage; }
private:
    QImage mImage;
};

class QWaylandShmWindowSurface : public QWindowSurface
{
public:
    QWaylandShmWindowSurface(QWidget *window);
    ~QWaylandShmWindowSurface();

    QPaintDevice *paintDevice();
    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
    void resize(const QSize &size);
    void beginPaint(const QRegion &);

private:
    QWaylandShmBuffer *mBuffer;
    QWaylandDisplay *mDisplay;
};

QT_END_NAMESPACE

#endif
