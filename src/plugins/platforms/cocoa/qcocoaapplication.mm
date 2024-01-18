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

/***********************************************************************
* Copyright (c) 2007-2008, Apple, Inc.
* All rights reserved.
*
* Refer to APPLE_LICENSE.TXT (in this directory) for license terms
***********************************************************************/

#include <qcocoaapplication.h>

#include <qcocoaintrospection.h>
#include <qcocoaapplicationdelegate.h>
#include <qcocoahelpers.h>
#include <qapplication.h>
#include <qdebug.h>

@implementation NSApplication (QApplicationIntegration)

- (void) qt_setDockMenu: (NSMenu *)newMenu
{
   [[QCocoaApplicationDelegate sharedDelegate] setDockMenu: newMenu];
}

- (QCocoaMenuLoader *) qt_qcocoamenuLoader
{
   return [[QCocoaApplicationDelegate sharedDelegate] menuLoader];
}

- (int)qt_validModesForFontPanel: (NSFontPanel *)fontPanel
{
   // only display those things that QFont can handle

   return NSFontPanelFaceModeMask
      | NSFontPanelSizeModeMask
      | NSFontPanelCollectionModeMask
      | NSFontPanelUnderlineEffectModeMask
      | NSFontPanelStrikethroughEffectModeMask;
}

- (void) qt_sendPostedMessage: (NSEvent *)event
{
   // WARNING: data1 and data2 is truncated to from 64-bit to 32-bit on OS 10.5
   // That is why we need to split the address in two parts:

   quint64 lower = [event data1];
   quint64 upper = [event data2];
   QCocoaPostMessageArgs *args = reinterpret_cast<QCocoaPostMessageArgs *>(lower | (upper << 32));

   // Special case for convenience: if the argument is an NSNumber, we unbox it directly.
   // Use NSValue instead if this behaviour is unwanted.

   id a1 = ([args->arg1 isKindOfClass: [NSNumber class]]) ? (id)[args->arg1 longValue] : args->arg1;
   id a2 = ([args->arg2 isKindOfClass: [NSNumber class]]) ? (id)[args->arg2 longValue] : args->arg2;

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

static const QByteArray q_macLocalEventType = "mac_generic_NSEvent";

- (BOOL) qt_filterEvent: (NSEvent *)event
{
   if (qApp && qApp->eventDispatcher()->filterNativeEvent(q_macLocalEventType, static_cast<void *>(event), nullptr)) {
      return true;
   }

   if ([event type] == NSEventTypeApplicationDefined) {
      switch (static_cast<short>([event subtype])) {
         case QtCocoaEventSubTypePostMessage:
            [NSApp qt_sendPostedMessage: event];
            return true;

         default:
            break;
      }
   }

   return false;
}

@end

@implementation QNSApplication

- (void) qt_sendEvent_original: (NSEvent *)event
{
   (void) event;
   // This method will only be used as a signature
   // template for the method we add into NSApplication
   // containing the original [NSApplication sendEvent:] implementation
}

- (void) qt_sendEvent_replacement: (NSEvent *)event
{
   // This method (or its implementation to be precise) will
   // be called instead of sendEvent if redirection occurs.
   // 'self' will then be an instance of NSApplication
   // (and not QNSApplication)

   if (! [NSApp qt_filterEvent: event]) {
      [self qt_sendEvent_original: event];
   }
}

- (void) sendEvent: (NSEvent *)event
{
   // This method will be called if
   // no redirection occurs
   if (! [NSApp qt_filterEvent: event]) {
      [super sendEvent: event];
   }
}

@end

void qt_redirectNSApplicationSendEvent()
{
   if (QCoreApplication::testAttribute(Qt::AA_MacPluginApplication)) {
      // In a plugin we can not chain sendEvent hooks: a second plugin could store
      // the implementation of the first, which during the program flow can be unloaded

      return;
   }

   if ([NSApp isMemberOfClass: [QNSApplication class]]) {
      // No need to change implementation since we
      // already control a subclass of NSApplication
      return;
   }

   // Change the implementation of [NSApplication sendEvent] to the
   // implementation of qt_sendEvent_replacement found in QNSApplication.
   // And keep the old implementation that gets overwritten inside a new
   // method 'qt_sendEvent_original' that we add to NSApplication

   qt_cocoa_change_implementation(
      [NSApplication class],
      @selector(sendEvent:),
      [QNSApplication class],
      @selector(qt_sendEvent_replacement:),
      @selector(qt_sendEvent_original:));
}

void qt_resetNSApplicationSendEvent()
{
   if (QCoreApplication::testAttribute(Qt::AA_MacPluginApplication)) {
      return;
   }

   qt_cocoa_change_back_implementation([NSApplication class],
      @selector(sendEvent:),
      @selector(qt_sendEvent_original:));
}

