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

#ifndef QTESTLITESCREEN_H
#define QTESTLITESCREEN_H

#include <QtGui/QPlatformScreen>
#include "qxlibintegration.h"

QT_BEGIN_NAMESPACE

class QXlibCursor;
class QXlibKeyboard;
class QXlibDisplay;

class QXlibScreen : public QPlatformScreen
{
    Q_OBJECT
public:
    QXlibScreen();

    ~QXlibScreen();

    QRect geometry() const { return mGeometry; }
    int depth() const { return mDepth; }
    QImage::Format format() const { return mFormat; }
    QSize physicalSize() const { return mPhysicalSize; }

    Window rootWindow();
    unsigned long blackPixel();
    unsigned long whitePixel();

    bool handleEvent(XEvent *xe);
    bool waitForClipboardEvent(Window win, int type, XEvent *event, int timeout);

    QImage grabWindow(Window window, int x, int y, int w, int h);

    static QXlibScreen *testLiteScreenForWidget(QWidget *widget);

    QXlibDisplay *display() const;
    int xScreenNumber() const;

    Visual *defaultVisual() const;

    QXlibKeyboard *keyboard() const;

#if !defined(QT_NO_OPENGL) && defined(QT_OPENGL_ES_2)
    void *eglDisplay() const { return mEGLDisplay; }
    void setEglDisplay(void *display) { mEGLDisplay = display; }
#endif

public slots:
    void eventDispatcher();

private:

    void handleSelectionRequest(XEvent *event);
    QRect mGeometry;
    QSize mPhysicalSize;
    int mDepth;
    QImage::Format mFormat;
    QXlibCursor *mCursor;
    QXlibKeyboard *mKeyboard;

    QXlibDisplay * mDisplay;
#if !defined(QT_NO_OPENGL) && defined(QT_OPENGL_ES_2)
    void *mEGLDisplay;
#endif
    int mScreen;
};

QT_END_NAMESPACE

#endif // QTESTLITESCREEN_H
