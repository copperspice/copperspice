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

#ifndef QGRAPHICSSYSTEM_OPENKODE_H
#define QGRAPHICSSYSTEM_OPENKODE_H

#include "qopenkodeeventloopintegration.h"
#include <QtCore/qsemaphore.h>
#include <QtGui/QPlatformIntegration>
#include <QtGui/QPlatformScreen>
#include <QtGui/QPlatformGLContext>
#include <QtGui/QPlatformFontDatabase>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

QT_BEGIN_NAMESPACE

struct KDDesktopNV;
struct KDDisplayNV;
class QOpenKODECursor;

class QOpenKODEScreen : public QPlatformScreen
{
    Q_OBJECT
public:
    QOpenKODEScreen(KDDisplayNV *kdDisplay,  KDDesktopNV *kdDesktop);
    ~QOpenKODEScreen() {}

    QRect geometry() const { return mGeometry; }
    int depth() const { return mDepth; }
    QImage::Format format() const { return mFormat; }

    EGLDisplay eglDisplay() { return mEglDisplay; }

    bool isFullScreen() const {return mIsFullScreen;}
    void setFullScreen(bool fullscreen) { mIsFullScreen = fullscreen; }
private:
    QRect mGeometry;
    int mDepth;
    QImage::Format mFormat;
    EGLDisplay mEglDisplay;
    bool mIsFullScreen;
};

class QOpenKODEIntegration : public QPlatformIntegration
{
public:
    QOpenKODEIntegration();
    ~QOpenKODEIntegration();

    bool hasCapability(QPlatformIntegration::Capability cap) const;

    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId = 0) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QPlatformEventLoopIntegration *createEventLoopIntegration() const;

    QPlatformFontDatabase *fontDatabase() const;

    virtual QList<QPlatformScreen *> screens() const { return mScreens; }

    static GLuint blitterProgram();

    void setMainGLContext(QEGLPlatformContext *ctx) { mMainGlContext = ctx; }
    void mainGLContext() const { return mMainGlContext; }

private:
    QList<QPlatformScreen *> mScreens;
    QOpenKODEEventLoopIntegration *mEventLoopIntegration;
    QPlatformFontDatabase *mFontDb;
    QEGLPlatformContext *mMainGlContext;
};

QT_END_NAMESPACE

#endif
