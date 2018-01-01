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

#ifndef QGRAPHICSSYSTEM_TESTLITE_H
#define QGRAPHICSSYSTEM_TESTLITE_H

//make sure textstream is included before any X11 headers
#include <QtCore/QTextStream>

#include <QtGui/QPlatformIntegration>
#include <QtGui/QPlatformScreen>
#include <QtGui/QPlatformNativeInterface>

#include "qxlibstatic.h"

QT_BEGIN_NAMESPACE

class QXlibScreen;

class QXlibIntegration : public QPlatformIntegration
{
public:
    QXlibIntegration(bool useOpenGL = false);

    bool hasCapability(Capability cap) const;
    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QPixmap grabWindow(WId window, int x, int y, int width, int height) const;

    QList<QPlatformScreen *> screens() const { return mScreens; }

    QPlatformFontDatabase *fontDatabase() const;
    QPlatformClipboard *clipboard() const;

    QPlatformNativeInterface *nativeInterface() const;

private:
    bool hasOpenGL() const;

    bool mUseOpenGL;
    QXlibScreen *mPrimaryScreen;
    QList<QPlatformScreen *> mScreens;
    QPlatformFontDatabase *mFontDb;
    QPlatformClipboard *mClipboard;
    QPlatformNativeInterface *mNativeInterface;
};

QT_END_NAMESPACE

#endif
