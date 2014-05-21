/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPLATFORMINTEGRATION_QPA_H
#define QPLATFORMINTEGRATION_QPA_H

#include <QtGui/qwindowdefs.h>
#include <qwindowsurface_p.h>
#include <qpixmapdata_p.h>
#include <QtGui/qplatformscreen_qpa.h>

QT_BEGIN_NAMESPACE

class QPlatformWindow;
class QWindowSurface;
class QBlittable;
class QWidget;
class QPlatformEventLoopIntegration;
class QPlatformFontDatabase;
class QPlatformClipboard;
class QPlatformNativeInterface;

class Q_GUI_EXPORT QPlatformIntegration
{

public:
    enum Capability {
        ThreadedPixmaps = 1,
        OpenGL = 2
    };

    virtual ~QPlatformIntegration() { }

    virtual bool hasCapability(Capability cap) const;

// GraphicsSystem functions
    virtual QPixmapData *createPixmapData(QPixmapData::PixelType type) const = 0;
    virtual QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId = 0) const = 0;
    virtual QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const = 0;

// Window System functions
    virtual QList<QPlatformScreen *> screens() const = 0;
    virtual void moveToScreen(QWidget *window, int screen) {Q_UNUSED(window); Q_UNUSED(screen);}
    virtual bool isVirtualDesktop() { return false; }
    virtual QPixmap grabWindow(WId window, int x, int y, int width, int height) const;

// Deeper window system integrations
    virtual QPlatformFontDatabase *fontDatabase() const;

#ifndef QT_NO_CLIPBOARD
    virtual QPlatformClipboard *clipboard() const;
#endif

// Experimental in mainthread eventloop integration
// This should only be used if it is only possible to do window system event processing in
// the gui thread. All of the functions in QWindowSystemInterface are thread safe.
    virtual QPlatformEventLoopIntegration *createEventLoopIntegration() const;

// Access native handles. The window handle is already available from Wid;
    virtual QPlatformNativeInterface *nativeInterface() const;
};

QT_END_NAMESPACE

#endif // QPLATFORMINTEGRATION_H
