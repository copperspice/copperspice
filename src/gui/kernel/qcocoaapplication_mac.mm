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

/***********************************************************************
** Copyright (C) 2007-2008, Apple, Inc.
***********************************************************************/

#include <qglobal.h>
#include <qcocoaapplication_mac_p.h>
#include <qcocoaapplicationdelegate_mac_p.h>
#include <qt_cocoa_helpers_mac_p.h>
#include <qcocoaintrospection_p.h>

QT_USE_NAMESPACE

@implementation NSApplication (QT_MANGLE_NAMESPACE(QApplicationIntegration))

- (void)QT_MANGLE_NAMESPACE(qt_setDockMenu): (NSMenu *)newMenu
{
   [[QT_MANGLE_NAMESPACE(QCocoaApplicationDelegate) sharedDelegate] setDockMenu: newMenu];
}

- (QApplicationPrivate *)QT_MANGLE_NAMESPACE(qt_qappPrivate)
{
   return [[QT_MANGLE_NAMESPACE(QCocoaApplicationDelegate) sharedDelegate] qAppPrivate];
}

- (QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *)QT_MANGLE_NAMESPACE(qt_qcocoamenuLoader)
{
   return [[QT_MANGLE_NAMESPACE(QCocoaApplicationDelegate) sharedDelegate] menuLoader];
}

- (int)QT_MANGLE_NAMESPACE(qt_validModesForFontPanel): (NSFontPanel *)fontPanel
{
   Q_UNUSED(fontPanel);
   // only display those things that QFont can handle
   return NSFontPanelFaceModeMask
          | NSFontPanelSizeModeMask
          | NSFontPanelCollectionModeMask
          | NSFontPanelUnderlineEffectModeMask
          | NSFontPanelStrikethroughEffectModeMask;
}

- (void)QT_MANGLE_NAMESPACE(qt_sendPostedMessage): (NSEvent *)event
{
   // WARNING: data1 and data2 is truncated to from 64-bit to 32-bit on OS 10.5!
   // That is why we need to split the address in two parts:
   quint64 lower = [event data1];
   quint64 upper = [event data2];
   QCocoaPostMessageArgs *args = reinterpret_cast<QCocoaPostMessageArgs *>(lower | (upper << 32));
   // Special case for convenience: if the argument is an NSNumber, we unbox it directly.
   // Use NSValue instead if this behaviour is unwanted.
   id a1 = ([args->arg1 isKindOfClass: [NSNumber class]]) ? (id)[args->arg1 intValue] : args->arg1;
   id a2 = ([args->arg2 isKindOfClass: [NSNumber class]]) ? (id)[args->arg2 intValue] : args->arg2;
   switch (args->argCount) {
      case 0:
         [args->target performSelector: args->selector];
         break;
      case 1:
         [args->target performSelector: args->selector withObject: a1];
         break;
      case 3:
         [args->target performSelector: args->selector withObject: a1 withObject: a2];
         break;
   }

   delete args;
}

- (BOOL)QT_MANGLE_NAMESPACE(qt_filterEvent): (NSEvent *)event
{
   if (!qApp) {
      return false;
   }

   if (qApp->macEventFilter(0, reinterpret_cast<EventRef>(event))) {
      return true;
   }

   if ([event type] == NSApplicationDefined) {
      switch ([event subtype]) {
         case QtCocoaEventSubTypePostMessage:
            [[NSApplication sharedApplication] QT_MANGLE_NAMESPACE(qt_sendPostedMessage): event];
            return true;
         default:
            break;
      }
   }
   return false;
}

@end

@implementation QT_MANGLE_NAMESPACE(QNSApplication)

- (void)QT_MANGLE_NAMESPACE(qt_sendEvent_original): (NSEvent *)event
{
   Q_UNUSED(event);
   // This method will only be used as a signature
   // template for the method we add into NSApplication
   // containing the original [NSApplication sendEvent:] implementation
}

- (void)QT_MANGLE_NAMESPACE(qt_sendEvent_replacement): (NSEvent *)event
{
   // This method (or its implementation to be precise) will
   // be called instead of sendEvent if redirection occurs.
   // 'self' will then be an instance of NSApplication
   // (and not QNSApplication)
   if (![[NSApplication sharedApplication] QT_MANGLE_NAMESPACE(qt_filterEvent): event]) {
      [self QT_MANGLE_NAMESPACE(qt_sendEvent_original): event];
   }
}

- (void)sendEvent: (NSEvent *)event
{
   // This method will be called if
   // no redirection occurs
   if (![[NSApplication sharedApplication] QT_MANGLE_NAMESPACE(qt_filterEvent): event]) {
      [super sendEvent: event];
   }
}

- (void)qtDispatcherToQAction: (id)sender
{
   // Forward actions sendt from the menu bar (e.g. quit) to the menu loader.
   // Having this method here means that we are the last stop in the responder
   // chain, and that we are able to handle menu actions even when no window is
   // visible on screen. Note: If Qt is used as a plugin, Qt will not use a
   // native menu bar. Hence, we will also not need to do any redirection etc. as
   // we do with sendEvent.
   [[[NSApplication sharedApplication] QT_MANGLE_NAMESPACE(qt_qcocoamenuLoader)] qtDispatcherToQAction: sender];
}

@end

QT_BEGIN_NAMESPACE

void qt_redirectNSApplicationSendEvent()
{
   if ([[NSApplication sharedApplication] isMemberOfClass: [QT_MANGLE_NAMESPACE(QNSApplication) class]]) {
      // No need to change implementation since Qt
      // already controls a subclass of NSApplication
      return;
   }

   // Change the implementation of [NSApplication sendEvent] to the
   // implementation of qt_sendEvent_replacement found in QNSApplication.
   // And keep the old implementation that gets overwritten inside a new
   // method 'qt_sendEvent_original' that we add to NSApplication
   qt_cocoa_change_implementation(
      [NSApplication class],
      @selector(sendEvent:),
      [QT_MANGLE_NAMESPACE(QNSApplication) class],
      @selector(QT_MANGLE_NAMESPACE(qt_sendEvent_replacement):),
      @selector(QT_MANGLE_NAMESPACE(qt_sendEvent_original):));
}

void qt_resetNSApplicationSendEvent()
{
   qt_cocoa_change_back_implementation([NSApplication class],
                                       @selector(sendEvent:),
                                       @selector(QT_MANGLE_NAMESPACE(qt_sendEvent_original):));
}

QT_END_NAMESPACE

