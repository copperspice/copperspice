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

#include <qcocoanativeinterface.h>

#include <qcocoaprintersupport.h>
#include <qcocoawindow.h>
#include <qcocoamenu.h>
#include <qcocoamenubar.h>
#include <qcocoahelpers.h>
#include <qcocoaapplication.h>
#include <qcocoaintegration.h>
#include <qcocoaeventdispatcher.h>
#include <qapplication.h>
#include <qdebug.h>
#include <qbytearray.h>
#include <qwindow.h>
#include <qpixmap.h>
#include <qplatform_window.h>
#include <qplatform_printersupport.h>
#include <qsurfaceformat.h>
#include <platformheaders/qcocoawindowfunctions.h>

#include <qprintengine_mac_p.h>

#ifndef QT_NO_OPENGL
#include <qplatform_openglcontext.h>
#include <qopenglcontext.h>
#include <qcocoaglcontext.h>
#endif

#include <Cocoa/Cocoa.h>

QCocoaNativeInterface::QCocoaNativeInterface()
{
}

#ifndef QT_NO_OPENGL
void *QCocoaNativeInterface::nativeResourceForContext(const QByteArray &resourceString, QOpenGLContext *context)
{
   if (! context) {
      return nullptr;
   }

   if (resourceString.toLower() == "nsopenglcontext") {
      return nsOpenGLContextForContext(context);
   }

   if (resourceString.toLower() == "cglcontextobj") {
      return cglContextForContext(context);
   }

   return nullptr;
}
#endif

void *QCocoaNativeInterface::nativeResourceForWindow(const QByteArray &resourceString, QWindow *window)
{
   if (! window->handle()) {
      return nullptr;
   }

   if (resourceString == "nsview") {
      return static_cast<QCocoaWindow *>(window->handle())->m_contentView;

#ifndef QT_NO_OPENGL
   } else if (resourceString == "nsopenglcontext") {
      return static_cast<QCocoaWindow *>(window->handle())->currentContext()->nsOpenGLContext();
#endif

   } else if (resourceString == "nswindow") {
      return static_cast<QCocoaWindow *>(window->handle())->m_nsWindow;
   }

   return nullptr;
}

QPlatformNativeInterface::FP_Integration QCocoaNativeInterface::nativeResourceFunctionForIntegration(const QByteArray &resource)
{
   if (resource.toLower() == "addtomimelist") {
      return FP_Integration(QCocoaNativeInterface::addToMimeList);
   }

   if (resource.toLower() == "removefrommimelist") {
      return FP_Integration(QCocoaNativeInterface::removeFromMimeList);
   }

   if (resource.toLower() == "registerdraggedtypes") {
      return FP_Integration(QCocoaNativeInterface::registerDraggedTypes);
   }

   if (resource.toLower() == "setdockmenu") {
      return FP_Integration(QCocoaNativeInterface::setDockMenu);
   }

   if (resource.toLower() == "qmenutonsmenu") {
      return FP_Integration(QCocoaNativeInterface::qMenuToNSMenu);
   }

   if (resource.toLower() == "qmenubartonsmenu") {
      return FP_Integration(QCocoaNativeInterface::qMenuBarToNSMenu);
   }

   if (resource.toLower() == "qimagetocgimage") {
      return FP_Integration(QCocoaNativeInterface::qImageToCGImage);
   }

   if (resource.toLower() == "cgimagetoqimage") {
      return FP_Integration(QCocoaNativeInterface::cgImageToQImage);
   }

   if (resource.toLower() == "setwindowcontentview") {
      return FP_Integration(QCocoaNativeInterface::setWindowContentView);
   }

   if (resource.toLower() == "registertouchwindow") {
      return FP_Integration(QCocoaNativeInterface::registerTouchWindow);
   }

   if (resource.toLower() == "setembeddedinforeignview") {
      return FP_Integration(QCocoaNativeInterface::setEmbeddedInForeignView);
   }

   if (resource.toLower() == "setcontentborderthickness") {
      return FP_Integration(QCocoaNativeInterface::setContentBorderThickness);
   }

   if (resource.toLower() == "registercontentborderarea") {
      return FP_Integration(QCocoaNativeInterface::registerContentBorderArea);
   }

   if (resource.toLower() == "setcontentborderareaenabled") {
      return FP_Integration(QCocoaNativeInterface::setContentBorderAreaEnabled);
   }

   if (resource.toLower() == "setcontentborderenabled") {
      return FP_Integration(QCocoaNativeInterface::setContentBorderEnabled);
   }

   if (resource.toLower() == "setnstoolbar") {
      return FP_Integration(QCocoaNativeInterface::setNSToolbar);
   }

   if (resource.toLower() == "testcontentborderposition") {
      return FP_Integration(QCocoaNativeInterface::testContentBorderPosition);
   }

   return nullptr;
}

void QCocoaNativeInterface::beep()
{
   NSBeep();
}

QPlatformPrinterSupport *QCocoaNativeInterface::createPlatformPrinterSupport()
{
   return new QCocoaPrinterSupport();
}

void *QCocoaNativeInterface::NSPrintInfoForPrintEngine(QPrintEngine *printEngine)
{
   QMacPrintEnginePrivate *macPrintEnginePriv = static_cast<QMacPrintEngine *>(printEngine)->d_func();

   if (macPrintEnginePriv->state == QPrinter::Idle && !macPrintEnginePriv->isPrintSessionInitialized()) {
      macPrintEnginePriv->initialize();
   }

   return macPrintEnginePriv->printInfo;
}

QPixmap QCocoaNativeInterface::defaultBackgroundPixmapForQWizard()
{
   QCFType<CFURLRef> url;
   const int ExpectedImageWidth = 242;
   const int ExpectedImageHeight = 414;

   if (LSFindApplicationForInfo(kLSUnknownCreator, CFSTR("com.apple.KeyboardSetupAssistant"), nullptr, nullptr, &url) == noErr) {
      QCFType<CFBundleRef> bundle = CFBundleCreate(kCFAllocatorDefault, url);

      if (bundle) {
         url = CFBundleCopyResourceURL(bundle, CFSTR("Background"), CFSTR("png"), nullptr);

         if (url) {
            QCFType<CGImageSourceRef> imageSource = CGImageSourceCreateWithURL(url, nullptr);
            QCFType<CGImageRef> image = CGImageSourceCreateImageAtIndex(imageSource, 0, nullptr);

            if (image) {
               int width = CGImageGetWidth(image);
               int height = CGImageGetHeight(image);
               if (width == ExpectedImageWidth && height == ExpectedImageHeight) {
                  return QPixmap::fromImage(qt_mac_toQImage(image));
               }
            }
         }
      }
   }
   return QPixmap();
}

void QCocoaNativeInterface::clearCurrentThreadCocoaEventDispatcherInterruptFlag()
{
   QCocoaEventDispatcher::clearCurrentThreadCocoaEventDispatcherInterruptFlag();
}

void QCocoaNativeInterface::onAppFocusWindowChanged(QWindow *window)
{
   (void) window;
   QCocoaMenuBar::updateMenuBarImmediately();
}

#ifndef QT_NO_OPENGL
void *QCocoaNativeInterface::cglContextForContext(QOpenGLContext *context)
{
   NSOpenGLContext *nsOpenGLContext = static_cast<NSOpenGLContext *>(nsOpenGLContextForContext(context));
   if (nsOpenGLContext) {
      return [nsOpenGLContext CGLContextObj];
   }

   return nullptr;
}

void *QCocoaNativeInterface::nsOpenGLContextForContext(QOpenGLContext *context)
{
   if (context) {
      QCocoaGLContext *cocoaGLContext = static_cast<QCocoaGLContext *>(context->handle());

      if (cocoaGLContext) {
         return cocoaGLContext->nsOpenGLContext();
      }
   }

   return nullptr;
}
#endif

QCocoaNativeInterface::FP_Void QCocoaNativeInterface::platformFunction(const QByteArray &function) const
{
   if (function == QCocoaWindowFunctions::bottomLeftClippedByNSWindowOffsetIdentifier()) {
      return FP_Void(QCocoaWindowFunctions::BottomLeftClippedByNSWindowOffset(
               QCocoaWindow::bottomLeftClippedByNSWindowOffsetStatic));
   }

   return nullptr;
}

void QCocoaNativeInterface::addToMimeList(void *macPasteboardMime)
{
   qt_mac_addToGlobalMimeList(reinterpret_cast<QMacInternalPasteboardMime *>(macPasteboardMime));
}

void QCocoaNativeInterface::removeFromMimeList(void *macPasteboardMime)
{
   qt_mac_removeFromGlobalMimeList(reinterpret_cast<QMacInternalPasteboardMime *>(macPasteboardMime));
}

void QCocoaNativeInterface::registerDraggedTypes(const QStringList &types)
{
   qt_mac_registerDraggedTypes(types);
}

void QCocoaNativeInterface::setDockMenu(QPlatformMenu *platformMenu)
{
   QMacAutoReleasePool pool;
   QCocoaMenu *cocoaPlatformMenu = static_cast<QCocoaMenu *>(platformMenu);
   NSMenu *menu = cocoaPlatformMenu->nsMenu();
   [NSApp qt_setDockMenu: menu];
}

void *QCocoaNativeInterface::qMenuToNSMenu(QPlatformMenu *platformMenu)
{
   QCocoaMenu *cocoaPlatformMenu = static_cast<QCocoaMenu *>(platformMenu);
   NSMenu *menu = cocoaPlatformMenu->nsMenu();
   return reinterpret_cast<void *>(menu);
}

void *QCocoaNativeInterface::qMenuBarToNSMenu(QPlatformMenuBar *platformMenuBar)
{
   QCocoaMenuBar *cocoaPlatformMenuBar = static_cast<QCocoaMenuBar *>(platformMenuBar);
   NSMenu *menu = cocoaPlatformMenuBar->nsMenu();
   return reinterpret_cast<void *>(menu);
}

CGImageRef QCocoaNativeInterface::qImageToCGImage(const QImage &image)
{
   return qt_mac_toCGImage(image);
}

QImage QCocoaNativeInterface::cgImageToQImage(CGImageRef image)
{
   return qt_mac_toQImage(image);
}

void QCocoaNativeInterface::setWindowContentView(QPlatformWindow *window, void *contentView)
{
   QCocoaWindow *cocoaPlatformWindow = static_cast<QCocoaWindow *>(window);
   cocoaPlatformWindow->setContentView(reinterpret_cast<NSView *>(contentView));
}

void QCocoaNativeInterface::setEmbeddedInForeignView(QPlatformWindow *window, bool embedded)
{
   QCocoaWindow *cocoaPlatformWindow = static_cast<QCocoaWindow *>(window);
   cocoaPlatformWindow->setEmbeddedInForeignView(embedded);
}

void QCocoaNativeInterface::registerTouchWindow(QWindow *window,  bool enable)
{
   if (!window) {
      return;
   }

   QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
   if (cocoaWindow) {
      cocoaWindow->registerTouch(enable);
   }
}

void QCocoaNativeInterface::setContentBorderThickness(QWindow *window, int topThickness, int bottomThickness)
{
   if (!window) {
      return;
   }

   QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
   if (cocoaWindow) {
      cocoaWindow->setContentBorderThickness(topThickness, bottomThickness);
   }
}

void QCocoaNativeInterface::registerContentBorderArea(QWindow *window, quintptr identifier, int upper, int lower)
{
   if (!window) {
      return;
   }

   QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
   if (cocoaWindow) {
      cocoaWindow->registerContentBorderArea(identifier, upper, lower);
   }
}

void QCocoaNativeInterface::setContentBorderAreaEnabled(QWindow *window, quintptr identifier, bool enable)
{
   if (!window) {
      return;
   }

   QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
   if (cocoaWindow) {
      cocoaWindow->setContentBorderAreaEnabled(identifier, enable);
   }
}

void QCocoaNativeInterface::setContentBorderEnabled(QWindow *window, bool enable)
{
   if (!window) {
      return;
   }

   QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
   if (cocoaWindow) {
      cocoaWindow->setContentBorderEnabled(enable);
   }
}

void QCocoaNativeInterface::setNSToolbar(QWindow *window, void *nsToolbar)
{
   QCocoaIntegration::instance()->setToolbar(window, static_cast<NSToolbar *>(nsToolbar));

   QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
   if (cocoaWindow) {
      cocoaWindow->updateNSToolbar();
   }
}

bool QCocoaNativeInterface::testContentBorderPosition(QWindow *window, int position)
{
   if (!window) {
      return false;
   }

   QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
   if (cocoaWindow) {
      return cocoaWindow->testContentBorderAreaPosition(position);
   }
   return false;
}
