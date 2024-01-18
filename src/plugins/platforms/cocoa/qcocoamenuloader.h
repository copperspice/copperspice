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

#ifndef QCOCOAMENULOADER_P_H
#define QCOCOAMENULOADER_P_H

#import <Cocoa/Cocoa.h>

#include <qcore_mac_p.h>

@interface QCocoaMenuLoader : NSResponder
{
   IBOutlet NSMenu *theMenu;
   IBOutlet NSMenu *appMenu;
   IBOutlet NSMenuItem *quitItem;
   IBOutlet NSMenuItem *preferencesItem;
   IBOutlet NSMenuItem *aboutItem;
   IBOutlet NSMenuItem *aboutCsItem;
   IBOutlet NSMenuItem *hideItem;
   NSMenuItem *lastAppSpecificItem;
   NSMenuItem *servicesItem;
   NSMenuItem *hideAllOthersItem;
   NSMenuItem *showAllItem;
}
- (instancetype)init;
- (void)ensureAppMenuInMenu: (NSMenu *)menu;
- (void)removeActionsFromAppMenu;
- (NSMenu *)applicationMenu;
- (NSMenu *)menu;
- (NSMenuItem *)quitMenuItem;
- (NSMenuItem *)preferencesMenuItem;
- (NSMenuItem *)aboutMenuItem;
- (NSMenuItem *)aboutCsMenuItem;
- (NSMenuItem *)hideMenuItem;
- (NSMenuItem *)appSpecificMenuItem: (NSInteger)tag;
- (IBAction)terminate: (id)sender;
- (IBAction)orderFrontStandardAboutPanel: (id)sender;
- (IBAction)hideOtherApplications: (id)sender;
- (IBAction)unhideAllApplications: (id)sender;
- (IBAction)hide: (id)sender;
- (IBAction)qtDispatcherToQPAMenuItem: (id)sender;
- (void)orderFrontCharacterPalette: (id)sender;
- (BOOL)validateMenuItem: (NSMenuItem *)menuItem;
- (void)qtTranslateApplicationMenu;
- (NSArray *)mergeable;
@end

#endif
