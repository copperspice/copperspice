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

#include <qmacdefines_mac.h>

#import <qcocoawindow_mac_p.h>
#import <qcocoawindowdelegate_mac_p.h>
#import <qcocoaview_mac_p.h>
#import <qt_cocoa_helpers_mac_p.h>
#import <qcocoawindowcustomthemeframe_mac_p.h>
#import <qcocoaapplication_mac_p.h>
#import <qdnd_p.h>
#import <qmultitouch_mac_p.h>

#include <QtGui/QWidget>

QT_FORWARD_DECLARE_CLASS(QWidget);
QT_USE_NAMESPACE

@implementation NSWindow (QT_MANGLE_NAMESPACE(QWidgetIntegration))

- (id)QT_MANGLE_NAMESPACE(qt_initWithQWidget): (QWidget *)widget contentRect: (NSRect)rect styleMask: (NSUInteger)mask
{
   self = [self initWithContentRect: rect styleMask: mask backing: NSBackingStoreBuffered defer: YES];
   if (self) {
      [[QT_MANGLE_NAMESPACE(QCocoaWindowDelegate) sharedDelegate] becomeDelegateForWindow: self widget: widget];
      [self setReleasedWhenClosed: NO];
   }
   return self;
}

- (QWidget *)QT_MANGLE_NAMESPACE(qt_qwidget)
{
   QWidget *widget = 0;
   if ([self delegate] == [QT_MANGLE_NAMESPACE(QCocoaWindowDelegate) sharedDelegate]) {
      widget = [[QT_MANGLE_NAMESPACE(QCocoaWindowDelegate) sharedDelegate] qt_qwidgetForWindow: self];
   }
   return widget;
}

@end

@implementation QT_MANGLE_NAMESPACE(QCocoaWindow)

/***********************************************************************
  Copy and Paste between QCocoaWindow and QCocoaPanel
  This is a bit unfortunate, but thanks to the dynamic dispatch we
  have to duplicate this code or resort to really silly forwarding methods
**************************************************************************/

#include <qcocoasharedwindowmethods_mac_p.h>

@end
