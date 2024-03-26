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

#import "qcocoaapplicationdelegate.h"
#import "qnswindowdelegate.h"
#import "qcocoamenuloader.h"
#include "qcocoaintegration.h"
#include <qevent.h>
#include <qurl.h>
#include <qdebug.h>
#include <qapplication.h>
#include <qapplication_p.h>
#include "qt_mac_p.h"
#include <qwindowsysteminterface.h>

static QCocoaApplicationDelegate *sharedCocoaApplicationDelegate = nil;

static void cleanupCocoaApplicationDelegate()
{
   [sharedCocoaApplicationDelegate release];
}

@implementation QCocoaApplicationDelegate

- (id)init
{
   self = [super init];
   if (self) {
      inLaunch = true;
      [[NSNotificationCenter defaultCenter]
         addObserver: self
            selector: @selector(updateScreens:)
                name: NSApplicationDidChangeScreenParametersNotification
              object: NSApp];
   }
   return self;
}

- (void)updateScreens: (NSNotification *)notification
{
   (void) notification;
   if (QCocoaIntegration *ci = QCocoaIntegration::instance()) {
      ci->updateScreens();
   }
}

- (void)dealloc
{
   sharedCocoaApplicationDelegate = nil;
   [dockMenu release];
   [qtMenuLoader release];
   if (reflectionDelegate) {
      [[NSApplication sharedApplication] setDelegate: reflectionDelegate];
      [reflectionDelegate release];
   }
   [[NSNotificationCenter defaultCenter] removeObserver: self];

   [super dealloc];
}

+ (id)allocWithZone: (NSZone *)zone
{
   @synchronized(self) {
      if (sharedCocoaApplicationDelegate == nil) {
         sharedCocoaApplicationDelegate = [super allocWithZone: zone];
         qAddPostRoutine(cleanupCocoaApplicationDelegate);
         return sharedCocoaApplicationDelegate;
      }
   }
   return nil;
}

+ (QCocoaApplicationDelegate *)sharedDelegate
{
   @synchronized(self) {
      if (sharedCocoaApplicationDelegate == nil) {
         [[self alloc] init];
      }
   }
   return [[sharedCocoaApplicationDelegate retain] autorelease];
}

- (void)setDockMenu: (NSMenu *)newMenu
{
   [newMenu retain];
   [dockMenu release];
   dockMenu = newMenu;
}

- (NSMenu *)applicationDockMenu: (NSApplication *)sender
{
   (void) sender;
   // Manually invoke the delegate's -menuWillOpen: method.
   // See QTBUG-39604 (and its fix) for details.
   [[dockMenu delegate] menuWillOpen: dockMenu];
   return [[dockMenu retain] autorelease];
}

- (void)setMenuLoader: (QCocoaMenuLoader *)menuLoader
{
   [menuLoader retain];
   [qtMenuLoader release];
   qtMenuLoader = menuLoader;
}

- (QCocoaMenuLoader *)menuLoader
{
   return [[qtMenuLoader retain] autorelease];
}

- (BOOL) canQuit
{
   [[NSApp mainMenu] cancelTracking];

   bool handle_quit = true;
   NSMenuItem *quitMenuItem = [[[QCocoaApplicationDelegate sharedDelegate] menuLoader] quitMenuItem];

   if (! QApplicationPrivate::instance()->modalWindowList.isEmpty() && [quitMenuItem isEnabled]) {
      int visible = 0;
      const QWindowList tlws = QApplication::topLevelWindows();

      for (int i = 0; i < tlws.size(); ++i) {
         if (tlws.at(i)->isVisible()) {
            ++visible;
         }
      }
      handle_quit = (visible <= 1);
   }

   if (handle_quit) {
      QCloseEvent ev;
      QApplication::sendEvent(qGuiApp, &ev);
      if (ev.isAccepted()) {
         return YES;
      }
   }

   return NO;
}

// This function will only be called when NSApp is actually running.
- (NSApplicationTerminateReply)applicationShouldTerminate: (NSApplication *)sender
{
   // The reflection delegate gets precedence
   if (reflectionDelegate) {
      if ([reflectionDelegate respondsToSelector: @selector(applicationShouldTerminate:)]) {
         return [reflectionDelegate applicationShouldTerminate: sender];
      }
      return NSTerminateNow;
   }

   if ([self canQuit]) {
      if (!startedQuit) {
         startedQuit = true;
         // Close open windows. This is done in order to deliver de-expose
         // events while the event loop is still running.
         const QWindowList topLevels = QApplication::topLevelWindows();
         for (int i = 0; i < topLevels.size(); ++i) {
            QWindow *topLevelWindow = topLevels.at(i);
            // Already closed windows will not have a platform window, skip those
            if (topLevelWindow->handle()) {
               QWindowSystemInterface::handleCloseEvent(topLevelWindow);
            }
         }
         QWindowSystemInterface::flushWindowSystemEvents();

         QApplication::exit(0);
         startedQuit = false;
      }
   }

   if (QApplicationPrivate::instance()->getThreadData()->eventLoops.isEmpty()) {
      // INVARIANT: No event loop is executing. This probably means
      // CS is used as a plugin, or as a part of a native Cocoa application.
      // In any case it should be fine to terminate now

      return NSTerminateNow;
   }

   return NSTerminateCancel;
}

- (void) applicationWillFinishLaunching: (NSNotification *)notification
{
   (void) notification;

   /*
       From the Cocoa documentation: "A good place to install event handlers
       is in the applicationWillFinishLaunching: method of the application
       delegate. At that point, the Application Kit has installed its default
       event handlers, so if you install a handler for one of the same events,
       it will replace the Application Kit version."
   */

   /*
       If CS is used as a plugin, we let the 3rd party application handle
       events like quit and open file events. Otherwise, if we install our own
       handlers, we easily end up breaking functionality the 3rd party
       application depends on.
    */
   NSAppleEventManager *eventManager = [NSAppleEventManager sharedAppleEventManager];
   [eventManager setEventHandler: self
                     andSelector: @selector(appleEventQuit: withReplyEvent:)
                   forEventClass: kCoreEventClass
                      andEventID: kAEQuitApplication];
   [eventManager setEventHandler: self
                     andSelector: @selector(getUrl: withReplyEvent:)
                   forEventClass: kInternetEventClass
                      andEventID: kAEGetURL];
}

// called by QCocoaIntegration's destructor before resetting the application delegate to nil
- (void) removeAppleEventHandlers
{
   NSAppleEventManager *eventManager = [NSAppleEventManager sharedAppleEventManager];
   [eventManager removeEventHandlerForEventClass: kCoreEventClass andEventID: kAEQuitApplication];
   [eventManager removeEventHandlerForEventClass: kInternetEventClass andEventID: kAEGetURL];
}

- (bool) inLaunch
{
   return inLaunch;
}

- (void)applicationDidFinishLaunching: (NSNotification *)aNotification
{
   inLaunch = false;

   if (qgetenv("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM").isEmpty()) {

      if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_12) {
         // Move the application window to front to avoid launching behind the terminal.
         // Ignoring other apps is necessary (we must ignore the terminal), but makes
         // CSt apps play slightly less nice with other apps when lanching from Finder
         // (See the activateIgnoringOtherApps docs.)

         [[NSApplication sharedApplication] activateIgnoringOtherApps: YES];
      }
   }
}

- (void)application: (NSApplication *)sender openFiles: (NSArray *)filenames
{
   (void) filenames;
   (void) sender;

   for (NSString * fileName in filenames) {
      QString qtFileName = QCFString::toQString(fileName);
      if (inLaunch) {
         // We need to be careful because Cocoa will be nice enough to take
         // command line arguments and send them to us as events. Given the history
         // of CS Applications, this will result in behavior people don't want, as
         // they might be doing the opening themselves with the command line parsing.
         if (qApp->arguments().contains(qtFileName)) {
            continue;
         }
      }
      QWindowSystemInterface::handleFileOpenEvent(qtFileName);
   }

   if (reflectionDelegate &&
      [reflectionDelegate respondsToSelector: @selector(application: openFiles:)]) {
      [reflectionDelegate application: sender openFiles: filenames];
   }

}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed: (NSApplication *)sender
{
   // If we have a reflection delegate, that will get to call the shots.
   if (reflectionDelegate
      && [reflectionDelegate respondsToSelector:
   @selector(applicationShouldTerminateAfterLastWindowClosed:)]) {
      return [reflectionDelegate applicationShouldTerminateAfterLastWindowClosed: sender];
   }
   return NO; // Someday qApp->quitOnLastWindowClosed(); when QApp and NSApp work closer together.
}


- (void)applicationDidBecomeActive: (NSNotification *)notification
{
   if (reflectionDelegate
      && [reflectionDelegate respondsToSelector: @selector(applicationDidBecomeActive:)]) {
      [reflectionDelegate applicationDidBecomeActive: notification];
   }

   QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationActive);
   /*
       onApplicationChangedActivation(true);

       if (!QWidget::mouseGrabber()){
           // Update enter/leave immidiatly, don't wait for a move event. But only
           // if no grab exists (even if the grab points to this widget, it seems, ref X11)
           QPoint qlocal, qglobal;
           QWidget *widgetUnderMouse = 0;
           qt_mac_getTargetForMouseEvent(0, QEvent::Enter, qlocal, qglobal, 0, &widgetUnderMouse);
           QApplicationPrivate::dispatchEnterLeave(widgetUnderMouse, 0);
           qt_last_mouse_receiver = widgetUnderMouse;
           qt_last_native_mouse_receiver = widgetUnderMouse ?
               (widgetUnderMouse->internalWinId() ? widgetUnderMouse : widgetUnderMouse->nativeParentWidget()) : 0;
       }
   */
}

- (void)applicationDidResignActive: (NSNotification *)notification
{
   if (reflectionDelegate
      && [reflectionDelegate respondsToSelector: @selector(applicationDidResignActive:)]) {
      [reflectionDelegate applicationDidResignActive: notification];
   }

   QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationInactive);
   /*
       onApplicationChangedActivation(false);

       if (!QWidget::mouseGrabber())
           QApplicationPrivate::dispatchEnterLeave(0, qt_last_mouse_receiver);
       qt_last_mouse_receiver = 0;
       qt_last_native_mouse_receiver = 0;
       qt_button_down = 0;
   */
}

- (BOOL)applicationShouldHandleReopen: (NSApplication *)theApplication hasVisibleWindows: (BOOL)flag
{
   (void) theApplication;
   (void) flag;
   if (reflectionDelegate
      && [reflectionDelegate respondsToSelector: @selector(applicationShouldHandleReopen: hasVisibleWindows:)]) {
      return [reflectionDelegate applicationShouldHandleReopen: theApplication hasVisibleWindows: flag];
   }

   /*
      true to force delivery of the event even if the application state is already active,
      because rapp (handle reopen) events are sent each time the dock icon is clicked regardless
      of the active state of the application or number of visible windows. For example, a browser
      app that has no windows opened would need the event be to delivered even if it was already
      active in order to create a new window as per OS X conventions.
    */
   QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationActive, true);

   return YES;
}

- (void)setReflectionDelegate: (NSObject <NSApplicationDelegate> *)oldDelegate
{
   [oldDelegate retain];
   [reflectionDelegate release];
   reflectionDelegate = oldDelegate;
}

- (NSMethodSignature *)methodSignatureForSelector: (SEL)aSelector
{
   NSMethodSignature *result = [super methodSignatureForSelector: aSelector];
   if (!result && reflectionDelegate) {
      result = [reflectionDelegate methodSignatureForSelector: aSelector];
   }
   return result;
}

- (BOOL)respondsToSelector: (SEL)aSelector
{
   BOOL result = [super respondsToSelector: aSelector];
   if (!result && reflectionDelegate) {
      result = [reflectionDelegate respondsToSelector: aSelector];
   }
   return result;
}

- (void)forwardInvocation: (NSInvocation *)invocation
{
   SEL invocationSelector = [invocation selector];
   if (reflectionDelegate && [reflectionDelegate respondsToSelector: invocationSelector]) {
      [invocation invokeWithTarget: reflectionDelegate];
   } else {
      [self doesNotRecognizeSelector: invocationSelector];
   }
}

- (void)getUrl: (NSAppleEventDescriptor *)event withReplyEvent: (NSAppleEventDescriptor *)replyEvent
{
   (void) replyEvent;
   NSString *urlString = [[event paramDescriptorForKeyword: keyDirectObject] stringValue];
   QWindowSystemInterface::handleFileOpenEvent(QUrl(QCFString::toQString(urlString)));
}

- (void)appleEventQuit: (NSAppleEventDescriptor *)event withReplyEvent: (NSAppleEventDescriptor *)replyEvent
{
   (void) event;
   (void) replyEvent;
   [NSApp terminate: self];
}

- (void)qtDispatcherToQAction: (id)sender
{
   (void) sender;
   [qtMenuLoader qtDispatcherToQPAMenuItem: sender];
}

@end
