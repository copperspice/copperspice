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

#include "qcocoaintegration.h"
#include "qcocoawindow.h"
#include "qcocoawindowsurface.h"
#include "qcocoaeventloopintegration.h"
#include "qcoretextfontdatabase.h"
#include <QtGui/QApplication>
#include <qpixmap_raster_p.h>

QT_BEGIN_NAMESPACE

QCocoaScreen::QCocoaScreen(int screenIndex)
    :QPlatformScreen()
{
    m_screen = [[NSScreen screens] objectAtIndex:screenIndex];
    NSRect rect = [m_screen frame];
    m_geometry = QRect(rect.origin.x,rect.origin.y,rect.size.width,rect.size.height);

    m_format = QImage::Format_ARGB32;

    m_depth = NSBitsPerPixelFromDepth([m_screen depth]);

    const int dpi = 72;
    const qreal inch = 25.4;
    m_physicalSize = QSize(qRound(m_geometry.width() * inch / dpi), qRound(m_geometry.height() *inch / dpi));
}

QCocoaScreen::~QCocoaScreen()
{
}

QCocoaIntegration::QCocoaIntegration()
    : mFontDb(new QCoreTextFontDatabase())
{
    mPool = new QCocoaAutoReleasePool;

    //Make sure we have a nsapplication :)
    [NSApplication sharedApplication];
//    [[OurApplication alloc] init];

    NSArray *screens = [NSScreen screens];
    for (uint i = 0; i < [screens count]; i++) {
        QCocoaScreen *screen = new QCocoaScreen(i);
        mScreens.append(screen);
    }
}

QCocoaIntegration::~QCocoaIntegration()
{
    delete mPool;
}

bool QCocoaIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}



QPixmapData *QCocoaIntegration::createPixmapData(QPixmapData::PixelType type) const
{
    return new QRasterPixmapData(type);
}

QPlatformWindow *QCocoaIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
    return new QCocoaWindow(widget);
}

QWindowSurface *QCocoaIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
    return new QCocoaWindowSurface(widget,winId);
}

QPlatformFontDatabase *QCocoaIntegration::fontDatabase() const
{
    return mFontDb;
}

QPlatformEventLoopIntegration *QCocoaIntegration::createEventLoopIntegration() const
{
    return new QCocoaEventLoopIntegration();
}
QT_END_NAMESPACE
