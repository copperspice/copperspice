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

#ifndef QCocoaWindowDelegate_MAC_P_H
#define QCocoaWindowDelegate_MAC_P_H

#include <qmacdefines_mac.h>
#import  <Cocoa/Cocoa.h>

#include <qcontainerfwd.h>

class QWidget;
class QSize;
class QWidgetData;

@interface QT_MANGLE_NAMESPACE(QCocoaWindowDelegate) : NSObject<NSWindowDelegate, NSDrawerDelegate>
{
   QHash<NSWindow *, QWidget *> *m_windowHash;
   QHash<NSDrawer *, QWidget *> *m_drawerHash;
}
+ (QT_MANGLE_NAMESPACE(QCocoaWindowDelegate) *)sharedDelegate;
- (void)becomeDelegateForWindow: (NSWindow *)window  widget: (QWidget *)widget;
- (void)resignDelegateForWindow: (NSWindow *)window;
- (void)becomeDelegateForDrawer: (NSDrawer *)drawer widget: (QWidget *)widget;
- (void)resignDelegateForDrawer: (NSDrawer *)drawer;
- (void)dumpMaximizedStateforWidget: (QWidget *)qwidget window: (NSWindow *)window;
- (void)syncSizeForWidget: (QWidget *)qwidget
                  toSize: (const QSize &)newSize
                  fromSize: (const QSize &)oldSize;
- (NSSize)closestAcceptableSizeForWidget: (QWidget *)qwidget
                  window: (NSWindow *)window withNewSize: (NSSize)proposedSize;
- (QWidget *)qt_qwidgetForWindow: (NSWindow *)window;
- (void)syncContentViewFrame: (NSNotification *)notification;
@end

#endif