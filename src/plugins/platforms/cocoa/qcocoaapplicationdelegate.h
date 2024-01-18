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

/***********************************************************************
* Copyright (c) 2007-2008, Apple, Inc.
* All rights reserved.
*
* Refer to APPLE_LICENSE.TXT (in this directory) for license terms
***********************************************************************/

#import <Cocoa/Cocoa.h>

#include <qglobal.h>
#include <qcore_mac_p.h>

@class QCocoaMenuLoader;

@interface QCocoaApplicationDelegate : NSObject <NSApplicationDelegate>
{
   bool startedQuit;
   NSMenu *dockMenu;
   QCocoaMenuLoader *qtMenuLoader;
   NSObject <NSApplicationDelegate> *reflectionDelegate;
   bool inLaunch;
}

+ (QCocoaApplicationDelegate *)sharedDelegate;
- (void)setDockMenu: (NSMenu *)newMenu;
- (void)setMenuLoader: (QCocoaMenuLoader *)menuLoader;
- (QCocoaMenuLoader *)menuLoader;
- (void)setReflectionDelegate: (NSObject <NSApplicationDelegate> *)oldDelegate;
- (void)getUrl: (NSAppleEventDescriptor *)event withReplyEvent: (NSAppleEventDescriptor *)replyEvent;
- (void) removeAppleEventHandlers;
- (bool) inLaunch;
@end

