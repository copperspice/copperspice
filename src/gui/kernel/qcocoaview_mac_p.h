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

#ifndef QCocoaView_MAC_P_H
#define QCocoaView_MAC_P_H

#include <qevent.h>
#import  <Cocoa/Cocoa.h>

@class QT_MANGLE_NAMESPACE(QCocoaView);
QT_FORWARD_DECLARE_CLASS(QWidgetPrivate);
QT_FORWARD_DECLARE_CLASS(QWidget);
QT_FORWARD_DECLARE_CLASS(QEvent);
QT_FORWARD_DECLARE_CLASS(QString);
QT_FORWARD_DECLARE_CLASS(QStringList);

Q_GUI_EXPORT
@interface QT_MANGLE_NAMESPACE(QCocoaView) : NSControl <NSTextInput>
{
   QWidget *qwidget;
   QWidgetPrivate *qwidgetprivate;
   NSDragOperation supportedActions;
   bool composing;
   int composingLength;
   bool sendKeyEvents;
   bool fromKeyDownEvent;
   QString *composingText;
 @public int alienTouchCount;
}
- (id)initWithQWidget: (QWidget *)widget widgetPrivate: (QWidgetPrivate *)widgetprivate;
- (void) finishInitWithQWidget: (QWidget *)widget widgetPrivate: (QWidgetPrivate *)widgetprivate;
- (void)frameDidChange: (NSNotification *)note;
- (void)setSupportedActions: (NSDragOperation)actions;
- (NSDragOperation)draggingSourceOperationMaskForLocal: (BOOL)isLocal;
- (void)draggedImage: (NSImage *)anImage endedAt: (NSPoint)aPoint operation: (NSDragOperation)operation;
- (BOOL)isComposing;
- (QWidget *)qt_qwidget;
- (void) qt_clearQWidget;

@end
#endif