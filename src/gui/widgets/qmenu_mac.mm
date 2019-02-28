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

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "qmenu.h"
#include "qmenubar.h"
#include "qmenubar_p.h"
#include "qmacnativewidget_mac.h"

#include <QtCore/QDebug>
#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <qplatform_nativeinterface.h>


#ifndef QT_NO_MENU

namespace {
// TODO use QtMacExtras copy of this function when available.
inline QPlatformNativeInterface::FP_Integration resolvePlatformFunction(const QByteArray &functionName)
{
    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    QPlatformNativeInterface::FP_Integration function =
        nativeInterface->nativeResourceFunctionForIntegration(functionName);

    if (! function)
         qWarning() << "Unable to resolve function" << functionName
                    << "from QGuiApplication::platformNativeInterface()->nativeResourceFunctionForIntegration()";
    return function;
}
} //namespsace


/*!
    \since 5.2

    Returns the native NSMenu for this menu. Available on \macos only.

    \note Qt sets the delegate on the native menu. If you need to set your own
    delegate, make sure you save the original one and forward any calls to it.
*/
NSMenu *QMenu::toNSMenu()
{
    // Call into the cocoa platform plugin: qMenuToNSMenu(platformMenu())
    QPlatformNativeInterface::FP_Integration function = resolvePlatformFunction("qmenutonsmenu");

    if (function) {
        typedef void* (*QMenuToNSMenuFunction)(QPlatformMenu *platformMenu);
        return reinterpret_cast<NSMenu *>(reinterpret_cast<QMenuToNSMenuFunction>(function)(platformMenu()));
    }
    return nil;
}


/*!
    \since 5.2

    Set this menu to be the dock menu available by option-clicking
    on the application dock icon. Available on \macos only.
*/
void QMenu::setAsDockMenu()
{
    // Call into the cocoa platform plugin: setDockMenu(platformMenu())
    QPlatformNativeInterface::FP_Integration function = resolvePlatformFunction("setdockmenu");
    if (function) {
        typedef void (*SetDockMenuFunction)(QPlatformMenu *platformMenu);
        reinterpret_cast<SetDockMenuFunction>(function)(platformMenu());
    }
}


/*! \fn void qt_mac_set_dock_menu(QMenu *menu)
    \relates QMenu
    \deprecated

    Sets this \a menu to be the dock menu available by option-clicking
    on the application dock icon. Available on \macos only.

    Deprecated; use \l QMenu::setAsDockMenu() instead.
*/

void QMenuPrivate::moveWidgetToPlatformItem(QWidget *widget, QPlatformMenuItem* item)
{
    QMacNativeWidget *container = new QMacNativeWidget;
    QObject::connect(platformMenu, SIGNAL(destroyed()), container, SLOT(deleteLater()));
    container->resize(widget->sizeHint());
    widget->setParent(container);
    widget->setVisible(true);

    NSView *containerView = container->nativeView();
    QWindow *containerWindow = container->windowHandle();
    Qt::WindowFlags wf = containerWindow->flags();
    containerWindow->setFlags(wf | Qt::SubWindow);
    [(NSView *)widget->winId() setAutoresizingMask:NSViewWidthSizable];

    item->setNativeContents((WId)containerView);
    container->show();
}

#endif //QT_NO_MENU

#ifndef QT_NO_MENUBAR

/*!
    \since 5.2

    Returns the native NSMenu for this menu bar. Available on \macos only.

    \note Qt may set the delegate on the native menu bar. If you need to set your
    own delegate, make sure you save the original one and forward any calls to it.
*/
NSMenu *QMenuBar::toNSMenu()
{
    // Call into the cocoa platform plugin: qMenuBarToNSMenu(platformMenuBar())
    QPlatformNativeInterface::FP_Integration function = resolvePlatformFunction("qmenubartonsmenu");
    if (function) {
        typedef void* (*QMenuBarToNSMenuFunction)(QPlatformMenuBar *platformMenuBar);
        return reinterpret_cast<NSMenu *>(reinterpret_cast<QMenuBarToNSMenuFunction>(function)(platformMenuBar()));
    }
    return nil;
}
#endif //QT_NO_MENUBAR



