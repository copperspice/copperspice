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

#include "qcocoanativeinterface.h"
#include "qcocoawindow.h"
#include "qcocoamenu.h"
#include "qcocoamenubar.h"
#include "qcocoahelpers.h"
#include "qcocoaapplication.h"
#include "qcocoaintegration.h"
#include "qcocoaeventdispatcher.h"

#include <qbytearray.h>
#include <qwindow.h>
#include <qpixmap.h>
#include <qpa/qplatformwindow.h>
#include "qsurfaceformat.h"
#ifndef QT_NO_OPENGL
#include <qpa/qplatformopenglcontext.h>
#include "qopenglcontext.h"
#include "qcocoaglcontext.h"
#endif
#include "qguiapplication.h"
#include <qdebug.h>

#ifndef QT_NO_WIDGETS
#include "qcocoaprintersupport.h"
#include "qprintengine_mac_p.h"
#include <qpa/qplatformprintersupport.h>
#endif

#include <QtPlatformHeaders/qcocoawindowfunctions.h>

#include <Cocoa/Cocoa.h>

QT_BEGIN_NAMESPACE

QCocoaNativeInterface::QCocoaNativeInterface()
{
}

#ifndef QT_NO_OPENGL
void *QCocoaNativeInterface::nativeResourceForContext(const QByteArray &resourceString, QOpenGLContext *context)
{
    if (!context)
        return 0;
    if (resourceString.toLower() == "nsopenglcontext")
        return nsOpenGLContextForContext(context);
    if (resourceString.toLower() == "cglcontextobj")
        return cglContextForContext(context);

    return 0;
}
#endif

void *QCocoaNativeInterface::nativeResourceForWindow(const QByteArray &resourceString, QWindow *window)
{
    if (!window->handle())
        return 0;

    if (resourceString == "nsview") {
        return static_cast<QCocoaWindow *>(window->handle())->m_contentView;
#ifndef QT_NO_OPENGL
    } else if (resourceString == "nsopenglcontext") {
        return static_cast<QCocoaWindow *>(window->handle())->currentContext()->nsOpenGLContext();
#endif
    } else if (resourceString == "nswindow") {
        return static_cast<QCocoaWindow *>(window->handle())->m_nsWindow;
    }
    return 0;
}

QPlatformNativeInterface::NativeResourceForIntegrationFunction QCocoaNativeInterface::nativeResourceFunctionForIntegration(const QByteArray &resource)
{
    if (resource.toLower() == "addtomimelist")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::addToMimeList);
    if (resource.toLower() == "removefrommimelist")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::removeFromMimeList);
    if (resource.toLower() == "registerdraggedtypes")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::registerDraggedTypes);
    if (resource.toLower() == "setdockmenu")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::setDockMenu);
    if (resource.toLower() == "qmenutonsmenu")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::qMenuToNSMenu);
    if (resource.toLower() == "qmenubartonsmenu")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::qMenuBarToNSMenu);
    if (resource.toLower() == "qimagetocgimage")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::qImageToCGImage);
    if (resource.toLower() == "cgimagetoqimage")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::cgImageToQImage);
    if (resource.toLower() == "setwindowcontentview")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::setWindowContentView);
    if (resource.toLower() == "registertouchwindow")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::registerTouchWindow);
    if (resource.toLower() == "setembeddedinforeignview")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::setEmbeddedInForeignView);
    if (resource.toLower() == "setcontentborderthickness")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::setContentBorderThickness);
    if (resource.toLower() == "registercontentborderarea")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::registerContentBorderArea);
    if (resource.toLower() == "setcontentborderareaenabled")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::setContentBorderAreaEnabled);
    if (resource.toLower() == "setcontentborderenabled")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::setContentBorderEnabled);
    if (resource.toLower() == "setnstoolbar")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::setNSToolbar);
    if (resource.toLower() == "testcontentborderposition")
        return NativeResourceForIntegrationFunction(QCocoaNativeInterface::testContentBorderPosition);

    return 0;
}

void QCocoaNativeInterface::beep()
{
    NSBeep();
}

QPlatformPrinterSupport *QCocoaNativeInterface::createPlatformPrinterSupport()
{
#if !defined(QT_NO_WIDGETS) && !defined(QT_NO_PRINTER)
    return new QCocoaPrinterSupport();
#else
    qFatal("Printing is not supported when Qt is configured with -no-widgets");
    return 0;
#endif
}

void *QCocoaNativeInterface::NSPrintInfoForPrintEngine(QPrintEngine *printEngine)
{
#if !defined(QT_NO_WIDGETS) && !defined(QT_NO_PRINTER)
    QMacPrintEnginePrivate *macPrintEnginePriv = static_cast<QMacPrintEngine *>(printEngine)->d_func();
    if (macPrintEnginePriv->state == QPrinter::Idle && !macPrintEnginePriv->isPrintSessionInitialized())
        macPrintEnginePriv->initialize();
    return macPrintEnginePriv->printInfo;
#else
    Q_UNUSED(printEngine);
    qFatal("Printing is not supported when Qt is configured with -no-widgets");
    return 0;
#endif
}

QPixmap QCocoaNativeInterface::defaultBackgroundPixmapForQWizard()
{
    QCFType<CFURLRef> url;
    const int ExpectedImageWidth = 242;
    const int ExpectedImageHeight = 414;
    if (LSFindApplicationForInfo(kLSUnknownCreator, CFSTR("com.apple.KeyboardSetupAssistant"),
                                 0, 0, &url) == noErr) {
        QCFType<CFBundleRef> bundle = CFBundleCreate(kCFAllocatorDefault, url);
        if (bundle) {
            url = CFBundleCopyResourceURL(bundle, CFSTR("Background"), CFSTR("png"), 0);
            if (url) {
                QCFType<CGImageSourceRef> imageSource = CGImageSourceCreateWithURL(url, 0);
                QCFType<CGImageRef> image = CGImageSourceCreateImageAtIndex(imageSource, 0, 0);
                if (image) {
                    int width = CGImageGetWidth(image);
                    int height = CGImageGetHeight(image);
                    if (width == ExpectedImageWidth && height == ExpectedImageHeight)
                        return QPixmap::fromImage(qt_mac_toQImage(image));
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
    Q_UNUSED(window);
    QCocoaMenuBar::updateMenuBarImmediately();
}

#ifndef QT_NO_OPENGL
void *QCocoaNativeInterface::cglContextForContext(QOpenGLContext* context)
{
    NSOpenGLContext *nsOpenGLContext = static_cast<NSOpenGLContext*>(nsOpenGLContextForContext(context));
    if (nsOpenGLContext)
        return [nsOpenGLContext CGLContextObj];
    return 0;
}

void *QCocoaNativeInterface::nsOpenGLContextForContext(QOpenGLContext* context)
{
    if (context) {
        QCocoaGLContext *cocoaGLContext = static_cast<QCocoaGLContext *>(context->handle());
        if (cocoaGLContext) {
            return cocoaGLContext->nsOpenGLContext();
        }
    }
    return 0;
}
#endif

QFunctionPointer QCocoaNativeInterface::platformFunction(const QByteArray &function) const
{
    if (function == QCocoaWindowFunctions::bottomLeftClippedByNSWindowOffsetIdentifier())
        return QFunctionPointer(QCocoaWindowFunctions::BottomLeftClippedByNSWindowOffset(QCocoaWindow::bottomLeftClippedByNSWindowOffsetStatic));

    return Q_NULLPTR;
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
    [NSApp QT_MANGLE_NAMESPACE(qt_setDockMenu): menu];
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
    if (!window)
        return;

    QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
    if (cocoaWindow)
        cocoaWindow->registerTouch(enable);
}

void QCocoaNativeInterface::setContentBorderThickness(QWindow *window, int topThickness, int bottomThickness)
{
    if (!window)
        return;

    QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
    if (cocoaWindow)
        cocoaWindow->setContentBorderThickness(topThickness, bottomThickness);
}

void QCocoaNativeInterface::registerContentBorderArea(QWindow *window, quintptr identifier, int upper, int lower)
{
    if (!window)
        return;

    QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
    if (cocoaWindow)
        cocoaWindow->registerContentBorderArea(identifier, upper, lower);
}

void QCocoaNativeInterface::setContentBorderAreaEnabled(QWindow *window, quintptr identifier, bool enable)
{
    if (!window)
        return;

    QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
    if (cocoaWindow)
        cocoaWindow->setContentBorderAreaEnabled(identifier, enable);
}

void QCocoaNativeInterface::setContentBorderEnabled(QWindow *window, bool enable)
{
    if (!window)
        return;

    QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
    if (cocoaWindow)
        cocoaWindow->setContentBorderEnabled(enable);
}

void QCocoaNativeInterface::setNSToolbar(QWindow *window, void *nsToolbar)
{
    QCocoaIntegration::instance()->setToolbar(window, static_cast<NSToolbar *>(nsToolbar));

    QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
    if (cocoaWindow)
        cocoaWindow->updateNSToolbar();
}

bool QCocoaNativeInterface::testContentBorderPosition(QWindow *window, int position)
{
    if (!window)
        return false;

    QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
    if (cocoaWindow)
        return cocoaWindow->testContentBorderAreaPosition(position);
    return false;
}

QT_END_NAMESPACE
