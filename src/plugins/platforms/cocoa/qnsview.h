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

#ifndef QNSVIEW_H
#define QNSVIEW_H

#include <Cocoa/Cocoa.h>

#include <QPointer>
#include <QImage>
#include <QAccessible>

#include <qcore_mac_p.h>

class QCocoaWindow;
class QCocoaBackingStore;
class QCocoaGLContext;

#ifdef __OBJC__
@class QNSViewMouseMoveHelper;
#else
using QNSViewMouseMoveHelper = struct objc_object;
#endif

@interface QNSView : NSView <NSTextInputClient>
{
   QCocoaBackingStore *m_backingStore;
   QPoint m_backingStoreOffset;
   CGImageRef m_maskImage;
   uchar *m_maskData;
   bool m_shouldInvalidateWindowShadow;
   QPointer<QWindow> m_window;
   QCocoaWindow *m_platformWindow;
   NSTrackingArea *m_trackingArea;
   Qt::MouseButtons m_buttons;
   Qt::MouseButtons m_frameStrutButtons;
   QString m_composingText;
   bool m_sendKeyEvent;
   QStringList *currentCustomDragTypes;
   bool m_sendUpAsRightButton;
   Qt::KeyboardModifiers currentWheelModifiers;
   bool m_subscribesForGlobalFrameNotifications;

#ifndef QT_NO_OPENGL
   QCocoaGLContext *m_glContext;
   bool m_shouldSetGLContextinDrawRect;
#endif

   NSString *m_inputSource;
   QNSViewMouseMoveHelper *m_mouseMoveHelper;
   bool m_resendKeyEvent;
   bool m_scrolling;
   bool m_updatingDrag;
   bool m_exposedOnMoveToWindow;
   NSEvent *m_currentlyInterpretedKeyEvent;
   bool m_isMenuView;
}

- (id)init;
- (id)initWithQWindow: (QWindow *)window platformWindow: (QCocoaWindow *) platformWindow;
- (void) clearQWindowPointers;

#ifndef QT_NO_OPENGL
- (void)setQCocoaGLContext: (QCocoaGLContext *)context;
#endif

- (void)flushBackingStore: (QCocoaBackingStore *)backingStore region: (const QRegion &)region offset: (QPoint)offset;
- (void)clearBackingStore: (QCocoaBackingStore *)backingStore;
- (void)setMaskRegion: (const QRegion *)region;
- (void)invalidateWindowShadowIfNeeded;
- (void)drawRect: (NSRect)dirtyRect;
- (void)updateGeometry;
- (void)notifyWindowStateChanged: (Qt::WindowState)newState;
- (void)windowNotification: (NSNotification *) windowNotification;
- (void)notifyWindowWillZoom: (BOOL)willZoom;
- (void)textInputContextKeyboardSelectionDidChangeNotification: (NSNotification *)
   textInputContextKeyboardSelectionDidChangeNotification;
- (void)viewDidHide;
- (void)viewDidUnhide;
- (void)removeFromSuperview;

- (BOOL)isFlipped;
- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)hasMask;
- (BOOL)isOpaque;

- (void)convertFromScreen: (NSPoint)mouseLocation toWindowPoint: (QPointF *)qtWindowPoint andScreenPoint: (QPointF *)qtScreenPoint;

- (void)resetMouseButtons;

- (void)handleMouseEvent: (NSEvent *)theEvent;
- (void)mouseDown: (NSEvent *)theEvent;
- (void)mouseDragged: (NSEvent *)theEvent;
- (void)mouseUp: (NSEvent *)theEvent;
- (void)mouseMovedImpl: (NSEvent *)theEvent;
- (void)mouseEnteredImpl: (NSEvent *)theEvent;
- (void)mouseExitedImpl: (NSEvent *)theEvent;
- (void)rightMouseDown: (NSEvent *)theEvent;
- (void)rightMouseDragged: (NSEvent *)theEvent;
- (void)rightMouseUp: (NSEvent *)theEvent;
- (void)otherMouseDown: (NSEvent *)theEvent;
- (void)otherMouseDragged: (NSEvent *)theEvent;
- (void)otherMouseUp: (NSEvent *)theEvent;
- (void)handleFrameStrutMouseEvent: (NSEvent *)theEvent;

- (bool)handleTabletEvent: (NSEvent *)theEvent;
- (void)tabletPoint: (NSEvent *)theEvent;
- (void)tabletProximity: (NSEvent *)theEvent;

- (int) convertKeyCode: (QChar)keyCode;
+ (Qt::KeyboardModifiers) convertKeyModifiers: (ulong)modifierFlags;
- (void)handleKeyEvent: (NSEvent *)theEvent eventType: (int)eventType;
- (void)keyDown: (NSEvent *)theEvent;
- (void)keyUp: (NSEvent *)theEvent;

- (void)registerDragTypes;
- (NSDragOperation)handleDrag: (id <NSDraggingInfo>)sender;

@end

#endif
