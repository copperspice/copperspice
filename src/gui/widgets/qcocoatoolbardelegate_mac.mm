/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#import <qcocoatoolbardelegate_mac_p.h>

#include <qmainwindowlayout_p.h>
#include <qt_mac_p.h>
#include <qt_cocoa_helpers_mac_p.h>
#include <qcocoaview_mac_p.h>
#include <qwidget_p.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE
extern QWidgetPrivate *qt_widget_private(QWidget *widget);
QT_END_NAMESPACE

QT_FORWARD_DECLARE_CLASS(QMainWindowLayout);
QT_FORWARD_DECLARE_CLASS(QToolBar);
QT_FORWARD_DECLARE_CLASS(QCFString);

@implementation QT_MANGLE_NAMESPACE(QCocoaToolBarDelegate)

- (id)initWithMainWindowLayout: (QMainWindowLayout *)layout
{
   self = [super init];
   if (self) {
      mainWindowLayout = layout;
   }
   return self;
}

- (NSArray *)toolbarAllowedItemIdentifiers: (NSToolbar *)toolbar
{
   Q_UNUSED(toolbar);
   return [NSArray arrayWithObject: @"com.copperspice.nstoolbar-qtoolbar"];
}

- (NSArray *)toolbarDefaultItemIdentifiers: (NSToolbar *)toolbar
{
   return [self toolbarAllowedItemIdentifiers: toolbar];
}

- (void)toolbarDidRemoveItem: (NSNotification *)notification
{
   NSToolbarItem *item = [[notification userInfo] valueForKey: @"item"];
   mainWindowLayout->unifiedToolbarHash.remove(item);
   for (int i = 0; i < mainWindowLayout->toolbarItemsCopy.size(); ++i) {
      if (mainWindowLayout->toolbarItemsCopy.at(i) == item) {
         // I know about it, so release it.
         mainWindowLayout->toolbarItemsCopy.removeAt(i);
         mainWindowLayout->qtoolbarsInUnifiedToolbarList.removeAt(i);
         [item release];
         break;
      }
   }
}

- (NSToolbarItem *)toolbar: (NSToolbar *)nstoolbar itemForItemIdentifier: (NSString *)itemIdentifier
 willBeInsertedIntoToolbar: (BOOL)flag
{
   Q_UNUSED(flag);
   Q_UNUSED(nstoolbar);
   QToolBar *tb = mainWindowLayout->cocoaItemIDToToolbarHash.value(
                     QT_PREPEND_NAMESPACE(qt_mac_NSStringToQString)(itemIdentifier));
   NSToolbarItem *item = nil;
   if (tb) {
      item = [[NSToolbarItem alloc] initWithItemIdentifier: itemIdentifier];
      mainWindowLayout->unifiedToolbarHash.insert(item, tb);
   }
   return item;
}

- (void)toolbarWillAddItem: (NSNotification *)notification
{
   NSToolbarItem *item = [[notification userInfo] valueForKey: @"item"];
   QToolBar *tb = mainWindowLayout->cocoaItemIDToToolbarHash.value(
                     QT_PREPEND_NAMESPACE(qt_mac_NSStringToQString)([item itemIdentifier]));
   if (!tb) {
      return;   // I can't really do anything about this.
   }
   [item retain];
   [item setView: QT_PREPEND_NAMESPACE(qt_mac_nativeview_for)(tb)];

   NSArray *items = [[qt_mac_window_for(mainWindowLayout->layoutState.mainWindow->window()) toolbar] items];
   int someIndex = 0;
   for (NSToolbarItem * i in items) {
      if (i == item) {
         break;
      }
      ++someIndex;
   }
   mainWindowLayout->toolbarItemsCopy.insert(someIndex, item);

   // This is synchronization code that was needed in Carbon, but may not be needed anymore here.
   QToolBar *toolbar = mainWindowLayout->unifiedToolbarHash.value(item);
   if (toolbar) {
      int toolbarIndex = mainWindowLayout->qtoolbarsInUnifiedToolbarList.indexOf(toolbar);
      if (someIndex != toolbarIndex) {
         // Dang, we must be out of sync, rebuild it from the "toolbarItemsCopy"
         mainWindowLayout->qtoolbarsInUnifiedToolbarList.clear();
         for (int i = 0; i < mainWindowLayout->toolbarItemsCopy.size(); ++i) {
            // This will either append the correct toolbar or an
            // null toolbar. This is fine because this list
            // is really only kept to make sure that things are but in the right order.
            mainWindowLayout->qtoolbarsInUnifiedToolbarList.append(
               mainWindowLayout->unifiedToolbarHash.value(mainWindowLayout->
                     toolbarItemsCopy.at(i)));
         }
      }
      toolbar->update();
   }
}

@end
