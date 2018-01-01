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

#ifndef QCOCOAPANEL_MAC_P_H
#define QCOCOAPANEL_MAC_P_H

#include <qmacdefines_mac.h>

#import <Cocoa/Cocoa.h>

QT_FORWARD_DECLARE_CLASS(QStringList);
QT_FORWARD_DECLARE_CLASS(QCocoaDropData);

@interface NSPanel (QtIntegration)
- (NSDragOperation)draggingEntered: (id <NSDraggingInfo>)sender;
- (NSDragOperation)draggingUpdated: (id <NSDraggingInfo>)sender;
- (void)draggingExited: (id <NSDraggingInfo>)sender;
- (BOOL)performDragOperation: (id <NSDraggingInfo>)sender;
@end

@interface QT_MANGLE_NAMESPACE(QCocoaPanel) : NSPanel
{
   QStringList *currentCustomDragTypes;
   QCocoaDropData *dropData;
   NSInteger dragEnterSequence;
}

+ (Class)frameViewClassForStyleMask: (NSUInteger)styleMask;
- (void)registerDragTypes;
- (void)drawRectOriginal: (NSRect)rect;

@end
#endif

