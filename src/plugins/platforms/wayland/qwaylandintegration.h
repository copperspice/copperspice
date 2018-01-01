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

#ifndef QPLATFORMINTEGRATION_WAYLAND_H
#define QPLATFORMINTEGRATION_WAYLAND_H

#include <QtGui/QPlatformIntegration>

QT_BEGIN_NAMESPACE

class QWaylandBuffer;
class QWaylandDisplay;

class QWaylandIntegration : public QPlatformIntegration
{
public:
    QWaylandIntegration(bool useOpenGL = false);

    bool hasCapability(QPlatformIntegration::Capability cap) const;
    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QList<QPlatformScreen *> screens() const;

    QPlatformFontDatabase *fontDatabase() const;

    QPlatformNativeInterface *nativeInterface() const;

    QPlatformClipboard *clipboard() const;

private:
    bool hasOpenGL() const;

    QPlatformFontDatabase *mFontDb;
    QWaylandDisplay *mDisplay;
    bool mUseOpenGL;
    QPlatformNativeInterface *mNativeInterface;
    mutable QPlatformClipboard *mClipboard;
};

QT_END_NAMESPACE

#endif
