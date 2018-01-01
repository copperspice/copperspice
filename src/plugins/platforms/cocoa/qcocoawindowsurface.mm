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

#include "qcocoawindowsurface.h"

#include <QtCore/qdebug.h>

#include <QtGui/QPainter>

QT_BEGIN_NAMESPACE

QRect flipedRect(const QRect &sourceRect,int height)
{
    if (!sourceRect.isValid())
        return QRect();
    QRect flippedRect = sourceRect;
    flippedRect.moveTop(height - sourceRect.y());
    return flippedRect;
}

QCocoaWindowSurface::QCocoaWindowSurface(QWidget *window, WId wId)
    : QWindowSurface(window)
{
    m_cocoaWindow = static_cast<QCocoaWindow *>(window->platformWindow());

    const QRect geo = window->geometry();
    NSRect rect = NSMakeRect(geo.x(),geo.y(),geo.width(),geo.height());
    m_contentView = [[QNSView alloc] initWithWidget:window];
    m_cocoaWindow->setContentView(m_contentView);

    m_image = new QImage(window->size(),QImage::Format_ARGB32);
}

QCocoaWindowSurface::~QCocoaWindowSurface()
{
    delete m_image;
}

QPaintDevice *QCocoaWindowSurface::paintDevice()
{
    return m_image;
}

void QCocoaWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(widget);
    Q_UNUSED(offset);

    QRect geo = region.boundingRect();

    NSRect rect = NSMakeRect(geo.x(), geo.y(), geo.width(), geo.height());
    [m_contentView displayRect:rect];
}

void QCocoaWindowSurface::resize(const QSize &size)
{
    QWindowSurface::resize(size);
    delete m_image;
    m_image = new QImage(size,QImage::Format_ARGB32_Premultiplied);
    NSSize newSize = NSMakeSize(size.width(),size.height());
    [m_contentView setImage:m_image];

}

QT_END_NAMESPACE
