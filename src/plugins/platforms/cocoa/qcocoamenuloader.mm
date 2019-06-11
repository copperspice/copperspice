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

#include "qcocoamenuloader.h"

#include "messages.h"
#include "qcocoahelpers.h"
#include "qcocoamenubar.h"
#include "qcocoamenuitem.h"

#include <qcoreapplication.h>
#include <qdir.h>
#include <qstring.h>
#include <qdebug.h>

#include <qapplication_p.h>
#include <qcore_mac_p.h>
#include <qthread_p.h>

class QCFString;

/*
    Loads and instantiates the main app menu from the menu nib file(s).

    The main app menu contains the Quit, Hide  About, Preferences entries, and
    The reason for having the nib file is that those can not be created
    programmatically. To ease deployment the nib files are stored in Qt resources
    and written to QDir::temp() before loading. (Earlier Qt versions used
    to require having the nib file in the Qt GUI framework.)
*/
void qt_mac_loadMenuNib(QCocoaMenuLoader *qtMenuLoader)
{
   // Create qt_menu.nib dir in temp.
   QDir temp = QDir::temp();
   temp.mkdir("qt_menu.nib");

   QString nibDir = temp.canonicalPath() + "/qt_menu.nib/";

   if (! QDir(nibDir).exists()) {
      qWarning("qt_mac_loadMenuNib: could not create nib directory in temp");
      return;
   }

   // Copy nib files from resources to temp
   QDir nibResource(":/copperspice/mac/qt_menu.nib/");

   if (! nibResource.exists()) {
      qWarning("qt_mac_loadMenuNib: Unable to load nib from resources");
      return;
   }

   for (const QFileInfo &file : nibResource.entryInfoList()) {
      QFileInfo destinationFile(nibDir + QLatin1String("/") + file.fileName());
      if (destinationFile.exists() && destinationFile.size() != file.size()) {
         QFile::remove(destinationFile.absoluteFilePath());
      }

      QFile::copy(file.absoluteFilePath(), destinationFile.absoluteFilePath());
   }

   // Load and instantiate nib file from temp
   NSURL *nibUrl = [NSURL fileURLWithPath: QCFString::toNSString(nibDir)];
   NSNib *nib = [[NSNib alloc] initWithContentsOfURL: nibUrl];
   [nib autorelease];

   if (!nib) {
      qWarning("qt_mac_loadMenuNib: could not load nib from  temp");
      return;
   }

   bool ok = [nib instantiateNibWithOwner: qtMenuLoader topLevelObjects: nil];
   if (! ok) {
      qWarning("qt_mac_loadMenuNib: could not instantiate nib");
   }
}

@implementation QCocoaMenuLoader

- (void)awakeFromNib
{
   servicesItem = [[appMenu itemWithTitle: @"Services"] retain];
   hideAllOthersItem = [[appMenu itemWithTitle: @"Hide Others"] retain];
   showAllItem = [[appMenu itemWithTitle: @"Show All"] retain];

   // Get the names in the nib to match the app name set by CS
   const NSString *appName = qt_mac_applicationName().toNSString();

   [quitItem setTitle: [[quitItem title] stringByReplacingOccurrencesOfString: @"NewApplication"
                  withString: const_cast<NSString *>(appName)]];
   [hideItem setTitle: [[hideItem title] stringByReplacingOccurrencesOfString: @"NewApplication"
                  withString: const_cast<NSString *>(appName)]];
   [aboutItem setTitle: [[aboutItem title] stringByReplacingOccurrencesOfString: @"NewApplication"
                   withString: const_cast<NSString *>(appName)]];

   // Disable the items that don't do anything. If someone associates a QAction with them
   // They should get synced back in.
   [preferencesItem setEnabled: NO];
   [preferencesItem setHidden: YES];

   // should set this in the NIB
   [preferencesItem setTarget: self];
   [preferencesItem setAction: @selector(qtDispatcherToQPAMenuItem:)];

   [aboutItem setEnabled: NO];
   [aboutItem setHidden: YES];
}

- (void)ensureAppMenuInMenu: (NSMenu *)menu
{
   // The application menu is the menu in the menu bar that contains the
   // 'Quit' item. When changing menu bar (e.g when switching between
   // windows with different menu bars), we never recreate this menu, but
   // instead pull it out the current menu bar and place into the new one:
   NSMenu *mainMenu = [NSApp mainMenu];

   if ([NSApp mainMenu] == menu) {
      return;   // nothing to do (menu is the current menu bar)
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

#ifndef QT_NO_TRANSLATION
   [servicesItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(0))];
   [hideItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(1).formatArg(qt_mac_applicationName()))];
   [hideAllOthersItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(2))];
   [showAllItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(3))];
   [preferencesItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(4))];
   [quitItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(5).formatArg(qt_mac_applicationName()))];
   [aboutItem setTitle: QCFString::toNSString(qt_mac_applicationmenu_string(6).formatArg(qt_mac_applicationName()))];
#endif
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
   // don't include the quitItem here, since we want it always visible and enabled regardless
   return [NSArray arrayWithObjects: preferencesItem, aboutItem, aboutQtItem, lastAppSpecificItem, nil];
}

@end
