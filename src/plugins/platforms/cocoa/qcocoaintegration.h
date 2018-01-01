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

#ifndef QPLATFORMINTEGRATION_COCOA_H
#define QPLATFORMINTEGRATION_COCOA_H

#include <Cocoa/Cocoa.h>

#include "qcocoaautoreleasepool.h"

#include <QtGui/QPlatformIntegration>

QT_BEGIN_NAMESPACE

class QCocoaScreen : public QPlatformScreen
{
public:
    QCocoaScreen(int screenIndex);
    ~QCocoaScreen();

    QRect geometry() const { return m_geometry; }
    int depth() const { return m_depth; }
    QImage::Format format() const { return m_format; }
    QSize physicalSize() const { return m_physicalSize; }

public:
    NSScreen *m_screen;
    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    QSize m_physicalSize;
};

class QCocoaIntegration : public QPlatformIntegration
{
public:
    QCocoaIntegration();
    ~QCocoaIntegration();

    bool hasCapability(QPlatformIntegration::Capability cap) const;
    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId = 0) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QList<QPlatformScreen *> screens() const { return mScreens; }

    QPlatformFontDatabase *fontDatabase() const;

    QPlatformEventLoopIntegration *createEventLoopIntegration() const;

private:
    QList<QPlatformScreen *> mScreens;
    QPlatformFontDatabase *mFontDb;

    QCocoaAutoReleasePool *mPool;
};

QT_END_NAMESPACE

#endif

