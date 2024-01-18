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

#include <qnswindowdelegate.h>

#include <qdebug.h>
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

- (void)windowDidBecomeKey: (NSNotification *)notification
{
   (void) notification;
   if (m_cocoaWindow->m_windowUnderMouse) {
      QPointF windowPoint;
      QPointF screenPoint;
      [m_cocoaWindow->m_qtView convertFromScreen: [NSEvent mouseLocation] toWindowPoint: &windowPoint andScreenPoint: &screenPoint];
      QWindowSystemInterface::handleEnterEvent(m_cocoaWindow->m_enterLeaveTargetWindow, windowPoint, screenPoint);
   }
}

- (void)windowDidResize: (NSNotification *)notification
{
   (void) notification;
   if (m_cocoaWindow) {
      m_cocoaWindow->windowDidResize();
   }
}

- (void)windowDidEndLiveResize: (NSNotification *)notification
{
   (void) notification;
   if (m_cocoaWindow) {
      m_cocoaWindow->windowDidEndLiveResize();
   }
}

- (void)windowWillMove: (NSNotification *)notification
{
   (void) notification;
   if (m_cocoaWindow) {
      m_cocoaWindow->windowWillMove();
   }
}

- (void)windowDidMove: (NSNotification *)notification
{
   (void) notification;
   if (m_cocoaWindow) {
      m_cocoaWindow->windowDidMove();
   }
}

- (BOOL)windowShouldClose: (NSNotification *)notification
{
   (void) notification;
   if (m_cocoaWindow) {
      return m_cocoaWindow->windowShouldClose();
   }

   return YES;
}

- (BOOL)windowShouldZoom: (NSWindow *)window toFrame: (NSRect)newFrame
{
   if (m_cocoaWindow && m_cocoaWindow->m_qtView) {
      [m_cocoaWindow->m_qtView notifyWindowWillZoom: ![window isZoomed]];
   }

   return YES;
}

- (BOOL)window: (NSWindow *)window shouldPopUpDocumentPathMenu: (NSMenu *)menu
{
   (void) window;
   (void) menu;
   return m_cocoaWindow && m_cocoaWindow->m_hasWindowFilePath;
}

- (BOOL)window: (NSWindow *)window shouldDragDocumentWithEvent: (NSEvent *)event from: (NSPoint)dragImageLocation withPasteboard:
   (NSPasteboard *)pasteboard
{
   (void) window;
   (void) event;
   (void) dragImageLocation;
   (void) pasteboard;
   return m_cocoaWindow && m_cocoaWindow->m_hasWindowFilePath;
}
@end
