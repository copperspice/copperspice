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

/***********************************************************************
** Copyright (c) 2007-2008, Apple, Inc.
***********************************************************************/

#ifndef QCocoaApplicationDelegate_MAC_P_H
#define QCocoaApplicationDelegate_MAC_P_H

#include <qmacdefines_mac.h>

#import <Cocoa/Cocoa.h>

QT_FORWARD_DECLARE_CLASS(QApplicationPrivate);

@class QT_MANGLE_NAMESPACE(QCocoaMenuLoader);

@interface QT_MANGLE_NAMESPACE(QCocoaApplicationDelegate) : NSObject <NSApplicationDelegate>
{
   bool startedQuit;
   QApplicationPrivate *qtPrivate;
   NSMenu *dockMenu;
   QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *qtMenuLoader;
   NSObject <NSApplicationDelegate> *reflectionDelegate;
   bool inLaunch;
}
+ (QT_MANGLE_NAMESPACE(QCocoaApplicationDelegate) *)sharedDelegate;
- (void)setDockMenu: (NSMenu *)newMenu;
- (void)setQtPrivate: (QApplicationPrivate *)value;
- (QApplicationPrivate *)qAppPrivate;
- (void)setMenuLoader: (QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *)menuLoader;
- (QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *)menuLoader;
- (void)setReflectionDelegate: (NSObject <NSApplicationDelegate> *)oldDelegate;
- (void)getUrl: (NSAppleEventDescriptor *)event withReplyEvent: (NSAppleEventDescriptor *)replyEvent;
@end

#endif