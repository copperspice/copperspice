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

#ifndef QCOCOAACCESIBILITYELEMENT_H
#define QCOCOAACCESIBILITYELEMENT_H

#include <qglobal.h>
#include <qt_mac_p.h>

#ifndef QT_NO_ACCESSIBILITY

#import <Cocoa/Cocoa.h>
#import <AppKit/NSAccessibility.h>

#import <qaccessible.h>

@class QMacAccessibilityElement;

@interface QMacAccessibilityElement : NSObject
{
   NSString *role;
   QAccessible::Id axid;
}

- (id)initWithId: (QAccessible::Id)anId;
+ (QMacAccessibilityElement *)elementWithId: (QAccessible::Id)anId;

@end

#endif // QT_NO_ACCESSIBILITY

#endif // QCOCOAACCESIBILITYELEMENT_H
