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

#include "qcocoaintegration.h"

#include "qcocoawindow.h"
#include "qcocoabackingstore.h"
#include "qcocoanativeinterface.h"
#include "qcocoamenuloader.h"
#include "qcocoaeventdispatcher.h"
#include "qcocoahelpers.h"
#include "qcocoaapplication.h"
#include "qcocoaapplicationdelegate.h"
#include "qcocoafiledialoghelper.h"
#include "qcocoatheme.h"
#include "qcocoainputcontext.h"
#include "qcocoamimetypes.h"
#include "qcocoaaccessibility.h"
#include <qcoreapplication.h>
#include <qplatform_accessibility.h>

#include <qplatform_inputcontextfactory_p.h>
#include <qplatform_inputcontextfactory_p.h>

#include <IOKit/graphics/IOGraphicsLib.h>

static void initResources()
{
   Q_INIT_RESOURCE(qcocoaresources);
}

QCocoaScreen::QCocoaScreen(int screenIndex)
   : QPlatformScreen(), m_screenIndex(screenIndex), m_refreshRate(60.0)
{
   updateGeometry();
   m_cursor = new QCocoaCursor;
}

QCocoaScreen::~QCocoaScreen()
{
   delete m_cursor;
}

NSScreen *QCocoaScreen::osScreen() const
{
   NSArray *screens = [NSScreen screens];
   return ((NSUInteger)m_screenIndex < [screens count]) ? [screens objectAtIndex: m_screenIndex] : nil;
}

void QCocoaScreen::updateGeometry()
{
   NSScreen *nsScreen = osScreen();
   if (!nsScreen) {
      return;
   }

   NSRect frameRect = [nsScreen frame];

   if (m_screenIndex == 0) {
      m_geometry = QRect(frameRect.origin.x, frameRect.origin.y, frameRect.size.width, frameRect.size.height);
      // This is the primary screen, the one that contains the menubar. Its origin should be
      // (0, 0), and it's the only one whose available geometry differs from its full geometry.
      NSRect visibleRect = [nsScreen visibleFrame];
      m_availableGeometry = QRect(visibleRect.origin.x,
            frameRect.size.height - (visibleRect.origin.y + visibleRect.size.height), // invert y
            visibleRect.size.width, visibleRect.size.height);
   } else {
      // NSScreen origin is at the bottom-left corner, QScreen is at the top-left corner.
      // When we get the NSScreen frame rect, we need to re-align its origin y coordinate
      // w.r.t. the primary screen, whose origin is (0, 0).
      NSRect r = [[[NSScreen screens] objectAtIndex: 0] frame];
      QRect referenceScreenGeometry = QRect(r.origin.x, r.origin.y, r.size.width, r.size.height);
      m_geometry = QRect(frameRect.origin.x,
            referenceScreenGeometry.height() - (frameRect.origin.y + frameRect.size.height),
            frameRect.size.width, frameRect.size.height);

      // Not primary screen. See above.
      m_availableGeometry = m_geometry;
   }

   m_format = QImage::Format_RGB32;
   m_depth = NSBitsPerPixelFromDepth([nsScreen depth]);

   NSDictionary *devDesc = [nsScreen deviceDescription];
   CGDirectDisplayID dpy = [[devDesc objectForKey: @"NSScreenNumber"] unsignedIntValue];
   CGSize size = CGDisplayScreenSize(dpy);
   m_physicalSize = QSizeF(size.width, size.height);
   m_logicalDpi.first = 72;
   m_logicalDpi.second = 72;
   CGDisplayModeRef displayMode = CGDisplayCopyDisplayMode(dpy);
   float refresh = CGDisplayModeGetRefreshRate(displayMode);
   CGDisplayModeRelease(displayMode);
   if (refresh > 0) {
      m_refreshRate = refresh;
   }

   // Get m_name (brand/model of the monitor)
   NSDictionary *deviceInfo = (NSDictionary *)IODisplayCreateInfoDictionary(CGDisplayIOServicePort(dpy), kIODisplayOnlyPreferredName);
   NSDictionary *localizedNames = [deviceInfo objectForKey: [NSString stringWithUTF8String: kDisplayProductName]];
   if ([localizedNames count] > 0) {
      m_name = QString::fromUtf8([[localizedNames objectForKey: [[localizedNames allKeys] objectAtIndex: 0]] UTF8String]);
   }
   [deviceInfo release];

   QWindowSystemInterface::handleScreenGeometryChange(screen(), geometry(), availableGeometry());
   QWindowSystemInterface::handleScreenLogicalDotsPerInchChange(screen(), m_logicalDpi.first, m_logicalDpi.second);
   QWindowSystemInterface::handleScreenRefreshRateChange(screen(), m_refreshRate);
}

qreal QCocoaScreen::devicePixelRatio() const
{
   QMacAutoReleasePool pool;
   NSScreen *screen = osScreen();
   return qreal(screen ? [screen backingScaleFactor] : 1.0);
}

QPlatformScreen::SubpixelAntialiasingType QCocoaScreen::subpixelAntialiasingTypeHint() const
{
   QPlatformScreen::SubpixelAntialiasingType type = QPlatformScreen::subpixelAntialiasingTypeHint();
   if (type == QPlatformScreen::Subpixel_None) {
      // Every OSX machine has RGB pixels unless a peculiar or rotated non-Apple screen is attached
      type = QPlatformScreen::Subpixel_RGB;
   }
   return type;
}

QWindow *QCocoaScreen::topLevelWindowAt(const QPoint &point) const
{
   NSPoint screenPoint = qt_mac_flipPoint(point);

   // Search (hit test) for the top-level window. [NSWidow windowNumberAtPoint:
   // belowWindowWithWindowNumber] may return windows that are not interesting
   // to Qt. The search iterates until a suitable window or no window is found.
   NSInteger topWindowNumber = 0;
   QWindow *window = nullptr;

   do {
      // Get the top-most window, below any previously rejected window.
      topWindowNumber = [NSWindow windowNumberAtPoint: screenPoint
                          belowWindowWithWindowNumber: topWindowNumber];

      // Continue the search if the window does not belong to this process.
      NSWindow *nsWindow = [NSApp windowWithWindowNumber: topWindowNumber];
      if (nsWindow == nullptr) {
         continue;
      }

      // Continue the search if the window does not belong to Qt.
      if (![nsWindow conformsToProtocol: @protocol(QNSWindowProtocol)]) {
         continue;
      }

      id<QNSWindowProtocol> proto = static_cast<id<QNSWindowProtocol>>(nsWindow);
      QCocoaWindow *cocoaWindow = proto.helper.platformWindow;
      if (!cocoaWindow) {
         continue;
      }
      window = cocoaWindow->window();

      // Continue the search if the window is not a top-level window.
      if (!window->isTopLevel()) {
         continue;
      }

      // Stop searching. The current window is the correct window.
      break;
   } while (topWindowNumber > 0);

   return window;
}

extern CGContextRef qt_mac_cg_context(const QPaintDevice *pdev);

QPixmap QCocoaScreen::grabWindow(WId window, int x, int y, int width, int height) const
{
   // TODO window should be handled
   (void) window;

   const int maxDisplays = 128; // 128 displays should be enough for everyone.
   CGDirectDisplayID displays[maxDisplays];
   CGDisplayCount displayCount;
   CGRect cgRect;

   if (width < 0 || height < 0) {
      // get all displays
      cgRect = CGRectInfinite;
   } else {
      cgRect = CGRectMake(x, y, width, height);
   }
   const CGDisplayErr err = CGGetDisplaysWithRect(cgRect, maxDisplays, displays, &displayCount);

   if (err && displayCount == 0) {
      return QPixmap();
   }

   // calculate pixmap size
   QSize windowSize(width, height);
   if (width < 0 || height < 0) {
      QRect windowRect;
      for (uint i = 0; i < displayCount; ++i) {
         const CGRect cgRect = CGDisplayBounds(displays[i]);
         QRect qRect(cgRect.origin.x, cgRect.origin.y, cgRect.size.width, cgRect.size.height);
         windowRect = windowRect.united(qRect);
      }
      if (width < 0) {
         windowSize.setWidth(windowRect.width());
      }
      if (height < 0) {
         windowSize.setHeight(windowRect.height());
      }
   }

   QPixmap windowPixmap(windowSize * devicePixelRatio());
   windowPixmap.fill(Qt::transparent);

   for (uint i = 0; i < displayCount; ++i) {
      const CGRect bounds = CGDisplayBounds(displays[i]);
      int w = (width < 0 ? bounds.size.width : width) * devicePixelRatio();
      int h = (height < 0 ? bounds.size.height : height) * devicePixelRatio();
      QRect displayRect = QRect(x, y, w, h);
      displayRect = displayRect.translated(qRound(-bounds.origin.x), qRound(-bounds.origin.y));

      QCFType<CGImageRef> image = CGDisplayCreateImageForRect(displays[i],
            CGRectMake(displayRect.x(), displayRect.y(), displayRect.width(), displayRect.height()));

      QPixmap pix(w, h);
      pix.fill(Qt::transparent);
      CGRect rect = CGRectMake(0, 0, w, h);
      CGContextRef ctx = qt_mac_cg_context(&pix);

      qt_mac_drawCGImage(ctx, &rect, image);
      CGContextRelease(ctx);

      QPainter painter(&windowPixmap);
      painter.drawPixmap(0, 0, pix);
   }
   return windowPixmap;
}

static QCocoaIntegration::Options parseOptions(const QStringList &paramList)
{
   QCocoaIntegration::Options options;
   for (const QString &param : paramList) {

#if defined(QT_USE_FREETYPE)
      if (param == "fontengine=freetype") {
         options |= QCocoaIntegration::UseFreeTypeFontEngine;

      } else

#endif
         qWarning() << "Unknown option" << param;
   }

   return options;
}

QCocoaIntegration *QCocoaIntegration::mInstance = nullptr;

QCocoaIntegration::QCocoaIntegration(const QStringList &paramList)
   : mOptions(parseOptions(paramList)), mFontDb(new QCoreTextFontDatabase(mOptions.testFlag(UseFreeTypeFontEngine))),
#ifndef QT_NO_ACCESSIBILITY
     mAccessibility(new QCocoaAccessibility),
#endif
     mCocoaClipboard(new QCocoaClipboard), mCocoaDrag(new QCocoaDrag),
     mNativeInterface(new QCocoaNativeInterface), mServices(new QCocoaServices),
     mKeyboardMapper(new QCocoaKeyMapper)
{
   if (mInstance != nullptr) {
      qWarning("Creating multiple Cocoa platform integrations is not supported");
   }

   mInstance = this;

   QString icStr = QPlatformInputContextFactory::requested();

   icStr.isEmpty() ? mInputContext.reset(new QCocoaInputContext)
      : mInputContext.reset(QPlatformInputContextFactory::create(icStr));

   initResources();
   QMacAutoReleasePool pool;

   qApp->setAttribute(Qt::AA_DontUseNativeMenuBar, false);

   NSApplication *cocoaApplication = [QNSApplication sharedApplication];
   qt_redirectNSApplicationSendEvent();

   if (qgetenv("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM").isEmpty()) {
      // Applications launched from plain executables (without an app
      // bundle) are "background" applications that does not take keybaord
      // focus or have a dock icon or task switcher entry. Gui apps generally
      // wants to be foreground applications so change the process type. (But
      // see the function implementation for exceptions.)
      qt_mac_transformProccessToForegroundApplication();

      // Move the application window to front to make it take focus, also when launching
      // from the terminal. On 10.12+ this call has been moved to applicationDidFinishLauching
      // to work around issues with loss of focus at startup.

      if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_12) {
         // Ignoring other apps is necessary (we must ignore the terminal), but makes
         // apps play slightly less nice with other apps when lanching from Finder
         // (See the activateIgnoringOtherApps docs.)
         [cocoaApplication activateIgnoringOtherApps: YES];
      }
   }

   if (! QCoreApplication::testAttribute(Qt::AA_MacPluginApplication)) {

      // Set app delegate, link to the current delegate (if any)
      QCocoaApplicationDelegate *newDelegate = [QCocoaApplicationDelegate sharedDelegate];
      [newDelegate setReflectionDelegate: [cocoaApplication delegate]];
      [cocoaApplication setDelegate: newDelegate];

      // Load the application menu. This menu contains Preferences, Hide, Quit.
      QCocoaMenuLoader *qtMenuLoader = [[QCocoaMenuLoader alloc] init];
      [cocoaApplication setMenu: [qtMenuLoader menu]];
      [newDelegate setMenuLoader: qtMenuLoader];
   }

   // The presentation options such as whether or not the dock and/or menu bar is
   // hidden (automatically by the system) affects the main screen's available
   // geometry. Since we're initializing the screens synchronously at application
   // startup we need to ensure that the presentation options have been propagated
   // to the screen before we read out its properties. Normally OS X does this in
   // an asynchronous callback, but that's too late for us. We force the propagation
   // by explicitly setting the presentation option to the magic 'default value',
   // which will resolve to an actual value and result in screen invalidation.
   cocoaApplication.presentationOptions = NSApplicationPresentationDefault;
   updateScreens();

   QMacInternalPasteboardMime::initializeMimeTypes();
   QCocoaMimeTypes::initializeMimeTypes();
}

QCocoaIntegration::~QCocoaIntegration()
{
   mInstance = nullptr;

   qt_resetNSApplicationSendEvent();

   QMacAutoReleasePool pool;
   if (!QCoreApplication::testAttribute(Qt::AA_MacPluginApplication)) {
      // remove the apple event handlers installed by QCocoaApplicationDelegate
      QCocoaApplicationDelegate *delegate = [QCocoaApplicationDelegate sharedDelegate];
      [delegate removeAppleEventHandlers];
      // reset the application delegate
      [[NSApplication sharedApplication] setDelegate: nullptr];
   }

   // Delete the clipboard integration and destroy mime type converters.
   // Deleting the clipboard integration flushes promised pastes using
   // the mime converters - the ordering here is important.
   delete mCocoaClipboard;
   QMacInternalPasteboardMime::destroyMimeTypes();

   // Delete screens in reverse order to avoid crash in case of multiple screens
   while (!mScreens.isEmpty()) {
      destroyScreen(mScreens.takeLast());
   }

   clearToolbars();
}

QCocoaIntegration *QCocoaIntegration::instance()
{
   return mInstance;
}

QCocoaIntegration::Options QCocoaIntegration::options() const
{
   return mOptions;
}

void QCocoaIntegration::updateScreens()
{
   NSArray *scrs = [NSScreen screens];
   NSMutableArray *screens = [NSMutableArray arrayWithArray: scrs];

   if ([screens count] == 0)
      if ([NSScreen mainScreen]) {
         [screens addObject: [NSScreen mainScreen]];
      }

   if ([screens count] == 0) {
      return;
   }

   QSet<QCocoaScreen *> remainingScreens = QSet<QCocoaScreen *>::fromList(mScreens);
   QList<QPlatformScreen *> siblings;
   uint screenCount = [screens count];

   for (uint i = 0; i < screenCount; i++) {
      NSScreen *scr = [screens objectAtIndex: i];
      CGDirectDisplayID dpy = [[[scr deviceDescription] objectForKey: @"NSScreenNumber"] unsignedIntValue];

      // If this screen is a mirror and is not the primary one of the mirror set, ignore it.
      // Exception: The NSScreen API has been observed to a return a screen list with one
      // mirrored, non-primary screen when CS is running as a startup item. Always use the
      // screen if there's only one screen in the list.

      if (screenCount > 1 && CGDisplayIsInMirrorSet(dpy)) {
         CGDirectDisplayID primary = CGDisplayMirrorsDisplay(dpy);
         if (primary != kCGNullDirectDisplay && primary != dpy) {
            continue;
         }
      }

      QCocoaScreen *screen = nullptr;

      for (QCocoaScreen *existingScr : mScreens)
         // NSScreen documentation says do not cache the array returned from [NSScreen screens].
         // However in practice, we can identify a screen by its pointer: if resolution changes,
         // the NSScreen object will be the same instance, just with different values.
         if (existingScr->osScreen() == scr) {
            screen = existingScr;
            break;
         }
      if (screen) {
         remainingScreens.remove(screen);
         screen->updateGeometry();
      } else {
         screen = new QCocoaScreen(i);
         mScreens.append(screen);
         screenAdded(screen);
      }
      siblings << screen;
   }

   // Set virtual siblings list. All screens in mScreens are siblings, because we ignored the
   // mirrors. Note that some of the screens we update the siblings list for here may be deleted
   // below, but update anyway to keep the to-be-deleted screens out of the siblings list.
   for (QCocoaScreen *screen : mScreens) {
      screen->setVirtualSiblings(siblings);
   }

   // Now the leftovers in remainingScreens are no longer current, so we can delete them.
   for (QCocoaScreen *screen : remainingScreens) {
      mScreens.removeOne(screen);
      destroyScreen(screen);
   }
}

QCocoaScreen *QCocoaIntegration::screenAtIndex(int index)
{
   if (index >= mScreens.count()) {
      updateScreens();
   }

   // It is possible that the screen got removed while updateScreens was called
   // so we do a sanity check to be certain
   if (index >= mScreens.count()) {
      return nullptr;
   }

   return mScreens.at(index);
}

bool QCocoaIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
   switch (cap) {
      case ThreadedPixmaps:
#ifndef QT_NO_OPENGL
      case OpenGL:
      case ThreadedOpenGL:
      case BufferQueueingOpenGL:
#endif
      case WindowMasks:
      case MultipleWindows:
      case ForeignWindows:
      case RasterGLSurface:
      case ApplicationState:
      case ApplicationIcon:
         return true;

      default:
         return QPlatformIntegration::hasCapability(cap);
   }
}

QPlatformWindow *QCocoaIntegration::createPlatformWindow(QWindow *window) const
{
   return new QCocoaWindow(window);
}

#ifndef QT_NO_OPENGL
QPlatformOpenGLContext *QCocoaIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
   QCocoaGLContext *glContext = new QCocoaGLContext(context->format(),
      context->shareHandle(), context->nativeHandle());

   context->setNativeHandle(glContext->nativeHandle());

   return glContext;
}
#endif

QPlatformBackingStore *QCocoaIntegration::createPlatformBackingStore(QWindow *window) const
{
   return new QCocoaBackingStore(window);
}

QAbstractEventDispatcher *QCocoaIntegration::createEventDispatcher() const
{
   return new QCocoaEventDispatcher;
}

QCoreTextFontDatabase *QCocoaIntegration::fontDatabase() const
{
   return mFontDb.data();
}

QCocoaNativeInterface *QCocoaIntegration::nativeInterface() const
{
   return mNativeInterface.data();
}

QPlatformInputContext *QCocoaIntegration::inputContext() const
{
   return mInputContext.data();
}

#ifndef QT_NO_ACCESSIBILITY
QCocoaAccessibility *QCocoaIntegration::accessibility() const
{
   return mAccessibility.data();
}
#endif

QCocoaClipboard *QCocoaIntegration::clipboard() const
{
   return mCocoaClipboard;
}

QCocoaDrag *QCocoaIntegration::drag() const
{
   return mCocoaDrag.data();
}

QStringList QCocoaIntegration::themeNames() const
{
   return QStringList(QCocoaTheme::name);
}

QPlatformTheme *QCocoaIntegration::createPlatformTheme(const QString &name) const
{
   if (name == QCocoaTheme::name) {
      return new QCocoaTheme;
   }

   return QPlatformIntegration::createPlatformTheme(name);
}

QCocoaServices *QCocoaIntegration::services() const
{
   return mServices.data();
}

QVariant QCocoaIntegration::styleHint(StyleHint hint) const
{
   if (hint == QPlatformIntegration::FontSmoothingGamma) {
      return 2.0;
   }

   return QPlatformIntegration::styleHint(hint);
}

Qt::KeyboardModifiers QCocoaIntegration::queryKeyboardModifiers() const
{
   return QCocoaKeyMapper::queryKeyboardModifiers();
}

QList<int> QCocoaIntegration::possibleKeys(const QKeyEvent *event) const
{
   return mKeyboardMapper->possibleKeys(event);
}

void QCocoaIntegration::setToolbar(QWindow *window, NSToolbar *toolbar)
{
   if (NSToolbar *prevToolbar = mToolbars.value(window)) {
      [prevToolbar release];
   }

   [toolbar retain];
   mToolbars.insert(window, toolbar);
}

NSToolbar *QCocoaIntegration::toolbar(QWindow *window) const
{
   return mToolbars.value(window);
}

void QCocoaIntegration::clearToolbars()
{
   QHash<QWindow *, NSToolbar *>::const_iterator it = mToolbars.constBegin();
   while (it != mToolbars.constEnd()) {
      [it.value() release];
      ++it;
   }

   mToolbars.clear();
}

void QCocoaIntegration::pushPopupWindow(QCocoaWindow *window)
{
   m_popupWindowStack.append(window);
}

QCocoaWindow *QCocoaIntegration::popPopupWindow()
{
   if (m_popupWindowStack.isEmpty()) {
      return nullptr;
   }

   return m_popupWindowStack.takeLast();
}

QCocoaWindow *QCocoaIntegration::activePopupWindow() const
{
   if (m_popupWindowStack.isEmpty()) {
      return nullptr;
   }
   return m_popupWindowStack.front();
}

QList<QCocoaWindow *> *QCocoaIntegration::popupWindowStack()
{
   return &m_popupWindowStack;
}

void QCocoaIntegration::setApplicationIcon(const QIcon &icon) const
{
   NSImage *image = nil;
   if (!icon.isNull()) {
      NSSize size = [[[NSApplication sharedApplication] dockTile] size];
      QPixmap pixmap = icon.pixmap(size.width, size.height);
      image = static_cast<NSImage *>(qt_mac_create_nsimage(pixmap));
   }
   [[NSApplication sharedApplication] setApplicationIconImage: image];
   [image release];
}
