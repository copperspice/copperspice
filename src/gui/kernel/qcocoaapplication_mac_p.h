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

#ifndef QCocoaApplication_MAC_P_H
#define QCocoaApplication_MAC_P_H

#include <qmacdefines_mac.h>

#import <AppKit/AppKit.h>

QT_FORWARD_DECLARE_CLASS(QApplicationPrivate)
@class QT_MANGLE_NAMESPACE(QCocoaMenuLoader);

@interface NSApplication (QT_MANGLE_NAMESPACE(QApplicationIntegration))
- (void)QT_MANGLE_NAMESPACE(qt_setDockMenu): (NSMenu *)newMenu;
- (QApplicationPrivate *)QT_MANGLE_NAMESPACE(qt_qappPrivate);
- (QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *)QT_MANGLE_NAMESPACE(qt_qcocoamenuLoader);
- (int)QT_MANGLE_NAMESPACE(qt_validModesForFontPanel): (NSFontPanel *)fontPanel;

- (void)QT_MANGLE_NAMESPACE(qt_sendPostedMessage): (NSEvent *)event;
- (BOOL)QT_MANGLE_NAMESPACE(qt_filterEvent): (NSEvent *)event;
@end

@interface QT_MANGLE_NAMESPACE(QNSApplication) : NSApplication
{
}
@end

QT_BEGIN_NAMESPACE

void qt_redirectNSApplicationSendEvent();
void qt_resetNSApplicationSendEvent();

QT_END_NAMESPACE

#endif
