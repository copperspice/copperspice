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

#ifndef QCocoaWindowDelegate_MAC_P_H
#define QCocoaWindowDelegate_MAC_P_H

#include "qmacdefines_mac.h"

#ifdef QT_MAC_USE_COCOA
#import <Cocoa/Cocoa.h>

QT_BEGIN_NAMESPACE
template <class Key, class T> class QHash;
QT_END_NAMESPACE
using QT_PREPEND_NAMESPACE(QHash);
QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QSize)
QT_FORWARD_DECLARE_CLASS(QWidgetData)

#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_5
@protocol NSWindowDelegate <NSObject>
- (NSSize)windowWillResize:(NSWindow *)window toSize:(NSSize)proposedFrameSize;
- (void)windowDidMiniaturize:(NSNotification*)notification;
- (void)windowDidResize:(NSNotification *)notification;
- (NSRect)windowWillUseStandardFrame:(NSWindow *)window defaultFrame:(NSRect)defaultFrame;
- (void)windowDidMove:(NSNotification *)notification;
- (BOOL)windowShouldClose:(id)window;
- (void)windowDidDeminiaturize:(NSNotification *)notification;
- (void)windowDidBecomeMain:(NSNotification*)notification;
- (void)windowDidResignMain:(NSNotification*)notification;
- (void)windowDidBecomeKey:(NSNotification*)notification;
- (void)windowDidResignKey:(NSNotification*)notification;
- (void)windowDidChangeScreen:(NSNotification*)notification;
- (BOOL)window:(NSWindow *)window shouldPopUpDocumentPathMenu:(NSMenu *)menu;
- (BOOL)window:(NSWindow *)window shouldDragDocumentWithEvent:(NSEvent *)event from:(NSPoint)dragImageLocation withPasteboard:(NSPasteboard *)pasteboard;
- (BOOL)windowShouldZoom:(NSWindow *)window toFrame:(NSRect)newFrame;
@end

@protocol NSDrawerDelegate <NSObject>
- (NSSize)drawerWillResizeContents:(NSDrawer *)sender toSize:(NSSize)contentSize;
@end

#endif



@interface QT_MANGLE_NAMESPACE(QCocoaWindowDelegate) : NSObject<NSWindowDelegate, NSDrawerDelegate> {
    QHash<NSWindow *, QWidget *> *m_windowHash;
    QHash<NSDrawer *, QWidget *> *m_drawerHash;
}
+ (QT_MANGLE_NAMESPACE(QCocoaWindowDelegate)*)sharedDelegate;
- (void)becomeDelegateForWindow:(NSWindow *)window  widget:(QWidget *)widget;
- (void)resignDelegateForWindow:(NSWindow *)window;
- (void)becomeDelegateForDrawer:(NSDrawer *)drawer widget:(QWidget *)widget;
- (void)resignDelegateForDrawer:(NSDrawer *)drawer;
- (void)dumpMaximizedStateforWidget:(QWidget*)qwidget window:(NSWindow *)window;
- (void)syncSizeForWidget:(QWidget *)qwidget
        toSize:(const QSize &)newSize
        fromSize:(const QSize &)oldSize;
- (NSSize)closestAcceptableSizeForWidget:(QWidget *)qwidget
        window:(NSWindow *)window withNewSize:(NSSize)proposedSize;
- (QWidget *)qt_qwidgetForWindow:(NSWindow *)window;
- (void)syncContentViewFrame: (NSNotification *)notification;
@end
#endif

#endif