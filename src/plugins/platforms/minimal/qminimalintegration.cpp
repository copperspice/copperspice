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

#include "qminimalintegration.h"
#include "qminimalwindowsurface.h"

#include <QtGui/private/qpixmap_raster_p.h>
#include <QtGui/QPlatformWindow>

QMinimalIntegration::QMinimalIntegration()
{
    QMinimalScreen *mPrimaryScreen = new QMinimalScreen();

    mPrimaryScreen->mGeometry = QRect(0, 0, 240, 320);
    mPrimaryScreen->mDepth = 32;
    mPrimaryScreen->mFormat = QImage::Format_ARGB32_Premultiplied;

    mScreens.append(mPrimaryScreen);
}

bool QMinimalIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPixmapData *QMinimalIntegration::createPixmapData(QPixmapData::PixelType type) const
{
    return new QRasterPixmapData(type);
}

QPlatformWindow *QMinimalIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
    return new QPlatformWindow(widget);
}

QWindowSurface *QMinimalIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
    return new QMinimalWindowSurface(widget);
}
