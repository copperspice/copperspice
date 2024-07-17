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

#include <qglobal.h>

#include <Carbon/Carbon.h>

#include <qnsview.h>
#include <qcocoawindow.h>
#include <qcocoahelpers.h>
#include <qcocoadrag.h>
#include <qcocoainputcontext.h>
#include <qplatform_integration.h>
#include <qwindowsysteminterface.h>
#include <qtextformat.h>
#include <qdebug.h>
#include <qsysinfo.h>
#include <qcocoabackingstore.h>

#include <qapplication_p.h>
#include <qmultitouch_mac_p.h>

#ifndef QT_NO_OPENGL
#include <qcocoaglcontext.h>
#endif

#include <qcocoaintegration.h>

#ifdef QT_COCOA_ENABLE_ACCESSIBILITY_INSPECTOR
#include <accessibilityinspector.h>
#endif

static QTouchDevice *touchDevice   = nullptr;
static bool _q_dontOverrideCtrlLMB = false;

@interface NSEvent (Qt_Compile_Leopard_DeviceDelta)
- (CGFloat)deviceDeltaX;
- (CGFloat)deviceDeltaY;
- (CGFloat)deviceDeltaZ;
@end

@interface QNSViewMouseMoveHelper : NSObject
{
   QNSView *view;
}

- (id)initWithView: (QNSView *)theView;

- (void)mouseMoved: (NSEvent *)theEvent;
- (void)mouseEntered: (NSEvent *)theEvent;
- (void)mouseExited: (NSEvent *)theEvent;
- (void)cursorUpdate: (NSEvent *)theEvent;

@end

@implementation QNSViewMouseMoveHelper

- (id)initWithView: (QNSView *)theView
{
   self = [super init];
   if (self) {
      view = theView;
   }
   return self;
}

- (void)mouseMoved: (NSEvent *)theEvent
{
   [view mouseMovedImpl: theEvent];
}

- (void)mouseEntered: (NSEvent *)theEvent
{
   [view mouseEnteredImpl: theEvent];
}

- (void)mouseExited: (NSEvent *)theEvent
{
   [view mouseExitedImpl: theEvent];
}

- (void)cursorUpdate: (NSEvent *)theEvent
{
   [self cursorUpdate: theEvent];
}

@end

@implementation QNSView

+ (void)initialize
{
   _q_dontOverrideCtrlLMB = qt_mac_resolveOption(false, "QT_MAC_DONT_OVERRIDE_CTRL_LMB");
}

- (id) init
{
   self = [super initWithFrame: NSMakeRect(0, 0, 300, 300)];

   if (self) {
      m_backingStore      = nullptr;
      m_maskImage         = nullptr;
      m_shouldInvalidateWindowShadow = false;
      m_window            = nullptr;
      m_buttons           = Qt::NoButton;
      m_frameStrutButtons = Qt::NoButton;
      m_sendKeyEvent      = false;
      m_subscribesForGlobalFrameNotifications = false;

#ifndef QT_NO_OPENGL
      m_glContext = nullptr;
      m_shouldSetGLContextinDrawRect = false;
#endif

      currentCustomDragTypes = nullptr;
      m_sendUpAsRightButton  = false;
      m_inputSource     = nullptr;
      m_mouseMoveHelper = [[QNSViewMouseMoveHelper alloc] initWithView: self];
      m_resendKeyEvent  = false;
      m_scrolling       = false;
      m_updatingDrag    = false;
      m_currentlyInterpretedKeyEvent = nullptr;

      if (!touchDevice) {
         touchDevice = new QTouchDevice;
         touchDevice->setType(QTouchDevice::TouchPad);
         touchDevice->setCapabilities(QTouchDevice::Position | QTouchDevice::NormalizedPosition | QTouchDevice::MouseEmulation);
         QWindowSystemInterface::registerTouchDevice(touchDevice);
      }

      m_isMenuView = false;
      self.focusRingType = NSFocusRingTypeNone;
   }

   return self;
}

- (void)dealloc
{
   CGImageRelease(m_maskImage);
   [m_trackingArea release];
   m_maskImage = nullptr;
   m_window    = nullptr;

   m_subscribesForGlobalFrameNotifications = false;
   [m_inputSource release];
   [[NSNotificationCenter defaultCenter] removeObserver: self];
   [m_mouseMoveHelper release];

   delete currentCustomDragTypes;

   [super dealloc];
}

- (id)initWithQWindow: (QWindow *)window platformWindow: (QCocoaWindow *) platformWindow
{
   self = [self init];
   if (! self) {
      return nullptr;
   }

   m_window = window;
   m_platformWindow = platformWindow;
   m_sendKeyEvent = false;
   m_trackingArea = nil;

#ifdef QT_COCOA_ENABLE_ACCESSIBILITY_INSPECTOR
   // prevent rift in space-time continuum, disable
   // accessibility for the accessibility inspector's windows.
   static bool skipAccessibilityForInspectorWindows = false;
   if (!skipAccessibilityForInspectorWindows) {

      // m_accessibleRoot = window->accessibleRoot();

      AccessibilityInspector *inspector = new AccessibilityInspector(window);
      skipAccessibilityForInspectorWindows = true;
      inspector->inspectWindow(window);
      skipAccessibilityForInspectorWindows = false;
   }
#endif

   [self registerDragTypes];
   [self setPostsFrameChangedNotifications: YES];
   [[NSNotificationCenter defaultCenter] addObserver: self
                                            selector: @selector(updateGeometry)
                                                name: NSViewFrameDidChangeNotification
                                              object: self];

   [[NSNotificationCenter defaultCenter] addObserver: self
                                            selector: @selector(textInputContextKeyboardSelectionDidChangeNotification:)
                                                name: NSTextInputContextKeyboardSelectionDidChangeNotification
                                              object: nil];

   return self;
}

- (void) clearQWindowPointers
{
   m_window = nullptr;
   m_platformWindow = nullptr;
}

#ifndef QT_NO_OPENGL
- (void) setQCocoaGLContext: (QCocoaGLContext *)context
{
   m_glContext = context;
   [m_glContext->nsOpenGLContext() setView: self];
   if (![m_glContext->nsOpenGLContext() view]) {
      //was unable to set view
      m_shouldSetGLContextinDrawRect = true;
   }

   if (!m_subscribesForGlobalFrameNotifications) {
      // NSOpenGLContext expects us to repaint (or update) the view when
      // it changes position on screen. Since this happens unnoticed for
      // the view when the parent view moves, we need to register a special
      // notification that lets us handle this case:
      m_subscribesForGlobalFrameNotifications = true;
      [[NSNotificationCenter defaultCenter] addObserver: self
                                               selector: @selector(globalFrameChanged:)
                                                   name: NSViewGlobalFrameDidChangeNotification
                                                 object: self];
   }
}
#endif

- (void) globalFrameChanged: (NSNotification *)notification
{
   (void) notification;
   m_platformWindow->updateExposedGeometry();
}

- (void)viewDidMoveToSuperview
{
   if (!(m_platformWindow->m_contentViewIsToBeEmbedded)) {
      return;
   }

   if ([self superview]) {
      m_platformWindow->m_contentViewIsEmbedded = true;
      QWindowSystemInterface::handleGeometryChange(m_window, m_platformWindow->geometry());
      m_platformWindow->updateExposedGeometry();
      QWindowSystemInterface::flushWindowSystemEvents();
   } else {
      m_platformWindow->m_contentViewIsEmbedded = false;
   }
}

- (void)viewDidMoveToWindow
{
   m_backingStore = nullptr;

   m_isMenuView = [self.window.className isEqualToString: @"NSCarbonMenuWindow"];

   if (self.window) {
      // get here when WidgetAction  inserted in an NSMenu
      // 10.9 and newer get the NSWindowDidChangeOcclusionStateNotification

      if (! NSWindowDidChangeOcclusionStateNotification && m_isMenuView) {
         m_exposedOnMoveToWindow = true;
         m_platformWindow->exposeWindow();
      }

   } else if (m_exposedOnMoveToWindow) {
      m_exposedOnMoveToWindow = false;
      m_platformWindow->obscureWindow();
   }
}

- (void)viewWillMoveToWindow: (NSWindow *)newWindow
{
   // ### Merge "normal" window code path with this one
   if (! (m_window->type() & Qt::SubWindow)) {
      return;
   }

   if (newWindow) {
      [[NSNotificationCenter defaultCenter] addObserver: self
                                               selector: @selector(windowNotification:)
                                                   name: nil // Get all notifications
                                                 object: newWindow];
   }

   if ([self window]) {
      [[NSNotificationCenter defaultCenter] removeObserver: self name: nil object: [self window]];
   }
}

- (QWindow *)topLevelWindow
{
   QWindow *focusWindow = m_window;

   // For widgets we need to do a bit of trickery as the window
   // to activate is the window of the top-level widget.

   if (m_window->metaObject()->className() == "QWidgetWindow") {
      while (focusWindow->parent()) {
         focusWindow = focusWindow->parent();
      }
   }

   return focusWindow;
}

- (void)updateGeometry
{
   QRect geometry;

   if (m_platformWindow->m_isNSWindowChild) {
      return;

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
      // geometry = qt_mac_toQRect([self frame]);
      qDebug() << "nsview updateGeometry" << m_platformWindow->window();

      QRect screenRect = qt_mac_toQRect([m_platformWindow->m_nsWindow convertRectToScreen: [self frame]]);
      qDebug() << "screenRect" << screenRect;

      screenRect.moveTop(qt_mac_flipYCoordinate(screenRect.y() + screenRect.height()));
      geometry = QRect(m_platformWindow->window()->parent()->mapFromGlobal(screenRect.topLeft()), screenRect.size());
      qDebug() << "geometry" << geometry;
#endif

      // geometry = QRect(screenRect.origin.x, qt_mac_flipYCoordinate(screenRect.origin.y + screenRect.size.height),
      // screenRect.size.width, screenRect.size.height);

   } else if (m_platformWindow->m_nsWindow) {
      // top level window, get window rect and flip y.
      NSRect rect = [self frame];
      NSRect windowRect = [[self window] frame];
      geometry = QRect(windowRect.origin.x, qt_mac_flipYCoordinate(windowRect.origin.y + rect.size.height), rect.size.width,
            rect.size.height);

   } else if (m_platformWindow->m_contentViewIsToBeEmbedded) {
      // embedded child window, use the frame rect ### merge with case below
      geometry = qt_mac_toQRect([self bounds]);

   } else {
      // child window, use the frame rect
      geometry = qt_mac_toQRect([self frame]);
   }

   if (m_platformWindow->m_nsWindow && geometry == m_platformWindow->geometry()) {
      return;
   }

   const bool isResize = geometry.size() != m_platformWindow->geometry().size();

   // It can happen that self.window is nil (if we are changing
   // styleMask from/to borderless and content view is being re-parented)
   // - this results in an invalid coordinates.
   if (m_platformWindow->m_inSetStyleMask && !self.window) {
      return;
   }

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug() << "QNSView::udpateGeometry" << m_platformWindow << geometry;
#endif

   // Call setGeometry on QPlatformWindow. (not on QCocoaWindow,
   // doing that will initiate a geometry change it and possibly create
   // an infinite loop when this notification is triggered again.)
   m_platformWindow->QPlatformWindow::setGeometry(geometry);

   // Don't send the geometry change if the QWindow is designated to be
   // embedded in a foreign view hiearchy but has not actually been
   // embedded yet - it's too early.
   if (m_platformWindow->m_contentViewIsToBeEmbedded && !m_platformWindow->m_contentViewIsEmbedded) {
      return;
   }

   // Send a geometry change event to Qt, if it's ready to handle events
   if (! m_platformWindow->m_inConstructor) {
      QWindowSystemInterface::handleGeometryChange(m_window, geometry);
      m_platformWindow->updateExposedGeometry();

      // Guard against processing window system events during QWindow::setGeometry calls

      if (! m_platformWindow->m_inSetGeometry) {
         QWindowSystemInterface::flushWindowSystemEvents();
      } else if (isResize) {
         m_backingStore = nullptr;
      }
   }
}

- (void)notifyWindowStateChanged: (Qt::WindowState)newState
{
   // If the window was maximized, then fullscreen, then tried to go directly to "normal" state,
   // this notification will say that it is "normal", but it will still look maximized, and
   // if you called performZoom it would actually take it back to "normal".
   // So we should say that it is maximized because it actually is.
   if (newState == Qt::WindowNoState && m_platformWindow->m_effectivelyMaximized) {
      newState = Qt::WindowMaximized;
   }
   QWindowSystemInterface::handleWindowStateChanged(m_window, newState);
   // We want to read the window state back from the window,
   // but the event we just sent may be asynchronous.
   QWindowSystemInterface::flushWindowSystemEvents();
   m_platformWindow->setSynchedWindowStateFromWindow();
}

- (void)windowNotification: (NSNotification *) windowNotification
{
   NSString *notificationName = [windowNotification name];

   if (notificationName == NSWindowDidBecomeKeyNotification) {
      if (!m_platformWindow->windowIsPopupType() && ! m_isMenuView) {
         QWindowSystemInterface::handleWindowActivated(m_window);
      }

   } else if (notificationName == NSWindowDidResignKeyNotification) {
      // key window will be non-nil if another window became key... do not
      // set the active window to zero here, the new key window's
      // NSWindowDidBecomeKeyNotification hander will change the active window
      NSWindow *keyWindow = [NSApp keyWindow];
      if (!keyWindow) {
         // no new key window, go ahead and set the active window to zero
         if (!m_platformWindow->windowIsPopupType() && ! m_isMenuView) {
            QWindowSystemInterface::handleWindowActivated(nullptr);
         }
      }

   } else if (notificationName == NSWindowDidMiniaturizeNotification
      || notificationName == NSWindowDidDeminiaturizeNotification) {
      Qt::WindowState newState = notificationName == NSWindowDidMiniaturizeNotification ?
         Qt::WindowMinimized : Qt::WindowNoState;

      [self notifyWindowStateChanged: newState];

   } else if ([notificationName isEqualToString: @"NSWindowDidOrderOffScreenNotification"]) {
      m_platformWindow->obscureWindow();

   } else if ([notificationName isEqualToString: @"NSWindowDidOrderOnScreenAndFinishAnimatingNotification"]) {
      m_platformWindow->exposeWindow();

   } else if ([notificationName isEqualToString: NSWindowDidChangeOcclusionStateNotification]) {


      if ((NSUInteger)[self.window occlusionState] & NSWindowOcclusionStateVisible) {
         m_platformWindow->exposeWindow();

      } else {
         // Send Obscure events on window occlusion to stop animations.
         m_platformWindow->obscureWindow();
      }

   } else if (notificationName == NSWindowDidChangeScreenNotification) {
      if (m_window) {
         NSUInteger screenIndex = [[NSScreen screens] indexOfObject: self.window.screen];

         if (screenIndex != NSNotFound) {
            QCocoaScreen *cocoaScreen = QCocoaIntegration::instance()->screenAtIndex(screenIndex);

            if (cocoaScreen) {
               QWindowSystemInterface::handleWindowScreenChanged(m_window, cocoaScreen->screen());
            }

            m_platformWindow->updateExposedGeometry();
         }
      }

   } else if (notificationName == NSWindowDidEnterFullScreenNotification
      || notificationName == NSWindowDidExitFullScreenNotification) {
      Qt::WindowState newState = notificationName == NSWindowDidEnterFullScreenNotification ? Qt::WindowFullScreen : Qt::WindowNoState;
      [self notifyWindowStateChanged: newState];
   }
}

- (void)textInputContextKeyboardSelectionDidChangeNotification: (NSNotification *)
   textInputContextKeyboardSelectionDidChangeNotification
{
   if (([NSApp keyWindow] == [self window]) && [[self window] firstResponder] == self) {
      QCocoaInputContext *ic = qobject_cast<QCocoaInputContext *>(QCocoaIntegration::instance()->inputContext());
      ic->updateLocale();
   }
}

- (void)notifyWindowWillZoom: (BOOL)willZoom
{
   Qt::WindowState newState = willZoom ? Qt::WindowMaximized : Qt::WindowNoState;

   if (! willZoom) {
      m_platformWindow->m_effectivelyMaximized = false;
   }

   [self notifyWindowStateChanged: newState];
}

- (void)viewDidHide
{
   m_platformWindow->obscureWindow();
}

- (void)viewDidUnhide
{
   m_platformWindow->exposeWindow();
}

- (void)removeFromSuperview
{
   QMacAutoReleasePool pool;
   [super removeFromSuperview];
}

- (void) flushBackingStore: (QCocoaBackingStore *)backingStore region: (const QRegion &)region offset: (QPoint)offset
{
   m_backingStore = backingStore;
   m_backingStoreOffset = offset * m_backingStore->getBackingStoreDevicePixelRatio();

   for (QRect rect : region.rects()) {
      [self setNeedsDisplayInRect: NSMakeRect(rect.x(), rect.y(), rect.width(), rect.height())];
   }
}

- (void)clearBackingStore: (QCocoaBackingStore *)backingStore
{
   if (backingStore == m_backingStore) {
      m_backingStore = nullptr;
   }
}

- (BOOL) hasMask
{
   return m_maskImage != nullptr;
}

- (BOOL) isOpaque
{
   if (!m_platformWindow) {
      return true;
   }
   return m_platformWindow->isOpaque();
}

- (void) setMaskRegion: (const QRegion *)region
{
   m_shouldInvalidateWindowShadow = true;
   if (m_maskImage) {
      CGImageRelease(m_maskImage);
   }
   if (region->isEmpty()) {
      m_maskImage = nullptr;
      return;
   }

   const QRect &rect = region->boundingRect();
   QImage tmp(rect.size(), QImage::Format_RGB32);
   tmp.fill(Qt::white);

   QPainter p(&tmp);
   p.setClipRegion(*region);
   p.fillRect(rect, Qt::black);
   p.end();
   QImage maskImage = QImage(rect.size(), QImage::Format_Indexed8);

   for (int y = 0; y < rect.height(); ++y) {
      const uint *src = (const uint *) tmp.constScanLine(y);
      uchar *dst = maskImage.scanLine(y);
      for (int x = 0; x < rect.width(); ++x) {
         dst[x] = src[x] & 0xff;
      }
   }
   m_maskImage = qt_mac_toCGImageMask(maskImage);
}

- (void)invalidateWindowShadowIfNeeded
{
   if (m_shouldInvalidateWindowShadow && m_platformWindow->m_nsWindow) {
      [m_platformWindow->m_nsWindow invalidateShadow];
      m_shouldInvalidateWindowShadow = false;
   }
}

- (void) drawRect: (NSRect)dirtyRect
{
#ifndef QT_NO_OPENGL
   if (m_glContext && m_shouldSetGLContextinDrawRect) {
      [m_glContext->nsOpenGLContext() setView: self];
      m_shouldSetGLContextinDrawRect = false;
   }
#endif

   if (m_platformWindow->m_drawContentBorderGradient) {
      NSDrawWindowBackground(dirtyRect);
   }

   if (! m_backingStore) {
      return;
   }

   // Calculate source and target rects. The target rect is the dirtyRect:
   CGRect dirtyWindowRect = NSRectToCGRect(dirtyRect);

   // The backing store source rect will be larger on retina displays.
   // Scale dirtyRect by the device pixel ratio:
   const qreal devicePixelRatio = m_backingStore->getBackingStoreDevicePixelRatio();
   CGRect dirtyBackingRect = CGRectMake(dirtyRect.origin.x * devicePixelRatio,
         dirtyRect.origin.y * devicePixelRatio,
         dirtyRect.size.width * devicePixelRatio,
         dirtyRect.size.height * devicePixelRatio);

   NSGraphicsContext *nsGraphicsContext = [NSGraphicsContext currentContext];
   CGContextRef cgContext = (CGContextRef) [nsGraphicsContext graphicsPort];

   // Translate coordiate system from CoreGraphics (bottom-left) to NSView (top-left):
   CGContextSaveGState(cgContext);
   int dy = dirtyWindowRect.origin.y + CGRectGetMaxY(dirtyWindowRect);

   CGContextTranslateCTM(cgContext, 0, dy);
   CGContextScaleCTM(cgContext, 1, -1);

   // If a mask is set, modify the sub image accordingly:
   CGImageRef subMask = nullptr;
   if (m_maskImage) {
      subMask = CGImageCreateWithImageInRect(m_maskImage, dirtyWindowRect);
      CGContextClipToMask(cgContext, dirtyWindowRect, subMask);
   }

   // Clip out and draw the correct sub image from the (shared) backingstore:
   CGRect backingStoreRect = CGRectMake(
         dirtyBackingRect.origin.x + m_backingStoreOffset.x(),
         dirtyBackingRect.origin.y + m_backingStoreOffset.y(),
         dirtyBackingRect.size.width,
         dirtyBackingRect.size.height
      );
   CGImageRef bsCGImage = qt_mac_toCGImage(m_backingStore->toImage());
   CGImageRef cleanImg = CGImageCreateWithImageInRect(bsCGImage, backingStoreRect);

   // Optimization: Copy frame buffer content instead of blending for
   // top-level windows where CS fills the entire window content area.
   // (But don't overpaint the title-bar gradient)

   if (m_platformWindow->m_nsWindow && !m_platformWindow->m_drawContentBorderGradient) {
      CGContextSetBlendMode(cgContext, kCGBlendModeCopy);
   }

   CGContextDrawImage(cgContext, dirtyWindowRect, cleanImg);

   // Clean-up:
   CGContextRestoreGState(cgContext);
   CGImageRelease(cleanImg);
   CGImageRelease(subMask);
   CGImageRelease(bsCGImage);

   [self invalidateWindowShadowIfNeeded];
}

- (BOOL) isFlipped
{
   return YES;
}

- (BOOL)becomeFirstResponder
{
   if (!m_window || !m_platformWindow) {
      return NO;
   }

   if (m_window->flags() & Qt::WindowTransparentForInput) {
      return NO;
   }

   if (!m_platformWindow->windowIsPopupType() && !m_isMenuView) {
      QWindowSystemInterface::handleWindowActivated([self topLevelWindow]);
   }

   return YES;
}

- (BOOL)acceptsFirstResponder
{
   if (!m_window || !m_platformWindow) {
      return NO;
   }

   if (m_isMenuView) {
      return NO;
   }

   if (m_platformWindow->shouldRefuseKeyWindowAndFirstResponder()) {
      return NO;
   }

   if (m_window->flags() & Qt::WindowTransparentForInput) {
      return NO;
   }
   if ((m_window->flags() & Qt::ToolTip) == Qt::ToolTip) {
      return NO;
   }
   return YES;
}

- (BOOL)acceptsFirstMouse: (NSEvent *)theEvent
{
   (void) theEvent;

   if (!m_window || !m_platformWindow) {
      return NO;
   }

   if (m_window->flags() & Qt::WindowTransparentForInput) {
      return NO;
   }

   return YES;
}

- (void)convertFromScreen: (NSPoint)mouseLocation toWindowPoint: (QPointF *)qtWindowPoint andScreenPoint: (QPointF *)qtScreenPoint
{
   // Calculate the mouse position in the QWindow and CS screen coordinate system,
   // starting from coordinates in the NSWindow coordinate system.
   //
   // This involves translating according to the window location on screen,
   // as well as inverting the y coordinate due to the origin change.
   //
   // Coordinate system overview, outer to innermost:
   //
   // Name             Origin
   //
   // OS X screen      bottom-left
   // CS screen        top-left
   // NSWindow         bottom-left
   // NSView/QWindow   top-left
   //
   // NSView and QWindow are equal coordinate systems: the QWindow covers the
   // entire NSView, and we've set the NSView's isFlipped property to true.

   NSWindow *window = [self window];
   NSPoint nsWindowPoint;
   NSRect windowRect = [window convertRectFromScreen: NSMakeRect(mouseLocation.x, mouseLocation.y, 1, 1)];
   nsWindowPoint = windowRect.origin;                                          // NSWindow coordinates
   NSPoint nsViewPoint = [self convertPoint: nsWindowPoint fromView: nil];     // NSView/QWindow coordinates
   *qtWindowPoint = QPointF(nsViewPoint.x, nsViewPoint.y);                     // NSView/QWindow coordinates

   *qtScreenPoint = QPointF(mouseLocation.x, qt_mac_flipYCoordinate(mouseLocation.y)); // CS screen coordinates
}

- (void)resetMouseButtons
{
   m_buttons = Qt::NoButton;
   m_frameStrutButtons = Qt::NoButton;
}

- (NSPoint) screenMousePoint: (NSEvent *)theEvent
{
   NSPoint screenPoint;
   if (theEvent) {
      NSPoint windowPoint = [theEvent locationInWindow];
      NSRect screenRect = [[theEvent window] convertRectToScreen: NSMakeRect(windowPoint.x, windowPoint.y, 1, 1)];
      screenPoint = screenRect.origin;
   } else {
      screenPoint = [NSEvent mouseLocation];
   }
   return screenPoint;
}

- (void)handleMouseEvent: (NSEvent *)theEvent
{
   bool isTabletEvent = [self handleTabletEvent: theEvent];

   QPointF qtWindowPoint;
   QPointF qtScreenPoint;
   QNSView *targetView = self;

   if (m_platformWindow && m_platformWindow->m_forwardWindow) {
      if (theEvent.type == NSEventTypeLeftMouseDragged || theEvent.type == NSEventTypeLeftMouseUp) {
         targetView = m_platformWindow->m_forwardWindow->m_qtView;
      } else {
         m_platformWindow->m_forwardWindow.clear();
      }
   }

   // Popups implicitly grap mouse events; forward to the active popup if there is one
   if (QCocoaWindow *popup = QCocoaIntegration::instance()->activePopupWindow()) {
      // Tooltips must be transparent for mouse events
      // The bug reference is QTBUG-46379
      if (! popup->m_windowFlags.testFlag(Qt::ToolTip)) {
         if (QNSView *popupView = popup->qtView()) {
            targetView = popupView;
         }
      }
   }

   [targetView convertFromScreen: [self screenMousePoint: theEvent] toWindowPoint: &qtWindowPoint andScreenPoint: &qtScreenPoint];
   ulong timestamp = [theEvent timestamp] * 1000;

   QCocoaDrag *nativeDrag = QCocoaIntegration::instance()->drag();
   nativeDrag->setLastMouseEvent(theEvent, self);

   Qt::KeyboardModifiers keyboardModifiers = [QNSView convertKeyModifiers: [theEvent modifierFlags]];
   QWindowSystemInterface::handleMouseEvent(targetView->m_window, timestamp, qtWindowPoint, qtScreenPoint, m_buttons, keyboardModifiers,
      isTabletEvent ? Qt::MouseEventSynthesizedByCS : Qt::MouseEventNotSynthesized);
}

- (void)handleFrameStrutMouseEvent: (NSEvent *)theEvent
{
   // get m_buttons in sync
   // Don't send frme strut events if we are in the middle of a mouse drag.
   if (m_buttons != Qt::NoButton) {
      return;
   }

   NSEventType ty = [theEvent type];
   switch (ty) {
      case NSEventTypeLeftMouseDown:
         m_frameStrutButtons |= Qt::LeftButton;
         break;
      case NSEventTypeLeftMouseUp:
         m_frameStrutButtons &= ~Qt::LeftButton;
         break;
      case NSEventTypeRightMouseDown:
         m_frameStrutButtons |= Qt::RightButton;
         break;
      case NSEventTypeLeftMouseDragged:
         m_frameStrutButtons |= Qt::LeftButton;
         break;
      case NSEventTypeRightMouseDragged:
         m_frameStrutButtons |= Qt::RightButton;
         break;
      case NSEventTypeRightMouseUp:
         m_frameStrutButtons &= ~Qt::RightButton;
         break;
      case NSEventTypeOtherMouseDown:
         m_frameStrutButtons |= cocoaButton2QtButton([theEvent buttonNumber]);
         break;
      case NSEventTypeOtherMouseUp:
         m_frameStrutButtons &= ~cocoaButton2QtButton([theEvent buttonNumber]);
      default:
         break;
   }

   NSWindow *window = [self window];
   NSPoint windowPoint = [theEvent locationInWindow];

   int windowScreenY = [window frame].origin.y + [window frame].size.height;
   NSPoint windowCoord = [self convertPoint: [self frame].origin toView: nil];
   int viewScreenY = [window convertRectToScreen: NSMakeRect(windowCoord.x, windowCoord.y, 0, 0)].origin.y;
   int titleBarHeight = windowScreenY - viewScreenY;

   NSPoint nsViewPoint = [self convertPoint: windowPoint fromView: nil];
   QPoint qtWindowPoint = QPoint(nsViewPoint.x, titleBarHeight + nsViewPoint.y);
   NSPoint screenPoint = [window convertRectToScreen: NSMakeRect(windowPoint.x, windowPoint.y, 0, 0)].origin;
   QPoint qtScreenPoint = QPoint(screenPoint.x, qt_mac_flipYCoordinate(screenPoint.y));

   ulong timestamp = [theEvent timestamp] * 1000;
   QWindowSystemInterface::handleFrameStrutMouseEvent(m_window, timestamp, qtWindowPoint, qtScreenPoint, m_frameStrutButtons);
}

- (void)mouseDown: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super mouseDown: theEvent];
   }
   m_sendUpAsRightButton = false;

   // Handle any active poup windows; clicking outisde them should close them
   // all. Do not do anything or clicks inside one of the menus, let Cocoa
   // handle that case. Note that in practice many windows of the Qt::Popup type
   // will actually close themselves in this case using logic implemented in
   // that particular poup type (for example context menus). However, CS expects
   // that plain popup QWindows will also be closed, so we implement the logic here as well.

   QList<QCocoaWindow *> *popups = QCocoaIntegration::instance()->popupWindowStack();
   if (!popups->isEmpty()) {
      // Check if the click is outside all popups.
      bool inside = false;
      QPointF qtScreenPoint = qt_mac_flipPoint([self screenMousePoint: theEvent]);
      for (QList<QCocoaWindow *>::const_iterator it = popups->begin(); it != popups->end(); ++it) {
         if ((*it)->geometry().contains(qtScreenPoint.toPoint())) {
            inside = true;
            break;
         }
      }
      // Close the popups if the click was outside.
      if (!inside) {
         Qt::WindowType type = QCocoaIntegration::instance()->activePopupWindow()->window()->type();
         while (QCocoaWindow *popup = QCocoaIntegration::instance()->popPopupWindow()) {
            QWindowSystemInterface::handleCloseEvent(popup->window());
            QWindowSystemInterface::flushWindowSystemEvents();
         }
         // Consume the mouse event when closing the popup, except for tool tips
         // were it's expected that the event is processed normally.
         if (type != Qt::ToolTip) {
            return;
         }
      }
   }

   if ([self hasMarkedText]) {
      [[NSTextInputContext currentInputContext] handleEvent: theEvent];
   } else {
      if (!_q_dontOverrideCtrlLMB && [QNSView convertKeyModifiers: [theEvent modifierFlags]] & Qt::MetaModifier) {
         m_buttons |= Qt::RightButton;
         m_sendUpAsRightButton = true;
      } else {
         m_buttons |= Qt::LeftButton;
      }
      [self handleMouseEvent: theEvent];
   }
}

- (void)mouseDragged: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super mouseDragged: theEvent];
   }

   if (! (m_buttons & (m_sendUpAsRightButton ? Qt::RightButton : Qt::LeftButton))) {

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
      qDebug("QNSView mouseDragged: Internal mouse button tracking invalid (missing Qt::LeftButton)");
#endif

   }

   [self handleMouseEvent: theEvent];
}

- (void)mouseUp: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super mouseUp: theEvent];
   }
   if (m_sendUpAsRightButton) {
      m_buttons &= ~Qt::RightButton;
      m_sendUpAsRightButton = false;
   } else {
      m_buttons &= ~Qt::LeftButton;
   }
   [self handleMouseEvent: theEvent];
}

- (void)updateTrackingAreas
{
   [super updateTrackingAreas];

   QMacAutoReleasePool pool;

   // NSTrackingInVisibleRect keeps care of updating once the tracking is set up, so bail out early
   if (m_trackingArea && [[self trackingAreas] containsObject: m_trackingArea]) {
      return;
   }

   // Ideally, we shouldn't have NSTrackingMouseMoved events included below, it should
   // only be turned on if mouseTracking, hover is on or a tool tip is set.
   // Unfortunately, CS will send "tooltip" events on mouse moves, so we need to
   // turn it on in ALL case. That means EVERY QWindow gets to pay the cost of
   // mouse moves delivered to it (Apple recommends keeping it OFF because there
   // is a performance hit). So it goes.

   NSUInteger trackingOptions = NSTrackingMouseEnteredAndExited | NSTrackingActiveInActiveApp
      | NSTrackingInVisibleRect | NSTrackingMouseMoved | NSTrackingCursorUpdate;
   [m_trackingArea release];
   m_trackingArea = [[NSTrackingArea alloc] initWithRect: [self frame]
                                                 options: trackingOptions
                                                   owner: m_mouseMoveHelper
                                                userInfo: nil];
   [self addTrackingArea: m_trackingArea];
}

- (void)cursorUpdate: (NSEvent *)theEvent
{
   (void) theEvent;
   m_platformWindow->applyEffectiveWindowCursor();
}

- (void)mouseMovedImpl: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return;
   }

   QPointF windowPoint;
   QPointF screenPoint;
   [self convertFromScreen: [self screenMousePoint: theEvent] toWindowPoint: &windowPoint andScreenPoint: &screenPoint];
   QWindow *childWindow = m_platformWindow->childWindowAt(windowPoint.toPoint());

   // Top-level windows generate enter-leave events for sub-windows.
   // CS wants to know which window (if any) will be entered at the
   // the time of the leave. This is dificult to accomplish by
   // handling mouseEnter and mouseLeave envents, since they are sent
   // individually to different views.
   if (m_platformWindow->m_nsWindow && childWindow) {
      if (childWindow != m_platformWindow->m_enterLeaveTargetWindow) {
         QWindowSystemInterface::handleEnterLeaveEvent(childWindow, m_platformWindow->m_enterLeaveTargetWindow, windowPoint, screenPoint);
         m_platformWindow->m_enterLeaveTargetWindow = childWindow;
      }
   }

   // Cocoa keeps firing mouse move events for obscured parent views. CS should not
   // send those events so filter them out here.
   if (childWindow != m_window) {
      return;
   }

   [self handleMouseEvent: theEvent];
}

- (void)mouseEnteredImpl: (NSEvent *)theEvent
{
   (void) theEvent;
   m_platformWindow->m_windowUnderMouse = true;

   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return;
   }

   // Top-level windows generate enter events for sub-windows.
   if (!m_platformWindow->m_nsWindow) {
      return;
   }

   QPointF windowPoint;
   QPointF screenPoint;
   [self convertFromScreen: [NSEvent mouseLocation] toWindowPoint: &windowPoint andScreenPoint: &screenPoint];
   m_platformWindow->m_enterLeaveTargetWindow = m_platformWindow->childWindowAt(windowPoint.toPoint());
   QWindowSystemInterface::handleEnterEvent(m_platformWindow->m_enterLeaveTargetWindow, windowPoint, screenPoint);
}

- (void)mouseExitedImpl: (NSEvent *)theEvent
{
   (void) theEvent;
   m_platformWindow->m_windowUnderMouse = false;

   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return;
   }

   // Top-level windows generate leave events for sub-windows.
   if (!m_platformWindow->m_nsWindow) {
      return;
   }

   QWindowSystemInterface::handleLeaveEvent(m_platformWindow->m_enterLeaveTargetWindow);
   m_platformWindow->m_enterLeaveTargetWindow = nullptr;
}

- (void)rightMouseDown: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super rightMouseDown: theEvent];
   }

   m_buttons |= Qt::RightButton;
   m_sendUpAsRightButton = true;
   [self handleMouseEvent: theEvent];
}

- (void)rightMouseDragged: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super rightMouseDragged: theEvent];
   }

   if (! (m_buttons & Qt::RightButton)) {
#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
      qDebug("QNSView rightMouseDragged: Internal mouse button tracking invalid (missing Qt::RightButton)");
#endif
   }

   [self handleMouseEvent: theEvent];
}

- (void)rightMouseUp: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super rightMouseUp: theEvent];
   }

   m_buttons &= ~Qt::RightButton;
   m_sendUpAsRightButton = false;
   [self handleMouseEvent: theEvent];
}

- (void)otherMouseDown: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super otherMouseDown: theEvent];
   }

   m_buttons |= cocoaButton2QtButton([theEvent buttonNumber]);
   [self handleMouseEvent: theEvent];
}

- (void)otherMouseDragged: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super otherMouseDragged: theEvent];
   }

   if (!(m_buttons & ~(Qt::LeftButton | Qt::RightButton))) {
#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
      qDebug("QNSView otherMouseDragged: Internal mouse button tracking invalid (missing Qt::MiddleButton or Qt::ExtraButton*)");
#endif
   }
   [self handleMouseEvent: theEvent];
}

- (void)otherMouseUp: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super otherMouseUp: theEvent];
   }
   m_buttons &= ~cocoaButton2QtButton([theEvent buttonNumber]);
   [self handleMouseEvent: theEvent];
}

struct QCocoaTabletDeviceData {
   QTabletEvent::TabletDevice device;
   QTabletEvent::PointerType pointerType;
   uint capabilityMask;
   qint64 uid;
};

using QCocoaTabletDeviceDataHash = QHash<uint, QCocoaTabletDeviceData>;

QCocoaTabletDeviceDataHash &tabletDeviceDataHash()
{
   static QCocoaTabletDeviceDataHash retval;
   return retval;
}

- (bool)handleTabletEvent: (NSEvent *)theEvent
{
   NSEventType eventType = [theEvent type];

   if (eventType != NSEventTypeTabletPoint && [theEvent subtype] != NSEventSubtypeTabletPoint) {
      return false;   // Not a tablet event
   }

   ulong timestamp = [theEvent timestamp] * 1000;

   QPointF windowPoint;
   QPointF screenPoint;
   [self convertFromScreen: [NSEvent mouseLocation] toWindowPoint: &windowPoint andScreenPoint: &screenPoint];

   uint deviceId = [theEvent deviceID];
   if (! tabletDeviceDataHash().contains(deviceId)) {
      // Error: Unknown tablet device. also gets into this state when running on a VM.
      // This appears to be harmless so do not print a warning
      return false;
   }

   const QCocoaTabletDeviceData &deviceData = tabletDeviceDataHash().value(deviceId);

   bool down = (eventType != NSEventTypeMouseMoved);

   qreal pressure;
   if (down) {
      pressure = [theEvent pressure];
   } else {
      pressure = 0.0;
   }

   NSPoint tilt = [theEvent tilt];
   int xTilt = qRound(tilt.x * 60.0);
   int yTilt = qRound(tilt.y * -60.0);
   Qt::MouseButtons buttons = static_cast<Qt::MouseButtons>(static_cast<uint>([theEvent buttonMask]));
   qreal tangentialPressure = 0;
   qreal rotation           = 0;
   int z = 0;

   if (deviceData.capabilityMask & 0x0200) {
      z = [theEvent absoluteZ];
   }

   if (deviceData.capabilityMask & 0x0800) {
      tangentialPressure = ([theEvent tangentialPressure] * 2.0) - 1.0;
   }

   rotation = 360.0 - [theEvent rotation];
   if (rotation > 180.0) {
      rotation -= 360.0;
   }

   Qt::KeyboardModifiers keyboardModifiers = [QNSView convertKeyModifiers: [theEvent modifierFlags]];

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug("Event on tablet %d with tool %d type %d unique ID %lld pos %6.1f, %6.1f root pos %6.1f, "
         " %6.1f buttons 0x%x pressure %4.2lf tilt %d, %d rotation %6.2lf",
         deviceId, deviceData.device, deviceData.pointerType, deviceData.uid,
         windowPoint.x(), windowPoint.y(), screenPoint.x(), screenPoint.y(),
         static_cast<uint>(buttons), pressure, xTilt, yTilt, rotation);
#endif

   QWindowSystemInterface::handleTabletEvent(m_window, timestamp, windowPoint, screenPoint,
      deviceData.device, deviceData.pointerType, buttons, pressure, xTilt, yTilt,
      tangentialPressure, rotation, z, deviceData.uid,
      keyboardModifiers);

   return true;
}

- (void)tabletPoint: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super tabletPoint: theEvent];
   }

   [self handleTabletEvent: theEvent];
}

static QTabletEvent::TabletDevice wacomTabletDevice(NSEvent *theEvent)
{
   qint64 uid = [theEvent uniqueID];
   uint bits = [theEvent vendorPointingDeviceType];
   if (bits == 0 && uid != 0) {
      // Fallback. It seems that the driver doesn't always include all the information.
      // High-End Wacom devices store their "type" in the uper bits of the Unique ID.
      // I'm not sure how to handle it for consumer devices, but I'll test that in a bit.
      bits = uid >> 32;
   }

   QTabletEvent::TabletDevice device;
   // Defined in the "EN0056-NxtGenImpGuideX"
   // on Wacom's Developer Website (www.wacomeng.com)
   if (((bits & 0x0006) == 0x0002) && ((bits & 0x0F06) != 0x0902)) {
      device = QTabletEvent::Stylus;
   } else {
      switch (bits & 0x0F06) {
         case 0x0802:
            device = QTabletEvent::Stylus;
            break;
         case 0x0902:
            device = QTabletEvent::Airbrush;
            break;
         case 0x0004:
            device = QTabletEvent::FourDMouse;
            break;
         case 0x0006:
            device = QTabletEvent::Puck;
            break;
         case 0x0804:
            device = QTabletEvent::RotationStylus;
            break;
         default:
            device = QTabletEvent::NoDevice;
      }
   }
   return device;
}

- (void)tabletProximity: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super tabletProximity: theEvent];
   }

   ulong timestamp = [theEvent timestamp] * 1000;

   QCocoaTabletDeviceData deviceData;
   deviceData.uid = [theEvent uniqueID];
   deviceData.capabilityMask = [theEvent capabilityMask];

   switch ([theEvent pointingDeviceType]) {
      case NSPointingDeviceTypeUnknown:
      default:
         deviceData.pointerType = QTabletEvent::UnknownPointer;
         break;

      case NSPointingDeviceTypePen:
         deviceData.pointerType = QTabletEvent::Pen;
         break;

      case NSPointingDeviceTypeCursor:
         deviceData.pointerType = QTabletEvent::Cursor;
         break;

      case NSPointingDeviceTypeEraser:
         deviceData.pointerType = QTabletEvent::Eraser;
         break;
   }

   deviceData.device = wacomTabletDevice(theEvent);

   // The deviceID is "unique" while in the proximity, it's a key that we can use for
   // linking up QCocoaTabletDeviceData to an event (especially if there are two devices in action).
   bool entering = [theEvent isEnteringProximity];
   uint deviceId = [theEvent deviceID];

   if (entering) {
      tabletDeviceDataHash().insert(deviceId, deviceData);
   } else {
      tabletDeviceDataHash().remove(deviceId);
   }

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug("Proximity change on tablet %d: current tool %d type %d unique ID %lld",
      deviceId, deviceData.device, deviceData.pointerType, deviceData.uid);
#endif

   if (entering) {
      QWindowSystemInterface::handleTabletEnterProximityEvent(timestamp, deviceData.device, deviceData.pointerType, deviceData.uid);
   } else {
      QWindowSystemInterface::handleTabletLeaveProximityEvent(timestamp, deviceData.device, deviceData.pointerType, deviceData.uid);
   }
}

- (bool) shouldSendSingleTouch
{
   // QWidgets expects single-point touch events, QtDeclarative does not.
   // Until there is an API we solve this by looking at the window class type.
   return m_window->inherits("QWidgetWindow");
}

- (void)touchesBeganWithEvent: (NSEvent *)event
{
   const NSTimeInterval timestamp = [event timestamp];
   const QList<QWindowSystemInterface::TouchPoint> points = QCocoaTouch::getCurrentTouchPointList(
         event, [self shouldSendSingleTouch]);

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug() << "touchesBeganWithEvent" << points;
#endif

   QWindowSystemInterface::handleTouchEvent(m_window, timestamp * 1000, touchDevice, points);
}

- (void)touchesMovedWithEvent: (NSEvent *)event
{
   const NSTimeInterval timestamp = [event timestamp];
   const QList<QWindowSystemInterface::TouchPoint> points = QCocoaTouch::getCurrentTouchPointList(
         event, [self shouldSendSingleTouch]);

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug() << "touchesMovedWithEvent" << points;
#endif

   QWindowSystemInterface::handleTouchEvent(m_window, timestamp * 1000, touchDevice, points);
}

- (void)touchesEndedWithEvent: (NSEvent *)event
{
   const NSTimeInterval timestamp = [event timestamp];
   const QList<QWindowSystemInterface::TouchPoint> points = QCocoaTouch::getCurrentTouchPointList(
         event, [self shouldSendSingleTouch]);

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug() << "touchesEndedWithEvent" << points;
#endif

   QWindowSystemInterface::handleTouchEvent(m_window, timestamp * 1000, touchDevice, points);
}

- (void)touchesCancelledWithEvent: (NSEvent *)event
{
   const NSTimeInterval timestamp = [event timestamp];
   const QList<QWindowSystemInterface::TouchPoint> points = QCocoaTouch::getCurrentTouchPointList(
         event, [self shouldSendSingleTouch]);

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug() << "touchesCancelledWithEvent" << points;
#endif

   QWindowSystemInterface::handleTouchEvent(m_window, timestamp * 1000, touchDevice, points);
}

#ifndef QT_NO_GESTURES

- (bool)handleGestureAsBeginEnd: (NSEvent *)event
{
   if (QSysInfo::QSysInfo::MacintoshVersion < QSysInfo::MV_10_11) {
      return false;
   }

   if ([event phase] == NSEventPhaseBegan) {
      [self beginGestureWithEvent: event];
      return true;
   }

   if ([event phase] == NSEventPhaseEnded) {
      [self endGestureWithEvent: event];
      return true;
   }

   return false;
}
- (void)magnifyWithEvent: (NSEvent *)event
{
   if ([self handleGestureAsBeginEnd: event]) {
      return;
   }

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug() << "magnifyWithEvent" << [event magnification];
#endif

   const NSTimeInterval timestamp = [event timestamp];
   QPointF windowPoint;
   QPointF screenPoint;
   [self convertFromScreen: [NSEvent mouseLocation] toWindowPoint: &windowPoint andScreenPoint: &screenPoint];
   QWindowSystemInterface::handleGestureEventWithRealValue(m_window, timestamp, Qt::ZoomNativeGesture,
      [event magnification], windowPoint, screenPoint);
}

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_8
- (void)smartMagnifyWithEvent: (NSEvent *)event
{
   static bool zoomIn = true;

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug() << "smartMagnifyWithEvent" << zoomIn;
#endif

   const NSTimeInterval timestamp = [event timestamp];
   QPointF windowPoint;
   QPointF screenPoint;
   [self convertFromScreen: [NSEvent mouseLocation] toWindowPoint: &windowPoint andScreenPoint: &screenPoint];

   QWindowSystemInterface::handleGestureEventWithRealValue(m_window, timestamp, Qt::SmartZoomNativeGesture,
      zoomIn ? 1.0f : 0.0f, windowPoint, screenPoint);
   zoomIn = !zoomIn;
}
#endif

- (void)rotateWithEvent: (NSEvent *)event
{
   if ([self handleGestureAsBeginEnd: event]) {
      return;
   }

   const NSTimeInterval timestamp = [event timestamp];
   QPointF windowPoint;
   QPointF screenPoint;

   [self convertFromScreen: [NSEvent mouseLocation] toWindowPoint: &windowPoint andScreenPoint: &screenPoint];

   QWindowSystemInterface::handleGestureEventWithRealValue(m_window, timestamp, Qt::RotateNativeGesture,
      -[event rotation], windowPoint, screenPoint);
}

- (void)swipeWithEvent: (NSEvent *)event
{
#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug() << "swipeWithEvent" << [event deltaX] << [event deltaY];
#endif

   const NSTimeInterval timestamp = [event timestamp];
   QPointF windowPoint;
   QPointF screenPoint;
   [self convertFromScreen: [NSEvent mouseLocation] toWindowPoint: &windowPoint andScreenPoint: &screenPoint];

   qreal angle = 0.0f;
   if ([event deltaX] == 1) {
      angle = 180.0f;
   } else if ([event deltaX] == -1) {
      angle = 0.0f;
   } else if ([event deltaY] == 1) {
      angle = 90.0f;
   } else if ([event deltaY] == -1) {
      angle = 270.0f;
   }

   QWindowSystemInterface::handleGestureEventWithRealValue(m_window, timestamp, Qt::SwipeNativeGesture,
      angle, windowPoint, screenPoint);
}

- (void)beginGestureWithEvent: (NSEvent *)event
{
   const NSTimeInterval timestamp = [event timestamp];
   QPointF windowPoint;
   QPointF screenPoint;
   [self convertFromScreen: [NSEvent mouseLocation] toWindowPoint: &windowPoint andScreenPoint: &screenPoint];

#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug() << "beginGestureWithEvent @" << windowPoint;
#endif

   QWindowSystemInterface::handleGestureEvent(m_window, timestamp, Qt::BeginNativeGesture,
      windowPoint, screenPoint);
}

- (void)endGestureWithEvent: (NSEvent *)event
{
#if defined(CS_SHOW_DEBUG_PLATFORM_WINDOW)
   qDebug() << "endGestureWithEvent";
#endif

   const NSTimeInterval timestamp = [event timestamp];
   QPointF windowPoint;
   QPointF screenPoint;
   [self convertFromScreen: [NSEvent mouseLocation] toWindowPoint: &windowPoint andScreenPoint: &screenPoint];
   QWindowSystemInterface::handleGestureEvent(m_window, timestamp, Qt::EndNativeGesture, windowPoint, screenPoint);
}
#endif // QT_NO_GESTURES

#ifndef QT_NO_WHEELEVENT
- (void)scrollWheel: (NSEvent *)theEvent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super scrollWheel: theEvent];
   }

   QPoint angleDelta;
   Qt::MouseEventSource source = Qt::MouseEventNotSynthesized;

   if ([theEvent hasPreciseScrollingDeltas]) {
      // The mouse device contains pixel scroll wheel support (Mighty Mouse, Trackpad).
      // Since deviceDelta is delivered as pixels rather than degrees, we need to
      // convert from pixels to degrees in a sensible manner.
      // It looks like 1/4 degrees per pixel behaves most native.
      // (NB: expects the unit for delta to be 8 per degree):

      const int pixelsToDegrees = 2; // 8 * 1/4
      angleDelta.setX([theEvent scrollingDeltaX] * pixelsToDegrees);
      angleDelta.setY([theEvent scrollingDeltaY] * pixelsToDegrees);
      source = Qt::MouseEventSynthesizedBySystem;

   } else {
      // Remove acceleration, and use either -120 or 120 as delta:
      angleDelta.setX(qBound(-120, int([theEvent deltaX] * 10000), 120));
      angleDelta.setY(qBound(-120, int([theEvent deltaY] * 10000), 120));
   }

   QPoint pixelDelta;
   if ([theEvent hasPreciseScrollingDeltas]) {
      pixelDelta.setX([theEvent scrollingDeltaX]);
      pixelDelta.setY([theEvent scrollingDeltaY]);
   } else {
      // docs: "In the case of !hasPreciseScrollingDeltas, multiply the delta with the line width."
      // scrollingDeltaX seems to return a minimum value of 0.1 in this case, map that to two pixels.
      const CGFloat lineWithEstimate = 20.0;
      pixelDelta.setX([theEvent scrollingDeltaX] * lineWithEstimate);
      pixelDelta.setY([theEvent scrollingDeltaY] * lineWithEstimate);
   }

   QPointF qt_windowPoint;
   QPointF qt_screenPoint;
   [self convertFromScreen: [NSEvent mouseLocation] toWindowPoint: &qt_windowPoint andScreenPoint: &qt_screenPoint];
   NSTimeInterval timestamp = [theEvent timestamp];
   ulong qt_timestamp = timestamp * 1000;

   // Prevent keyboard modifier state from changing during scroll event streams.
   // A two-finger trackpad flick generates a stream of scroll events. We want
   // the keyboard modifier state to be the state at the beginning of the
   // flick in order to avoid changing the interpretation of the events
   // mid-stream. One example of this happening would be when pressing cmd
   // after scrolling in QtCreator: not taking the phase into account causes
   // the end of the event stream to be interpreted as font size changes.
   NSEventPhase momentumPhase = [theEvent momentumPhase];
   if (momentumPhase == NSEventPhaseNone) {
      currentWheelModifiers = [QNSView convertKeyModifiers: [theEvent modifierFlags]];
   }

   NSEventPhase phase = [theEvent phase];
   Qt::ScrollPhase ph = Qt::ScrollUpdate;

   if (phase == NSEventPhaseMayBegin) {
      m_scrolling = true;
      ph = Qt::ScrollBegin;
   }


   if (phase == NSEventPhaseBegan) {
      // If MayBegin did not happen, Began is the actual beginning.
      if (! m_scrolling) {
         ph = Qt::ScrollBegin;
      }
      m_scrolling = true;

   } else if (phase == NSEventPhaseEnded || phase == NSEventPhaseCancelled ||
      momentumPhase == NSEventPhaseEnded || momentumPhase == NSEventPhaseCancelled) {
      ph = Qt::ScrollEnd;
      m_scrolling = false;
   } else if (phase == NSEventPhaseNone && momentumPhase == NSEventPhaseNone) {
      ph = Qt::NoScrollPhase;
      if (!QApplicationPrivate::scrollNoPhaseAllowed) {
         ph = Qt::ScrollUpdate;
      }
   }

   QWindowSystemInterface::handleWheelEvent(m_window, qt_timestamp, qt_windowPoint, qt_screenPoint, pixelDelta, angleDelta,
      currentWheelModifiers, ph, source);
}
#endif //QT_NO_WHEELEVENT

- (int) convertKeyCode: (QChar)keyChar
{
   return qt_mac_cocoaKey2QtKey(keyChar);
}

+ (Qt::KeyboardModifiers) convertKeyModifiers: (ulong)modifierFlags
{
   Qt::KeyboardModifiers qtMods = Qt::NoModifier;

   if (modifierFlags &  NSEventModifierFlagShift) {
      qtMods |= Qt::ShiftModifier;
   }

   if (modifierFlags & NSEventModifierFlagControl) {
      qtMods |= Qt::MetaModifier;
   }

   if (modifierFlags & NSEventModifierFlagOption) {
      qtMods |= Qt::AltModifier;
   }

   if (modifierFlags & NSEventModifierFlagCommand) {
      qtMods |= Qt::ControlModifier;
   }

   if (modifierFlags & NSEventModifierFlagNumericPad) {
      qtMods |= Qt::KeypadModifier;
   }

   return qtMods;
}

- (void)handleKeyEvent: (NSEvent *)nsevent eventType: (int)eventType
{
   ulong timestamp = [nsevent timestamp] * 1000;
   ulong nativeModifiers = [nsevent modifierFlags];
   Qt::KeyboardModifiers modifiers = [QNSView convertKeyModifiers: nativeModifiers];

   NSString *charactersIgnoringModifiers = [nsevent charactersIgnoringModifiers];
   NSString *characters = [nsevent characters];

   if (m_inputSource != characters) {
      [m_inputSource release];
      m_inputSource = [characters retain];
   }

   // There is no way to get the scan code from carbon/cocoa. But we cannot
   // use the value 0, since it indicates that the event originates from somewhere
   // else than the keyboard.

   quint32 nativeScanCode = 1;
   quint32 nativeVirtualKey = [nsevent keyCode];

   QChar ch = QChar::ReplacementCharacter;
   int keyCode = Qt::Key_unknown;
   if ([characters length] != 0) {
      if (((modifiers & Qt::MetaModifier) || (modifiers & Qt::AltModifier)) && ([charactersIgnoringModifiers length] != 0)) {
         ch = QChar([charactersIgnoringModifiers characterAtIndex: 0]);
      } else {
         ch = QChar([characters characterAtIndex: 0]);
      }
      keyCode = [self convertKeyCode: ch];
   }

   // we will send a key event unless the input method sets m_sendKeyEvent to false
   m_sendKeyEvent = true;
   QString text;

   // ignore text for the U+F700-U+F8FF range. This is used by Cocoa when
   // delivering function keys (e.g. arrow keys, backspace, F1-F35, etc.)
   if (!(modifiers & (Qt::ControlModifier | Qt::MetaModifier)) && (ch.unicode() < 0xf700 || ch.unicode() > 0xf8ff)) {
      text = QCFString::toQString(characters);
   }

   QWindow *window = [self topLevelWindow];

   // Popups implicitly grab key events; forward to the active popup if there is one.
   // This allows popups to e.g. intercept shortcuts and close the popup in response.
   if (QCocoaWindow *popup = QCocoaIntegration::instance()->activePopupWindow()) {
      if (!popup->m_windowFlags.testFlag(Qt::ToolTip)) {
         window = popup->window();
      }
   }

   if (eventType == QEvent::KeyPress) {

      if (m_composingText.isEmpty()) {
         m_sendKeyEvent = !QWindowSystemInterface::handleShortcutEvent(window, timestamp, keyCode,
               modifiers, nativeScanCode, nativeVirtualKey, nativeModifiers, text, [nsevent isARepeat], 1);
      }

      QObject *fo = QApplication::focusObject();
      if (m_sendKeyEvent && fo) {
         QInputMethodQueryEvent queryEvent(Qt::ImEnabled | Qt::ImHints);

         if (QCoreApplication::sendEvent(fo, &queryEvent)) {
            bool imEnabled = queryEvent.value(Qt::ImEnabled).toBool();
            Qt::InputMethodHints hints = static_cast<Qt::InputMethodHints>(queryEvent.value(Qt::ImHints).toUInt());

            if (imEnabled && !(hints & Qt::ImhDigitsOnly || hints & Qt::ImhFormattedNumbersOnly || hints & Qt::ImhHiddenText)) {
               // pass the key event to the input method. note that m_sendKeyEvent may be set to false during this call
               m_currentlyInterpretedKeyEvent = nsevent;
               [self interpretKeyEvents: [NSArray arrayWithObject: nsevent]];
               m_currentlyInterpretedKeyEvent = nullptr;
            }
         }
      }

      if (m_resendKeyEvent) {
         m_sendKeyEvent = true;
      }
   }

   if (m_sendKeyEvent && m_composingText.isEmpty())
      QWindowSystemInterface::handleExtendedKeyEvent(window, timestamp, QEvent::Type(eventType), keyCode, modifiers,
         nativeScanCode, nativeVirtualKey, nativeModifiers, text, [nsevent isARepeat], 1, false);

   m_sendKeyEvent = false;
   m_resendKeyEvent = false;
}

- (void)keyDown: (NSEvent *)nsevent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super keyDown: nsevent];
   }

   [self handleKeyEvent: nsevent eventType: int(QEvent::KeyPress)];
}

- (void)keyUp: (NSEvent *)nsevent
{
   if (m_window && (m_window->flags() & Qt::WindowTransparentForInput) ) {
      return [super keyUp: nsevent];
   }

   [self handleKeyEvent: nsevent eventType: int(QEvent::KeyRelease)];
}

- (void)cancelOperation: (id)sender
{
   NSEvent *currentEvent = [NSApp currentEvent];
   if (!currentEvent || currentEvent.type != NSEventTypeKeyDown) {
      return;
   }

   // Handling the key event may recurse back here through interpretKeyEvents
   // (when IM is enabled), so we need to guard against that.
   if (currentEvent == m_currentlyInterpretedKeyEvent) {
      return;
   }

   // Send Command+Key_Period and Escape as normal keypresses so that
   // the key sequence is delivered through Qt. That way clients can
   // intercept the shortcut and override its effect.
   [self handleKeyEvent: currentEvent eventType: int(QEvent::KeyPress)];
}

- (void)flagsChanged: (NSEvent *)nsevent
{
   ulong timestamp = [nsevent timestamp] * 1000;
   ulong modifiers = [nsevent modifierFlags];
   Qt::KeyboardModifiers qmodifiers = [QNSView convertKeyModifiers: modifiers];

   // calculate the delta and remember the current modifiers for next time
   static ulong m_lastKnownModifiers;
   ulong lastKnownModifiers = m_lastKnownModifiers;
   ulong delta = lastKnownModifiers ^ modifiers;
   m_lastKnownModifiers = modifiers;

   struct qt_mac_enum_mapper {
      ulong mac_mask;
      Qt::Key qt_code;
   };

   static qt_mac_enum_mapper modifier_key_symbols[] = {
      { NSEventModifierFlagShift,    Qt::Key_Shift },
      { NSEventModifierFlagControl,  Qt::Key_Meta },
      { NSEventModifierFlagCommand,  Qt::Key_Control },
      { NSEventModifierFlagOption,   Qt::Key_Alt },
      { NSEventModifierFlagCapsLock, Qt::Key_CapsLock },
      { 0ul, Qt::Key_unknown }
   };

   for (int i = 0; modifier_key_symbols[i].mac_mask != 0u; ++i) {
      uint mac_mask = modifier_key_symbols[i].mac_mask;

      if ((delta & mac_mask) == 0u) {
         continue;
      }

      QWindowSystemInterface::handleKeyEvent(m_window, timestamp,
         (lastKnownModifiers & mac_mask) ? QEvent::KeyRelease : QEvent::KeyPress,
         modifier_key_symbols[i].qt_code,
         qmodifiers ^ [QNSView convertKeyModifiers: mac_mask]);
   }
}

- (void) insertNewline: (id)sender
{
   (void) sender;
   m_resendKeyEvent = true;
}

- (void) doCommandBySelector: (SEL)aSelector
{
   [self tryToPerform: aSelector with: self];
}

- (void) insertText: (id)aString replacementRange: (NSRange)replacementRange
{
   (void) replacementRange;

   if (m_sendKeyEvent && m_composingText.isEmpty() && [aString isEqualToString: m_inputSource]) {
      // don't send input method events for simple text input (let handleKeyEvent send key events instead)
      return;
   }

   QString commitString;
   if ([aString length]) {
      if ([aString isKindOfClass: [NSAttributedString class]]) {
         commitString = QCFString::toQString(reinterpret_cast<CFStringRef>([aString string]));
      } else {
         commitString = QCFString::toQString(reinterpret_cast<CFStringRef>(aString));
      };
   }
   QObject *fo = QApplication::focusObject();
   if (fo) {
      QInputMethodQueryEvent queryEvent(Qt::ImEnabled);
      if (QCoreApplication::sendEvent(fo, &queryEvent)) {
         if (queryEvent.value(Qt::ImEnabled).toBool()) {
            QInputMethodEvent e;
            e.setCommitString(commitString);
            QCoreApplication::sendEvent(fo, &e);
            // prevent handleKeyEvent from sending a key event
            m_sendKeyEvent = false;
         }
      }
   }

   m_composingText.clear();
}

- (void) setMarkedText: (id)aString selectedRange: (NSRange)selectedRange replacementRange: (NSRange)replacementRange
{
   (void) replacementRange;
   QString preeditString;

   QList<QInputMethodEvent::Attribute> attrs;
   attrs << QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, selectedRange.location + selectedRange.length, 1, QVariant());

   if ([aString isKindOfClass: [NSAttributedString class]]) {
      // Preedit string has attribution
      preeditString = QCFString::toQString(reinterpret_cast<CFStringRef>([aString string]));
      int composingLength = preeditString.length();
      int index = 0;
      // Create attributes for individual sections of preedit text
      while (index < composingLength) {
         NSRange effectiveRange;
         NSRange range = NSMakeRange(index, composingLength - index);
         NSDictionary *attributes = [aString attributesAtIndex: index
                                         longestEffectiveRange: &effectiveRange
                                                       inRange: range];
         NSNumber *underlineStyle = [attributes objectForKey: NSUnderlineStyleAttributeName];
         if (underlineStyle) {
            QColor clr (Qt::black);
            NSColor *color = [attributes objectForKey: NSUnderlineColorAttributeName];
            if (color) {
               clr = qt_mac_toQColor(color);
            }
            QTextCharFormat format;
            format.setFontUnderline(true);
            format.setUnderlineColor(clr);
            attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,
                  effectiveRange.location,
                  effectiveRange.length,
                  format);
         }
         index = effectiveRange.location + effectiveRange.length;
      }
   } else {
      // No attributes specified, take only the preedit text.
      preeditString = QCFString::toQString(reinterpret_cast<CFStringRef>(aString));
   }

   if (attrs.isEmpty()) {
      QTextCharFormat format;
      format.setFontUnderline(true);
      attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,
            0, preeditString.length(), format);
   }

   m_composingText = preeditString;

   QObject *fo = QApplication::focusObject();
   if (fo) {
      QInputMethodQueryEvent queryEvent(Qt::ImEnabled);
      if (QCoreApplication::sendEvent(fo, &queryEvent)) {
         if (queryEvent.value(Qt::ImEnabled).toBool()) {
            QInputMethodEvent e(preeditString, attrs);
            QCoreApplication::sendEvent(fo, &e);
            // prevent handleKeyEvent from sending a key event
            m_sendKeyEvent = false;
         }
      }
   }
}

- (void) unmarkText
{
   if (!m_composingText.isEmpty()) {
      QObject *fo = QApplication::focusObject();
      if (fo) {
         QInputMethodQueryEvent queryEvent(Qt::ImEnabled);
         if (QCoreApplication::sendEvent(fo, &queryEvent)) {
            if (queryEvent.value(Qt::ImEnabled).toBool()) {
               QInputMethodEvent e;
               e.setCommitString(m_composingText);
               QCoreApplication::sendEvent(fo, &e);
            }
         }
      }
   }
   m_composingText.clear();
}

- (BOOL) hasMarkedText
{
   return (m_composingText.isEmpty() ? NO : YES);
}

- (NSAttributedString *) attributedSubstringForProposedRange: (NSRange)aRange actualRange: (NSRangePointer)actualRange
{
   (void) actualRange;
   QObject *fo = QApplication::focusObject();
   if (!fo) {
      return nil;
   }

   QInputMethodQueryEvent queryEvent(Qt::ImEnabled | Qt::ImCurrentSelection);
   if (!QCoreApplication::sendEvent(fo, &queryEvent)) {
      return nil;
   }

   if (!queryEvent.value(Qt::ImEnabled).toBool()) {
      return nil;
   }

   QString selectedText = queryEvent.value(Qt::ImCurrentSelection).toString();
   if (selectedText.isEmpty()) {
      return nil;
   }

   QCFString str(selectedText.mid(aRange.location, aRange.length));
   const NSString *tmpString = reinterpret_cast<const NSString *>(str.toCFStringRef());

   return [[[NSAttributedString alloc]  initWithString: const_cast<NSString *>(tmpString)] autorelease];
}

- (NSRange) markedRange
{
   NSRange range;
   if (!m_composingText.isEmpty()) {
      range.location = 0;
      range.length = m_composingText.length();
   } else {
      range.location = NSNotFound;
      range.length = 0;
   }
   return range;
}

- (NSRange) selectedRange
{
   NSRange selectedRange = {0, 0};

   QObject *fo = QApplication::focusObject();
   if (!fo) {
      return selectedRange;
   }

   QInputMethodQueryEvent queryEvent(Qt::ImEnabled | Qt::ImCurrentSelection);
   if (! QCoreApplication::sendEvent(fo, &queryEvent)) {
      return selectedRange;
   }

   if (! queryEvent.value(Qt::ImEnabled).toBool()) {
      return selectedRange;
   }

   QString selectedText = queryEvent.value(Qt::ImCurrentSelection).toString();

   if (!selectedText.isEmpty()) {
      selectedRange.location = 0;
      selectedRange.length = selectedText.length();
   }
   return selectedRange;
}

- (NSRect) firstRectForCharacterRange: (NSRange)aRange actualRange: (NSRangePointer)actualRange
{
   QObject *fo = QApplication::focusObject();

   if (! fo) {
      return NSZeroRect;
   }

   QInputMethodQueryEvent queryEvent(Qt::ImEnabled);
   if (!QCoreApplication::sendEvent(fo, &queryEvent)) {
      return NSZeroRect;
   }

   if (!queryEvent.value(Qt::ImEnabled).toBool()) {
      return NSZeroRect;
   }

   if (!m_window) {
      return NSZeroRect;
   }

   // The returned rect is always based on the internal cursor.
   QRect mr = qApp->inputMethod()->cursorRectangle().toRect();
   QPoint mp = m_window->mapToGlobal(mr.bottomLeft());

   NSRect rect;
   rect.origin.x = mp.x();
   rect.origin.y = qt_mac_flipYCoordinate(mp.y());
   rect.size.width = mr.width();
   rect.size.height = mr.height();
   return rect;
}

- (NSUInteger)characterIndexForPoint: (NSPoint)aPoint
{
   // do not support cursor movements using mouse while composing.
   return NSNotFound;
}

- (NSArray *) validAttributesForMarkedText
{
   if (m_window != QApplication::focusWindow()) {
      return nil;
   }

   QObject *fo = QApplication::focusObject();
   if (!fo) {
      return nil;
   }

   QInputMethodQueryEvent queryEvent(Qt::ImEnabled);
   if (!QCoreApplication::sendEvent(fo, &queryEvent)) {
      return nil;
   }

   if (!queryEvent.value(Qt::ImEnabled).toBool()) {
      return nil;
   }

   // Support only underline color/style.
   return [NSArray arrayWithObjects: NSUnderlineColorAttributeName,
                 NSUnderlineStyleAttributeName, nil];
}

-(void)registerDragTypes
{
   QMacAutoReleasePool pool;
   QStringList customTypes = qt_mac_enabledDraggedTypes();

   if (currentCustomDragTypes == nullptr || *currentCustomDragTypes != customTypes) {
      if (currentCustomDragTypes == nullptr) {
         currentCustomDragTypes = new QStringList();
      }

      *currentCustomDragTypes = customTypes;
      const NSString *mimeTypeGeneric = @"com.copperspice.cs.MimeTypeName";

      NSMutableArray *supportedTypes = [NSMutableArray arrayWithObjects: NSColorPboardType,
                                        NSFilenamesPboardType, NSStringPboardType,
                                        NSFilenamesPboardType, NSPostScriptPboardType, NSTIFFPboardType,
                                        NSRTFPboardType, NSTabularTextPboardType, NSFontPboardType,
                                        NSRulerPboardType, NSFileContentsPboardType, NSColorPboardType,
                                        NSRTFDPboardType, NSHTMLPboardType,
                                        NSURLPboardType, NSPDFPboardType, NSVCardPboardType,
                                        NSFilesPromisePboardType, NSInkTextPboardType,
                                        NSMultipleTextSelectionPboardType, mimeTypeGeneric, nil];
      // Add custom types supported by the application.
      for (int i = 0; i < customTypes.size(); i++) {
         [supportedTypes addObject: QCFString::toNSString(customTypes[i])];
      }

      [self registerForDraggedTypes: supportedTypes];
   }
}

static QWindow *findEventTargetWindow(QWindow *candidate)
{
   while (candidate) {
      if (! (candidate->flags() & Qt::WindowTransparentForInput)) {
         return candidate;
      }
      candidate = candidate->parent();
   }
   return candidate;
}

static QPoint mapWindowCoordinates(QWindow *source, QWindow *target, QPoint point)
{
   return target->mapFromGlobal(source->mapToGlobal(point));
}

- (NSDragOperation)draggingSession: (NSDraggingSession *)session
   sourceOperationMaskForDraggingContext: (NSDraggingContext)context
{
   (void) session;
   (void) context;
   QCocoaDrag *nativeDrag = QCocoaIntegration::instance()->drag();
   return qt_mac_mapDropActions(nativeDrag->currentDrag()->supportedActions());
}

- (BOOL)ignoreModifierKeysForDraggingSession: (NSDraggingSession *)session
{
   (void) session;
   // According to the "Dragging Sources" chapter on Cocoa DnD Programming
   // (https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/DragandDrop/Concepts/dragsource.html),
   // if the control, option, or command key is pressed, the source’s
   // operation mask is filtered to only contain a reduced set of operations.
   //
   // Since CS already takes care of tracking the keyboard modifiers, we
   // don't need (or want) Cocoa to filter anything. Instead, we will let
   // the application do the actual filtering.
   return YES;
}

- (BOOL)wantsPeriodicDraggingUpdates
{
   // From the documentation:
   //
   // "If the destination returns NO, these messages are sent only when the mouse moves
   //  or a modifier flag changes. Otherwise the destination gets the default behavior,
   //  where it receives periodic dragging-updated messages even if nothing changes."
   //
   // We do not want these constant drag update events while mouse is stationary,
   // since we do all animations (autoscroll) with timers.
   return NO;
}

- (void)updateCursorFromDragResponse: (QPlatformDragQtResponse)response drag: (QCocoaDrag *)drag
{
   const QPixmap pixmapCursor = drag->currentDrag()->dragCursor(response.acceptedAction());
   NSCursor *nativeCursor = nil;

   if (pixmapCursor.isNull()) {
      switch (response.acceptedAction()) {
         case Qt::CopyAction:
            nativeCursor = [NSCursor dragCopyCursor];
            break;
         case Qt::LinkAction:
            nativeCursor = [NSCursor dragLinkCursor];
            break;
         case Qt::IgnoreAction:
         // Uncomment the next lines if forbiden cursor wanted on non droppable targets.
         /*nativeCursor = [NSCursor operationNotAllowedCursor];
         break;*/
         case Qt::MoveAction:
         default:
            nativeCursor = [NSCursor arrowCursor];
            break;
      }
   } else {
      NSImage *nsimage = qt_mac_create_nsimage(pixmapCursor);
      nativeCursor = [[NSCursor alloc] initWithImage: nsimage hotSpot: NSZeroPoint];
      [nsimage release];
   }

   // change the cursor
   [nativeCursor set];

   // Make sure the cursor is updated correctly if the mouse does not move and window is under cursor
   // by creating a fake move event
   if (m_updatingDrag) {
      return;
   }

   const QPoint mousePos(QCursor::pos());

   CGEventRef moveEvent(CGEventCreateMouseEvent(nullptr, kCGEventMouseMoved,
         CGPointMake(mousePos.x(), mousePos.y()), kCGMouseButtonLeft));

   CGEventPost(kCGHIDEventTap, moveEvent);
   CFRelease(moveEvent);
}

- (NSDragOperation)draggingEntered: (id <NSDraggingInfo>)sender
{
   return [self handleDrag: sender];
}

- (NSDragOperation)draggingUpdated: (id <NSDraggingInfo>)sender
{
   m_updatingDrag = true;
   const NSDragOperation ret([self handleDrag: sender]);
   m_updatingDrag = false;

   return ret;
}

// Sends drag update to Qt, return the action
- (NSDragOperation)handleDrag: (id <NSDraggingInfo>)sender
{
   NSPoint windowPoint = [self convertPoint: [sender draggingLocation] fromView: nil];
   QPoint qt_windowPoint(windowPoint.x, windowPoint.y);
   Qt::DropActions qtAllowed = qt_mac_mapNSDragOperations([sender draggingSourceOperationMask]);

   QWindow *target = findEventTargetWindow(m_window);
   if (! target) {
      return NSDragOperationNone;
   }

   // update these so selecting move/copy/link works
   QApplicationPrivate::modifier_buttons = [QNSView convertKeyModifiers: [[NSApp currentEvent] modifierFlags]];

   QPlatformDragQtResponse response(false, Qt::IgnoreAction, QRect());
   QCocoaDrag *nativeDrag = QCocoaIntegration::instance()->drag();
   if (nativeDrag->currentDrag()) {
      // The drag was started from within the application
      response = QWindowSystemInterface::handleDrag(target, nativeDrag->platformDropData(), mapWindowCoordinates(m_window, target,
               qt_windowPoint), qtAllowed);
      [self updateCursorFromDragResponse: response drag: nativeDrag];

   } else {
      QCocoaDropData mimeData([sender draggingPasteboard]);
      response = QWindowSystemInterface::handleDrag(target, &mimeData, mapWindowCoordinates(m_window, target, qt_windowPoint), qtAllowed);
   }

   return qt_mac_mapDropAction(response.acceptedAction());
}

- (void)draggingExited: (id <NSDraggingInfo>)sender
{
   QWindow *target = findEventTargetWindow(m_window);
   if (! target) {
      return;
   }

   NSPoint windowPoint = [self convertPoint: [sender draggingLocation] fromView: nil];
   QPoint qt_windowPoint(windowPoint.x, windowPoint.y);

   // Send 0 mime data to indicate drag exit
   QWindowSystemInterface::handleDrag(target, nullptr, mapWindowCoordinates(m_window, target, qt_windowPoint), Qt::IgnoreAction);
}

// called on drop, send the drop to CS and return if it was accepted.
- (BOOL)performDragOperation: (id <NSDraggingInfo>)sender
{
   QWindow *target = findEventTargetWindow(m_window);
   if (! target) {
      return false;
   }

   NSPoint windowPoint = [self convertPoint: [sender draggingLocation] fromView: nil];
   QPoint qt_windowPoint(windowPoint.x, windowPoint.y);
   Qt::DropActions qtAllowed = qt_mac_mapNSDragOperations([sender draggingSourceOperationMask]);

   QPlatformDropQtResponse response(false, Qt::IgnoreAction);
   QCocoaDrag *nativeDrag = QCocoaIntegration::instance()->drag();
   if (nativeDrag->currentDrag()) {
      // The drag was started from within the application
      response = QWindowSystemInterface::handleDrop(target, nativeDrag->platformDropData(), mapWindowCoordinates(m_window, target,
               qt_windowPoint), qtAllowed);

   } else {
      QCocoaDropData mimeData([sender draggingPasteboard]);
      response = QWindowSystemInterface::handleDrop(target, &mimeData, mapWindowCoordinates(m_window, target, qt_windowPoint), qtAllowed);
   }
   if (response.isAccepted()) {
      QCocoaDrag *nativeDrag = QCocoaIntegration::instance()->drag();
      nativeDrag->setAcceptedAction(response.acceptedAction());
   }
   return response.isAccepted();
}

- (void)draggingSession: (NSDraggingSession *)session
           endedAtPoint: (NSPoint)screenPoint
              operation: (NSDragOperation)operation
{
   QWindow *target = findEventTargetWindow(m_window);

   if (! target) {
      return;
   }

   // keep our state, and QApplication state (buttons member) in-sync,
   // or future mouse events will be processed incorrectly

   NSUInteger pmb = [NSEvent pressedMouseButtons];
   for (int buttonNumber = 0; buttonNumber < 32; buttonNumber++) { // see cocoaButton2QtButton() for the 32 value
      if (! (pmb & (1 << buttonNumber))) {
         m_buttons &= ~cocoaButton2QtButton(buttonNumber);
      }
   }

   NSPoint windowPoint = [self.window convertRectFromScreen: NSMakeRect(screenPoint.x, screenPoint.y, 1, 1)].origin;
   QPoint qtWindowPoint(windowPoint.x, windowPoint.y);

   QPoint qtScreenPoint = QPoint(screenPoint.x, qt_mac_flipYCoordinate(screenPoint.y));

   QWindowSystemInterface::handleMouseEvent(target, mapWindowCoordinates(m_window, target, qtWindowPoint), qtScreenPoint, m_buttons);
}

@end
