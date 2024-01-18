/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#ifndef QNSWINDOWDELEGATE_H
#define QNSWINDOWDELEGATE_H

#include <Cocoa/Cocoa.h>

#include <qcocoawindow.h>

@interface QNSWindowDelegate : NSObject <NSWindowDelegate>
{
   QCocoaWindow *m_cocoaWindow;
}

- (id)initWithQCocoaWindow: (QCocoaWindow *) cocoaWindow;

- (void)windowDidBecomeKey: (NSNotification *)notification;
- (void)windowDidResize: (NSNotification *)notification;
- (void)windowDidMove: (NSNotification *)notification;
- (void)windowWillMove: (NSNotification *)notification;
- (BOOL)windowShouldClose: (NSNotification *)notification;
- (BOOL)windowShouldZoom: (NSWindow *)window toFrame: (NSRect)newFrame;
- (BOOL)window: (NSWindow *)window shouldPopUpDocumentPathMenu: (NSMenu *)menu;
- (BOOL)window: (NSWindow *)window shouldDragDocumentWithEvent: (NSEvent *)event from: (NSPoint)dragImageLocation withPasteboard:
   (NSPasteboard *)pasteboard;
@end

#endif
