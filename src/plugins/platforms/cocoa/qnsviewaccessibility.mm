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

#include <Carbon/Carbon.h>

#include "qnsview.h"
#include "qcocoahelpers.h"
#include "qcocoaaccessibility.h"
#include "qcocoaaccessibilityelement.h"
#include "qcocoaintegration.h"

#include <qaccessible.h>
#include <QDebug>

#import <AppKit/NSAccessibility.h>

#ifndef QT_NO_ACCESSIBILITY

@implementation QNSView (QNSViewAccessibility)

- (id)childAccessibleElement
{
   if (!m_window->accessibleRoot()) {
      return nil;
   }

   QAccessible::Id childId = QAccessible::uniqueId(m_window->accessibleRoot());
   return [QMacAccessibilityElement elementWithId: childId];
}

// The QNSView is a container that the user does not interact directly with:
// Remove it from the user-visible accessibility tree.
- (BOOL)accessibilityIsIgnored
{
   return YES;
}

- (id)accessibilityAttributeValue: (NSString *)attribute
{
   // activate accessibility updates
   QCocoaIntegration::instance()->accessibility()->setActive(true);

   if ([attribute isEqualToString: NSAccessibilityChildrenAttribute]) {
      return NSAccessibilityUnignoredChildrenForOnlyChild([self childAccessibleElement]);
   } else {
      return [super accessibilityAttributeValue: attribute];
   }
}

- (id)accessibilityHitTest: (NSPoint)point
{
   return [[self childAccessibleElement] accessibilityHitTest: point];
}

- (id)accessibilityFocusedUIElement
{
   return [[self childAccessibleElement] accessibilityFocusedUIElement];
}

@end

#endif // QT_NO_ACCESSIBILITY
