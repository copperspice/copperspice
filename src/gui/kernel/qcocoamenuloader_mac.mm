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

#include <qaction.h>
#include <qcoreapplication.h>
#include <qcocoamenuloader_mac_p.h>
#include <qapplication_p.h>
#include <qt_mac_p.h>
#include <qmenubar_p.h>
#include <qmenubar.h>
#include <qt_cocoa_helpers_mac_p.h>

QT_FORWARD_DECLARE_CLASS(QCFString)
QT_FORWARD_DECLARE_CLASS(QString)

#ifndef QT_NO_TRANSLATION
QT_BEGIN_NAMESPACE
extern QString qt_mac_applicationmenu_string(int type);
QT_END_NAMESPACE
#endif

QT_USE_NAMESPACE

@implementation QT_MANGLE_NAMESPACE(QCocoaMenuLoader)

- (void)awakeFromNib
{
   servicesItem = [[appMenu itemWithTitle: @"Services"] retain];
   hideAllOthersItem = [[appMenu itemWithTitle: @"Hide Others"] retain];
   showAllItem = [[appMenu itemWithTitle: @"Show All"] retain];

   // Get the names in the nib to match the app name set by Qt.
   const NSString *appName = reinterpret_cast<const NSString *>(QCFString::toCFStringRef(qAppName()));
   [quitItem setTitle: [[quitItem title] stringByReplacingOccurrencesOfString: @"NewApplication"
                        withString: const_cast<NSString *>(appName)]];
   [hideItem setTitle: [[hideItem title] stringByReplacingOccurrencesOfString: @"NewApplication"
                        withString: const_cast<NSString *>(appName)]];
   [aboutItem setTitle: [[aboutItem title] stringByReplacingOccurrencesOfString: @"NewApplication"
                         withString: const_cast<NSString *>(appName)]];
   [appName release];
   // Disable the items that don't do anything. If someone associates a QAction with them
   // They should get synced back in.
   [preferencesItem setEnabled: NO];
   [preferencesItem setHidden: YES];
   [aboutItem setEnabled: NO];
   [aboutItem setHidden: YES];
}

- (void)ensureAppMenuInMenu: (NSMenu *)menu
{
   // The application menu is the menu in the menu bar that contains the
   // 'Quit' item. When changing menu bar (e.g when switching between
   // windows with different menu bars), we never recreate this menu, but
   // instead pull it out the current menu bar and place into the new one:
   NSMenu *mainMenu = [[NSApplication sharedApplication] mainMenu];
   if (mainMenu == menu) {
      return;   // nothing to do (menu is the current menu bar)!
   }

   if (!mainMenu) {
      return;
   }

   // Grab the app menu out of the current menu.
   int numItems = [mainMenu numberOfItems];
   NSMenuItem *oldAppMenuItem = 0;
   for (int i = 0; i < numItems; ++i) {
      NSMenuItem *item = [mainMenu itemAtIndex: i];
      if ([item submenu] == appMenu) {
         oldAppMenuItem = item;
         [oldAppMenuItem retain];
         [mainMenu removeItemAtIndex: i];
         break;
      }
   }

   if (oldAppMenuItem) {
      [oldAppMenuItem setSubmenu: nil];
      [oldAppMenuItem release];
      NSMenuItem *appMenuItem = [[NSMenuItem alloc] initWithTitle: @"Apple"
                                 action: nil keyEquivalent: @""];
      [appMenuItem setSubmenu: appMenu];
      [menu insertItem: appMenuItem atIndex: 0];
   }
}

- (void)removeActionsFromAppMenu
{
   for (NSMenuItem * item in [appMenu itemArray]) {
      [item setTag: 0];
   }
}

- (void)dealloc
{
   [servicesItem release];
   [hideAllOthersItem release];
   [showAllItem release];

   [lastAppSpecificItem release];
   [theMenu release];
   [appMenu release];
   [super dealloc];
}

- (NSMenu *)menu
{
   return [[theMenu retain] autorelease];
}

- (NSMenu *)applicationMenu
{
   return [[appMenu retain] autorelease];
}

- (NSMenuItem *)quitMenuItem
{
   return [[quitItem retain] autorelease];
}

- (NSMenuItem *)preferencesMenuItem
{
   return [[preferencesItem retain] autorelease];
}

- (NSMenuItem *)aboutMenuItem
{
   return [[aboutItem retain] autorelease];
}

- (NSMenuItem *)aboutQtMenuItem
{
   return [[aboutQtItem retain] autorelease];
}

- (NSMenuItem *)hideMenuItem
{
   return [[hideItem retain] autorelease];
}

- (NSMenuItem *)appSpecificMenuItem: (NSInteger)tag
{
   NSMenuItem *item = [appMenu itemWithTag: tag];

   // No reason to create the item if it already exists. See QTBUG-27202.
   if (item) {
      return [[item retain] autorelease];
   }

   // Create an App-Specific menu item, insert it into the menu and return
   // it as an autorelease item.
   item = [[NSMenuItem alloc] init];

   NSInteger location;
   if (lastAppSpecificItem == nil) {
      location = [appMenu indexOfItem: aboutQtItem];
   } else {
      location = [appMenu indexOfItem: lastAppSpecificItem];
      [lastAppSpecificItem release];
   }
   lastAppSpecificItem = item;  // Keep track of this for later (i.e., don't release it)
   [appMenu insertItem: item atIndex: location + 1];

   return [[item retain] autorelease];
}

- (BOOL) acceptsFirstResponder
{
   return YES;
}

- (void)terminate: (id)sender
{
   [[NSApplication sharedApplication] terminate: sender];
}

- (void)orderFrontStandardAboutPanel: (id)sender
{
   [[NSApplication sharedApplication] orderFrontStandardAboutPanel: sender];
}

- (void)hideOtherApplications: (id)sender
{
   [[NSApplication sharedApplication] hideOtherApplications: sender];
}

- (void)unhideAllApplications: (id)sender
{
   [[NSApplication sharedApplication] unhideAllApplications: sender];
}

- (void)hide: (id)sender
{
   [[NSApplication sharedApplication] hide: sender];
}

- (void)qtUpdateMenubar
{
   QMenuBarPrivate::macUpdateMenuBarImmediatly();
}

- (void)qtTranslateApplicationMenu
{
#ifndef QT_NO_TRANSLATION
   [servicesItem setTitle: qt_mac_QStringToNSString(qt_mac_applicationmenu_string(0))];
   [hideItem setTitle: qt_mac_QStringToNSString(qt_mac_applicationmenu_string(1).arg(qAppName()))];
   [hideAllOthersItem setTitle: qt_mac_QStringToNSString(qt_mac_applicationmenu_string(2))];
   [showAllItem setTitle: qt_mac_QStringToNSString(qt_mac_applicationmenu_string(3))];
   [preferencesItem setTitle: qt_mac_QStringToNSString(qt_mac_applicationmenu_string(4))];
   [quitItem setTitle: qt_mac_QStringToNSString(qt_mac_applicationmenu_string(5).arg(qAppName()))];
   [aboutItem setTitle: qt_mac_QStringToNSString(qt_mac_applicationmenu_string(6).arg(qAppName()))];
#endif
}

- (IBAction)qtDispatcherToQAction: (id)sender
{
   // CopperSpice
   QThreadData *threadData = internal_get_ThreadData(QApplication::instance());
   QScopedLoopLevelCounter loopLevelCounter(threadData);

   NSMenuItem *item = static_cast<NSMenuItem *>(sender);

   if (QAction *action = reinterpret_cast<QAction *>([item tag])) {
      action->trigger();

   } else if (item == quitItem) {
      // We got here because someone was once the quitItem, but it has been
      // abandoned (e.g., the menubar was deleted). In the meantime, just do
      // normal QApplication::quit().
      qApp->quit();
   }
}

- (void)orderFrontCharacterPalette: (id)sender
{
   [[NSApplication sharedApplication] orderFrontCharacterPalette: sender];
}

- (BOOL)validateMenuItem: (NSMenuItem *)menuItem
{
   if ([menuItem action] == @selector(hide:)
         || [menuItem action] == @selector(hideOtherApplications:)
         || [menuItem action] == @selector(unhideAllApplications:)) {
      return [[NSApplication sharedApplication] validateMenuItem: menuItem];
   } else {
      return [menuItem isEnabled];
   }
}

@end

