/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include "qplatformintegration_qpa.h"

#include <QtGui/QPlatformFontDatabase>
#include <QtGui/QPlatformClipboard>

QT_BEGIN_NAMESPACE

QPixmap QPlatformIntegration::grabWindow(WId window, int x, int y, int width, int height) const
{
   Q_UNUSED(window);
   Q_UNUSED(x);
   Q_UNUSED(y);
   Q_UNUSED(width);
   Q_UNUSED(height);
   return QPixmap();
}

/*!
    Factory function for the eventloop integration interface.

    Default implementation returns 0, which causes the eventloop to run in a single thread mode.

    \sa QPlatformEventLoopIntegration
*/
QPlatformEventLoopIntegration *QPlatformIntegration::createEventLoopIntegration() const
{
   return 0;
}

/*!
    Accessor for the platform integrations fontdatabase.

    Default implementation returns a default QPlatformFontDatabase.

    \sa QPlatformFontDatabase
*/
QPlatformFontDatabase *QPlatformIntegration::fontDatabase() const
{
   static QPlatformFontDatabase *db = 0;
   if (!db) {
      db = new QPlatformFontDatabase;
   }
   return db;
}

/*!
    Accessor for the platform integrations clipboard.

    Default implementation returns a default QPlatformClipboard.

    \sa QPlatformClipboard

*/

#ifndef QT_NO_CLIPBOARD

QPlatformClipboard *QPlatformIntegration::clipboard() const
{
   static QPlatformClipboard *clipboard = 0;
   if (!clipboard) {
      clipboard = new QPlatformClipboard;
   }
   return clipboard;
}

#endif

QPlatformNativeInterface *QPlatformIntegration::nativeInterface() const
{
   return 0;
}

/*!
    \class QPlatformIntegration
    \since 4.8
    \internal
    \preliminary
    \ingroup qpa
    \brief The QPlatformIntegration class is the entry for WindowSystem specific functionality.

    QPlatformIntegration is the single entry point for windowsystem specific functionality when
    using the QPA platform. It has factory functions for creating platform specific pixmaps and
    windows. The class also controls the font subsystem.

    QPlatformIntegration is a singelton class which gets instansiated in the QApplication
    constructor. The QPlatformIntegration instance do not have ownership of objects it creates in
    functions where the name starts with create. However, functions which don't have a name
    starting with create acts as assessors to member variables.

    It is not trivial to create or build a platform plugin outside of the Qt source tree. Therefor
    the recommended approach for making new platform plugin is to copy an existing plugin inside
    the QTSRCTREE/src/plugins/platform and develop the plugin inside the source tree.

    The minimal platform integration is the smallest platform integration it is possible to make,
    which makes it an ideal starting point for new plugins. For a slightly more advanced plugin,
    consider reviewing the directfb plugin, or the testlite plugin.
*/

/*!
    \fn QPixmapData *QPlatformIntegration::createPixmapData(QPixmapData::PixelType type) const

    Factory function for QPixmapData. PixelType can be either PixmapType or BitmapType.
    \sa QPixmapData
*/

/*!
    \fn QPlatformWindow *QPlatformIntegration::createPlatformWindow(QWidget *widget, WId winId = 0) const

    Factory function for QPlatformWindow. The widget parameter is a pointer to the top level
    widget(tlw) which the QPlatformWindow is suppose to be created for. The WId handle is actually
    never used, but there for future reference. Its purpose is if it is going to be possible to
    create QPlatformWindows on existing WId.

    All tlw has to have a QPlatformWindow, and it will be created when the QPlatformWindow is set
    to be visible for the first time. If the tlw's window flags are changed, or if the tlw's
    QPlatformWindowFormat is changed, then the tlw's QPlatformWindow is deleted and a new one is
    created.

    \sa QPlatformWindow, QPlatformWindowFormat
    \sa createWindowSurface(QWidget *widget, WId winId) const
*/

/*!
    \fn QWindowSurface *QPlatformIntegration::createWindowSurface(QWidget *widget, WId winId) const

    Factory function for QWindowSurface. The QWidget parameter is a pointer to the
    top level widget(tlw) the window surface is created for. A QPlatformWindow is always created
    before the QWindowSurface for tlw where the widget also requires a WindowSurface. It is
    possible to create top level QWidgets without a QWindowSurface by specifying
    QPlatformWindowFormat::setWindowSurface(false) for the tlw QPlatformWindowFormat.

    \sa QWindowSurface
    \sa createPlatformWindow(QWidget *widget, WId winId = 0) const
*/

/*!
    \fn void QPlatformIntegration::moveToScreen(QWidget *window, int screen)

    This function is called when a QWidget is displayed on screen, or the QWidget is to be
    displayed on a new screen. The QWidget parameter is a pointer to the top level widget and
    the int parameter is the index to the screen in QList<QPlatformScreen *> screens() const.

    Default implementation does nothing.

    \sa screens() const
*/

/*!
    \fn QList<QPlatformScreen *> QPlatformIntegration::screens() const

    Accessor function to a list of all the screens on the current system. The screen with the
    index == 0 is the default/main screen.
*/

/*!
    \fn bool QPlatformIntegration::isVirtualDesktop()

    Returns if the current windowing system configuration defines all the screens to be one
    desktop(virtual desktop), or if each screen is a desktop of its own.

    Default implementation returns false.
*/

/*!
    \fn QPixmap QPlatformIntegration::grabWindow(WId window, int x, int y, int width, int height) const

    This function is called when Qt needs to be able to grab the content of a window.

    Returnes the content of the window specified with the WId handle within the boundaries of
    QRect(x,y,width,height).
*/


bool QPlatformIntegration::hasCapability(Capability cap) const
{
   Q_UNUSED(cap);
   return false;
}





QT_END_NAMESPACE
