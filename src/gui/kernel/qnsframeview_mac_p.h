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

// Private AppKit class (dumped from classdump).

#ifndef QNSFrameview_MAC_P_H
#define QNSFrameview_MAC_P_H

#import <Cocoa/Cocoa.h>

@interface NSFrameView : NSView
{
   unsigned int styleMask;
   NSString *_title;
   NSCell *titleCell;
   NSButton *closeButton;
   NSButton *zoomButton;
   NSButton *minimizeButton;
   char resizeByIncrement;
   char frameNeedsDisplay;
   unsigned char tabViewCount;
   NSSize resizeParameter;
   int shadowState;
}

+ (void)initialize;
+ (void)initTitleCell: fp8 styleMask: (unsigned int)fp12;
+ (struct _NSRect)frameRectForContentRect: (struct _NSRect)fp8 styleMask: (unsigned int)fp24;
+ (struct _NSRect)contentRectForFrameRect: (struct _NSRect)fp8 styleMask: (unsigned int)fp24;
+ (struct _NSSize)minFrameSizeForMinContentSize: (struct _NSSize)fp8 styleMask: (unsigned int)fp16;
+ (struct _NSSize)minContentSizeForMinFrameSize: (struct _NSSize)fp8 styleMask: (unsigned int)fp16;
+ (float)minFrameWidthWithTitle: fp8 styleMask: (unsigned int)fp12;
+ (unsigned int)_validateStyleMask: (unsigned int)fp8;
- initWithFrame:
(struct _NSRect)fp8 styleMask:
(unsigned int)fp24 owner:
fp28;
- initWithFrame:
(struct _NSRect)fp8;
- (void)dealloc;
- (void)shapeWindow;
- (void)tileAndSetWindowShape: (char)fp8;
- (void)tile;
- (void)drawRect: (struct _NSRect)fp8;
- (void)_drawFrameRects: (struct _NSRect)fp8;
- (void)drawFrame: (struct _NSRect)fp8;
- (void)drawThemeContentFill: (struct _NSRect)fp8 inView: fp24;
- (void)drawWindowBackgroundRect: (struct _NSRect)fp8;
- (void)drawWindowBackgroundRegion: (void *)fp8;
- (float)contentAlpha;
- (void)_windowChangedKeyState;
- (void)_updateButtonState;
- (char)_isSheet;
- (char)_isUtility;
- (void)setShadowState: (int)fp8;
- (int)shadowState;
- (char)_canHaveToolbar;
- (char)_toolbarIsInTransition;
- (char)_toolbarIsShown;
- (char)_toolbarIsHidden;
- (void)_showToolbarWithAnimation: (char)fp8;
- (void)_hideToolbarWithAnimation: (char)fp8;
- (float)_distanceFromToolbarBaseToTitlebar;
- (int)_shadowType;
- (unsigned int)_shadowFlags;
- (void)_setShadowParameters;
- (void)_drawFrameShadowAndFlushContext: fp8;
- (void)setUpGState;
- (void)adjustHalftonePhase;
- (void)systemColorsDidChange: fp8;
- frameColor;
- contentFill;
- (void)tabViewAdded;
- (void)tabViewRemoved;
- title;
- (void)setTitle: fp8;
- titleCell;
- (void)initTitleCell: fp8;
- (void)setResizeIncrements: (struct _NSSize)fp8;
- (struct _NSSize)resizeIncrements;
- (void)setAspectRatio: (struct _NSSize)fp8;
- (struct _NSSize)aspectRatio;
- (unsigned int)styleMask;
- representedFilename;
- (void)setRepresentedFilename: fp8;
- (void)setDocumentEdited: (char)fp8;
- (void)_setFrameNeedsDisplay: (char)fp8;
- (char)frameNeedsDisplay;
- titleFont;
- (struct _NSRect)_maxTitlebarTitleRect;
- (struct _NSRect)titlebarRect;
- (void)_setUtilityWindow: (char)fp8;
- (void)_setNonactivatingPanel: (char)fp8;
- (void)setIsClosable: (char)fp8;
- (void)setIsResizable: (char)fp8;
- closeButton;
- minimizeButton;
- zoomButton;
- (struct _NSSize)miniaturizedSize;
- (void)_clearDragMargins;
- (void)_resetDragMargins;
- (void)setTitle: fp8 andDefeatWrap: (char)fp12;
- (struct _NSRect)frameRectForContentRect: (struct _NSRect)fp8 styleMask: (unsigned int)fp24;
- (struct _NSRect)contentRectForFrameRect: (struct _NSRect)fp8 styleMask: (unsigned int)fp24;
- (struct _NSSize)minFrameSizeForMinContentSize: (struct _NSSize)fp8 styleMask: (unsigned int)fp16;
- (struct _NSRect)dragRectForFrameRect: (struct _NSRect)fp8;
- (struct _NSRect)contentRect;
- (struct _NSSize)minFrameSize;
- (void)_recursiveDisplayRectIfNeededIgnoringOpacity: (struct _NSRect)fp8 isVisibleRect:
                 (char)fp24 rectIsVisibleRectForView: fp28 topView: (char)fp32;

@end

#endif