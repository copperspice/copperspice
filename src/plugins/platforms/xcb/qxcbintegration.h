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

#ifndef QXCBINTEGRATION_H
#define QXCBINTEGRATION_H

#include <QtGui/QPlatformIntegration>
#include <QtGui/QPlatformScreen>

QT_BEGIN_NAMESPACE

class QXcbConnection;

class QXcbIntegration : public QPlatformIntegration
{
public:
    QXcbIntegration();
    ~QXcbIntegration();

    bool hasCapability(Capability cap) const;
    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QList<QPlatformScreen *> screens() const;
    void moveToScreen(QWidget *window, int screen);
    bool isVirtualDesktop();
    QPixmap grabWindow(WId window, int x, int y, int width, int height) const;

    QPlatformFontDatabase *fontDatabase() const;

    QPlatformNativeInterface *nativeInterface()const;

private:
    bool hasOpenGL() const;
    QList<QPlatformScreen *> m_screens;
    QXcbConnection *m_connection;

    QPlatformFontDatabase *m_fontDatabase;
    QPlatformNativeInterface *m_nativeInterface;
};

QT_END_NAMESPACE

#endif
