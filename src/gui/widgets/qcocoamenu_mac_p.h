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

#ifndef QCocoaMenu_MAC_P_H
#define QCocoaMenu_MAC_P_H

#include <qmacdefines_mac.h>

#import <Cocoa/Cocoa.h>

QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QAction)

@interface QT_MANGLE_NAMESPACE(QCocoaMenu) : NSMenu <NSMenuDelegate>
{
   QMenu *qmenu;
   QAction *previousAction;
}
- (id)initWithQMenu: (QMenu *)menu;
- (BOOL)menuHasKeyEquivalent: (NSMenu *)menu forEvent: (NSEvent *)event target: (id *)target action: (SEL *)action;
- (NSInteger)indexOfItemWithTarget: (id)anObject andAction: (SEL)actionSelector;
@end

#endif