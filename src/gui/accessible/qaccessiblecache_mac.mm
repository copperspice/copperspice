/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qaccessiblecache_p.h>

// qcocoaaccessibilityelement.h in Cocoa platform plugin
@interface QT_MANGLE_NAMESPACE(QMacAccessibilityElement)
- (void)invalidate;
@end

void QAccessibleCache::insertElement(QAccessible::Id axid, QT_MANGLE_NAMESPACE(QMacAccessibilityElement) *element) const
{
    cocoaElements[axid] = element;
}

void QAccessibleCache::removeCocoaElement(QAccessible::Id axid)
{
    QT_MANGLE_NAMESPACE(QMacAccessibilityElement) *element = elementForId(axid);
    [element invalidate];
    cocoaElements.remove(axid);
}

QT_MANGLE_NAMESPACE(QMacAccessibilityElement) *QAccessibleCache::elementForId(QAccessible::Id axid) const
{
    return cocoaElements.value(axid);
}

