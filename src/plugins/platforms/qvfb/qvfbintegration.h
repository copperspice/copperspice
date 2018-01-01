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

#ifndef QGRAPHICSSYSTEM_QVFB_H
#define QGRAPHICSSYSTEM_QVFB_H

#include <QPlatformScreen>
#include <QPlatformIntegration>

QT_BEGIN_NAMESPACE


class QVFbScreenPrivate;

class QVFbScreen : public QPlatformScreen
{
public:
    QVFbScreen(int id);
    ~QVFbScreen();

    QRect geometry() const;
     int depth() const;
     QImage::Format format() const;
     QSize physicalSize() const;

    QImage *screenImage();

    void setDirty(const QRect &rect);

public:

    QVFbScreenPrivate *d_ptr;
};

class QVFbIntegrationPrivate;


class QVFbIntegration : public QPlatformIntegration
{
public:
    QVFbIntegration(const QStringList &paramList);

    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QList<QPlatformScreen *> screens() const { return mScreens; }

    QPlatformFontDatabase *fontDatabase() const;

private:
    QVFbScreen *mPrimaryScreen;
    QList<QPlatformScreen *> mScreens;
    QPlatformFontDatabase *mFontDb;
};



QT_END_NAMESPACE


#endif
