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

#include "qnswindowdelegate.h"

#include <QDebug>
#include <qwindowsysteminterface.h>

@implementation QNSWindowDelegate

- (id) initWithQCocoaWindow: (QCocoaWindow *) cocoaWindow
{
    self = [super init];

    if (self) {
        m_cocoaWindow = cocoaWindow;
    }
    return self;
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    Q_UNUSED(notification);
    if (m_cocoaWindow->m_windowUnderMouse) {
        QPointF windowPoint;
        QPointF screenPoint;
        [m_cocoaWindow->m_qtView convertFromScreen:[NSEvent mouseLocation] toWindowPoint:&windowPoint andScreenPoint:&screenPoint];
        QWindowSystemInterface::handleEnterEvent(m_cocoaWindow->m_enterLeaveTargetWindow, windowPoint, screenPoint);
    }
}

- (void)windowDidResize:(NSNotification *)notification
{
    Q_UNUSED(notification);
    if (m_cocoaWindow) {
        m_cocoaWindow->windowDidResize();
    }
}

- (void)windowDidEndLiveResize:(NSNotification *)notification
{
    Q_UNUSED(notification);
    if (m_cocoaWindow) {
        m_cocoaWindow->windowDidEndLiveResize();
    }
}

- (void)windowWillMove:(NSNotification *)notification
{
    Q_UNUSED(notification);
    if (m_cocoaWindow) {
        m_cocoaWindow->windowWillMove();
    }
}

- (void)windowDidMove:(NSNotification *)notification
{
    Q_UNUSED(notification);
    if (m_cocoaWindow) {
        m_cocoaWindow->windowDidMove();
    }
}

- (BOOL)windowShouldClose:(NSNotification *)notification
{
    Q_UNUSED(notification);
    if (m_cocoaWindow) {
        return m_cocoaWindow->windowShouldClose();
    }

    return YES;
}

- (BOOL)windowShouldZoom:(NSWindow *)window toFrame:(NSRect)newFrame
{
    Q_UNUSED(newFrame);
    if (m_cocoaWindow && m_cocoaWindow->m_qtView)
        [m_cocoaWindow->m_qtView notifyWindowWillZoom:![window isZoomed]];
    return YES;
}

- (BOOL)window:(NSWindow *)window shouldPopUpDocumentPathMenu:(NSMenu *)menu
{
    Q_UNUSED(window);
    Q_UNUSED(menu);
    return m_cocoaWindow && m_cocoaWindow->m_hasWindowFilePath;
}

- (BOOL)window:(NSWindow *)window shouldDragDocumentWithEvent:(NSEvent *)event from:(NSPoint)dragImageLocation withPasteboard:(NSPasteboard *)pasteboard
{
    Q_UNUSED(window);
    Q_UNUSED(event);
    Q_UNUSED(dragImageLocation);
    Q_UNUSED(pasteboard);
    return m_cocoaWindow && m_cocoaWindow->m_hasWindowFilePath;
}
@end
