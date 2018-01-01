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

#ifndef QGRAPHICSSYSTEM_LINUXFB_H
#define QGRAPHICSSYSTEM_LINUXFB_H

#include <QPlatformIntegration>
#include "../fb_base/fb_base.h"

QT_BEGIN_NAMESPACE

class QLinuxFbScreen : public QFbScreen
{
    Q_OBJECT
public:
    QLinuxFbScreen(uchar * d, int w, int h, int lstep, QImage::Format screenFormat);
    void setGeometry(QRect rect);
    void setFormat(QImage::Format format);

public slots:
    QRegion doRedraw();

private:
    QImage * mFbScreenImage;
    uchar * data;
    int bytesPerLine;

    QPainter *compositePainter;
};

class QLinuxFbIntegrationPrivate;
struct fb_cmap;
struct fb_var_screeninfo;
struct fb_fix_screeninfo;

class QLinuxFbIntegration : public QPlatformIntegration
{
public:
    QLinuxFbIntegration();
    ~QLinuxFbIntegration();

    bool hasCapability(QPlatformIntegration::Capability cap) const;

    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId WinId) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId WinId) const;

    QList<QPlatformScreen *> screens() const { return mScreens; }

    QPlatformFontDatabase *fontDatabase() const;

private:
    QLinuxFbScreen *mPrimaryScreen;
    QList<QPlatformScreen *> mScreens;
    QLinuxFbIntegrationPrivate *d_ptr;

    enum PixelType { NormalPixel, BGRPixel };

    QRgb screenclut[256];
    int screencols;

    uchar * data;

    QImage::Format screenFormat;
    int w;
    int lstep;
    int h;
    int d;
    PixelType pixeltype;
    bool grayscale;

    int dw;
    int dh;

    int size;               // Screen size
    int mapsize;       // Total mapped memory

    int displayId;

    int physWidth;
    int physHeight;

    bool canaccel;
    int dataoffset;
    int cacheStart;

    bool connect(const QString &displaySpec);
    bool initDevice();
    void setPixelFormat(struct fb_var_screeninfo);
    void createPalette(fb_cmap &cmap, fb_var_screeninfo &vinfo, fb_fix_screeninfo &finfo);
    void blank(bool on);
    QPlatformFontDatabase *fontDb;
};

QT_END_NAMESPACE

#endif
