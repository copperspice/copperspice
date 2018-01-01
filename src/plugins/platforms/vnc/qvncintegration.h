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

#ifndef QGRAPHICSSYSTEM_VNC_H
#define QGRAPHICSSYSTEM_VNC_H

#include "qvnccursor.h"
#include "../fb_base/fb_base.h"
#include <QPlatformIntegration>
#include "qgenericunixfontdatabase.h"

QT_BEGIN_NAMESPACE

class QVNCServer;
class QVNCDirtyMap;

class QVNCScreenPrivate;

class QVNCScreen : public QFbScreen
{
    Q_OBJECT
public:
    QVNCScreen(QRect screenSize, int screenId);

    int linestep() const { return image() ? image()->bytesPerLine() : 0; }
    uchar *base() const { return image() ? image()->bits() : 0; }
    QVNCDirtyMap *dirtyMap();

public:
    QVNCScreenPrivate *d_ptr;

private:
    QVNCServer *server;
    QRegion doRedraw();
    friend class QVNCIntegration;
};

class QVNCIntegrationPrivate;


class QVNCIntegration : public QPlatformIntegration
{
public:
    QVNCIntegration(const QStringList& paramList);

    bool hasCapability(QPlatformIntegration::Capability cap) const;
    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QPixmap grabWindow(WId window, int x, int y, int width, int height) const;

    QList<QPlatformScreen *> screens() const { return mScreens; }

    bool isVirtualDesktop() { return virtualDesktop; }
    void moveToScreen(QWidget *window, int screen);

    QPlatformFontDatabase *fontDatabase() const;

private:
    QVNCScreen *mPrimaryScreen;
    QList<QPlatformScreen *> mScreens;
    bool virtualDesktop;
    QPlatformFontDatabase *fontDb;
};



QT_END_NAMESPACE

#endif //QGRAPHICSSYSTEM_VNC_H

