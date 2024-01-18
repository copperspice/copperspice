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

#ifndef QCOCOAACCESIBILITY_H
#define QCOCOAACCESIBILITY_H

#include <Cocoa/Cocoa.h>

#include <qplatform_accessibility.h>

#ifndef QT_NO_ACCESSIBILITY

class QCocoaAccessibility : public QPlatformAccessibility
{
 public:
   QCocoaAccessibility();
   ~QCocoaAccessibility();
   void notifyAccessibilityUpdate(QAccessibleEvent *event) override;
   void setRootObject(QObject *o) override;
   void initialize() override;
   void cleanup() override;
};

namespace QCocoaAccessible {

/*
    Qt Cocoa Accessibility Overview

    Cocoa accessibility is implemented in the following files:

    - qcocoaaccessibility (this file) : QCocoaAccessibility "plugin", conversion and helper functions.
    - qnsviewaccessibility            : Root accessibility implementation for QNSView
    - qcocoaaccessibilityelement      : Cocoa accessibility protocol wrapper for QAccessibleInterface

    The accessibility implementation wraps QAccessibleInterfaces in QCocoaAccessibleElements, which
    implements the cocoa accessibility protocol. The root QAccessibleInterface (the one returned
    by QWindow::accessibleRoot), is anchored to the QNSView in qnsviewaccessibility.mm.

    Cocoa explores the accessibility tree by walking the tree using the parent/child
    relationships or hit testing. When this happens we create QCocoaAccessibleElements on
    demand.
*/

NSString *macRole(QAccessibleInterface *interface);
NSString *macSubrole(QAccessibleInterface *interface);
bool shouldBeIgnored(QAccessibleInterface *interface);
NSArray *unignoredChildren(QAccessibleInterface *interface);
NSString *getTranslatedAction(const QString &qtAction);
NSMutableArray *createTranslatedActionsList(const QStringList &qtActions);
QString translateAction(NSString *nsAction, QAccessibleInterface *interface);
bool hasValueAttribute(QAccessibleInterface *interface);
id getValueAttribute(QAccessibleInterface *interface);

}

#endif // QT_NO_ACCESSIBILITY

#endif // QCOCOAACCESIBILITY_H
