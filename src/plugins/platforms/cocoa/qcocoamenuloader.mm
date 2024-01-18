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

#include <qcocoamenuloader.h>

#include <messages.h>
#include <qcocoahelpers.h>
#include <qcocoamenubar.h>
#include <qcocoamenuitem.h>

#include <qcoreapplication.h>
#include <qdir.h>
#include <qstring.h>
#include <qdebug.h>

#include <qapplication_p.h>
#include <qcore_mac_p.h>
#include <qthread_p.h>

class QCFString;

@implementation QCocoaMenuLoader

- (instancetype)init
{
   if ((self = [super init])) {
     NSString *appName = qt_mac_applicationName().toNSString();

     // Menubar as menu.
     theMenu = [[NSMenu alloc] initWithTitle:@"Main Menu"];

     // Application menu. first menu is always the application menu.
     NSMenuItem *appItem = [[[NSMenuItem alloc] init] autorelease];
     appItem.title   = appName;
     [theMenu addItem:appItem];
     appMenu = [[NSMenu alloc] initWithTitle:appName];
     appItem.submenu = appMenu;

     // About Application
     aboutItem = [[NSMenuItem alloc] initWithTitle:[@"About " stringByAppendingString:appName]
                                            action:@selector(orderFrontStandardAboutPanel:)
                                     keyEquivalent:@""];
     aboutItem.target = self;

     // Disable until there is a QAction
     aboutItem.enabled = NO;
     aboutItem.hidden  = YES;
     [appMenu addItem:aboutItem];

     aboutCsItem = [[NSMenuItem alloc] init];
     aboutCsItem.title = @"About CS";

     // Disable until there is a QAction
     aboutCsItem.enabled = NO;
     aboutCsItem.hidden  = YES;
     [appMenu addItem:aboutCsItem];

     [appMenu addItem:[NSMenuItem separatorItem]];

     // Preferences
     preferencesItem = [[NSMenuItem alloc] initWithTitle:@"Preferences"
                                                  action:@selector(qtDispatcherToQPAMenuItem:)
                                           keyEquivalent:@","];
     preferencesItem.target = self;

     // Disable until there is a QAction
     preferencesItem.enabled = NO;
     preferencesItem.hidden  = YES;
     [appMenu addItem:preferencesItem];

     [appMenu addItem:[NSMenuItem separatorItem]];

     // Services item and menu
     servicesItem = [[NSMenuItem alloc] init];
     servicesItem.title = @"Services";
     NSApplication *app = [NSApplication sharedApplication];
     app.servicesMenu   = [[[NSMenu alloc] initWithTitle:@"Services"] autorelease];
     servicesItem.submenu = app.servicesMenu;
     [appMenu addItem:servicesItem];

     [appMenu addItem:[NSMenuItem separatorItem]];

     // Hide Application
     hideItem = [[NSMenuItem alloc] initWithTitle:[@"Hide " stringByAppendingString:appName]
                                           action:@selector(hide:)
                                    keyEquivalent:@"h"];
     hideItem.target = self;
     [appMenu addItem:hideItem];

     // Hide Others
     hideAllOthersItem = [[NSMenuItem alloc] initWithTitle:@"Hide Others"
                                                    action:@selector(hideOtherApplications:)
                                             keyEquivalent:@"h"];
     hideAllOthersItem.target = self;
     hideAllOthersItem.keyEquivalentModifierMask = NSCommandKeyMask | NSAlternateKeyMask;
     [appMenu addItem:hideAllOthersItem];

     // Show All
     showAllItem = [[NSMenuItem alloc] initWithTitle:@"Show All"
                                              action:@selector(unhideAllApplications:)
                                       keyEquivalent:@""];
     showAllItem.target = self;
     [appMenu addItem:showAllItem];

     [appMenu addItem:[NSMenuItem separatorItem]];

     // Quit Application
     quitItem = [[NSMenuItem alloc] initWithTitle:[@"Quit " stringByAppendingString:appName]
                                           action:@selector(terminate:)
                                    keyEquivalent:@"q"];
     quitItem.target = self;
     [appMenu addItem:quitItem];
   }

   return self;
}
- (void)ensureAppMenuInMenu: (NSMenu *)menu
{
   // application menu is the menu in the menu bar that contains 'Quit'
   // When changing menu bar (e.g when switching between
   // windows with different menu bars), never recreate this menu,
   // instead pull it out the current menu bar and place into the new one

   NSMenu *mainMenu = [NSApp mainMenu];

   if ([NSApp mainMenu] == menu) {
      return;   // nothing to do (menu is the current menu bar)
   }

   Q_ASSERT(mainMenu);

   // Grab the app menu out of the current menu
   int numItems = [mainMenu numberOfItems];
   NSMenuItem *oldAppMenuItem = nullptr;

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
   [theMenu release];
   [appMenu release];
   [aboutItem release];
   [aboutCsItem release];
   [preferencesItem release];
   [servicesItem release];
   [hideItem release];
   [hideAllOthersItem release];
   [showAllItem release];
   [quitItem release];

   [lastAppSpecificItem release];

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

- (NSMenuItem *)aboutCsMenuItem
{
   return [[aboutCsItem retain] autorelease];
}

- (NSMenuItem *)hideMenuItem
{
   return [[hideItem retain] autorelease];
}

- (NSMenuItem *)appSpecificMenuItem: (NSInteger)tag
{
   NSMenuItem *item = [appMenu itemWithTag: tag];

   // No reason to create the item if it already exists
   if (item) {
      return [[item retain] autorelease];
   }

   // Create an App-Specific menu item, insert it into the menu and return
   // it as an autorelease item.
   item = [[NSMenuItem alloc] init];

   NSInteger location;
   if (lastAppSpecificItem == nil) {
      location = [appMenu indexOfItem: aboutCsItem];
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
   [NSApp terminate: sender];
}

- (void)orderFrontStandardAboutPanel: (id)sender
{
   [NSApp orderFrontStandardAboutPanel: sender];
}

- (void)hideOtherApplications: (id)sender
{
   [NSApp hideOtherApplications: sender];
}

- (void)unhideAllApplications: (id)sender
{
   [NSApp unhideAllApplications: sender];
}

- (void)hide: (id)sender
{
   [NSApp hide: sender];
}

- (void)qtTranslateApplicationMenu
{
   [servicesItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(0))];
   [hideItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(1).formatArg(qt_mac_applicationName()))];
   [hideAllOthersItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(2))];
   [showAllItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(3))];
   [preferencesItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(4))];
   [quitItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(5).formatArg(qt_mac_applicationName()))];
   [aboutItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(6).formatArg(qt_mac_applicationName()))];
}

- (IBAction)qtDispatcherToQPAMenuItem: (id)sender
{
   NSMenuItem *item = static_cast<NSMenuItem *>(sender);
   if (item == quitItem) {
      // We got here because someone was once the quitItem, but it has been
      // abandoned (e.g., the menubar was deleted). In the meantime, just do
      // normal QApplication::quit().
      qApp->quit();
      return;
   }

   if ([item tag]) {
      QCocoaMenuItem *cocoaItem = reinterpret_cast<QCocoaMenuItem *>([item tag]);
      QScopedLoopLevelCounter loopLevelCounter(QApplicationPrivate::instance()->getThreadData());
      cocoaItem->activated();
   }
}

- (void)orderFrontCharacterPalette: (id)sender
{
   [NSApp orderFrontCharacterPalette: sender];
}

- (BOOL)validateMenuItem: (NSMenuItem *)menuItem
{
   if ([menuItem action] == @selector(hide:)
      || [menuItem action] == @selector(hideOtherApplications:)
      || [menuItem action] == @selector(unhideAllApplications:)) {
      return [NSApp validateMenuItem: menuItem];

   } else if ([menuItem tag]) {
      QCocoaMenuItem *cocoaItem = reinterpret_cast<QCocoaMenuItem *>([menuItem tag]);
      return cocoaItem->isEnabled();

   } else {
      return [menuItem isEnabled];
   }
}

- (NSArray *) mergeable
{
   // do not include the quitItem here since we want it always visible and enabled regardless
   return [NSArray arrayWithObjects: preferencesItem, aboutItem, aboutCsItem, lastAppSpecificItem, nil];
}

@end
