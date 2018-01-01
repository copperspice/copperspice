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

#include "qcocoawindow.h"
#include "qnswindowdelegate.h"
#include "qcocoaautoreleasepool.h"

#include <QWidget>

#include <QtGui/QApplication>

#include <QWindowSystemInterface>

#include <QDebug>

QCocoaWindow::QCocoaWindow(QWidget *tlw)
    : QPlatformWindow(tlw)
{
    QCocoaAutoReleasePool pool;
    const QRect geo = tlw->geometry();
    NSRect frame = NSMakeRect(geo.x(), geo.y(), geo.width(), geo.height());

    m_nsWindow  = [[NSWindow alloc] initWithContentRect:frame
                                            styleMask:NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask
                                            backing:NSBackingStoreBuffered
                                            defer:YES];

    QNSWindowDelegate *delegate = [[QNSWindowDelegate alloc] initWithQCocoaWindow:this];
    [m_nsWindow setDelegate:delegate];

    [m_nsWindow makeKeyAndOrderFront:nil];
    [m_nsWindow setAcceptsMouseMovedEvents:YES];

    if ([m_nsWindow respondsToSelector:@selector(setRestorable:)])
        [m_nsWindow setRestorable: NO];

}

QCocoaWindow::~QCocoaWindow()
{
}

void QCocoaWindow::setGeometry(const QRect &rect)
{
    QPlatformWindow::setGeometry(rect);

    NSRect bounds = NSMakeRect(rect.x(), rect.y(), rect.width(), rect.height());
    [[m_nsWindow contentView]setFrameSize:bounds.size];
}

void QCocoaWindow::setVisible(bool visible)
{
    Q_UNUSED(visible);
}

WId QCocoaWindow::winId() const
{
    return WId([m_nsWindow windowNumber]);
}

NSView *QCocoaWindow::contentView() const
{
    return [m_nsWindow contentView];
}

void QCocoaWindow::setContentView(NSView *contentView)
{
    [m_nsWindow setContentView:contentView];
}

void QCocoaWindow::windowDidResize()
{
    //jlind: XXX This isn't ideal. Eventdispatcher does not run when resizing...
    NSRect rect = [[m_nsWindow contentView]frame];
    QRect geo(rect.origin.x,rect.origin.y,rect.size.width,rect.size.height);
    QWindowSystemInterface::handleGeometryChange(widget(),geo);
}
