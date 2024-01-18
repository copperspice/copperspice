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

#ifndef QCOCOANATIVEINTERFACE_H
#define QCOCOANATIVEINTERFACE_H

#include <ApplicationServices/ApplicationServices.h>

#include <qplatform_nativeinterface.h>
#include <qpixmap.h>

class QWidget;
class QPlatformPrinterSupport;
class QPrintEngine;
class QPlatformMenu;
class QPlatformMenuBar;

class QCocoaNativeInterface : public QPlatformNativeInterface
{
   CS_OBJECT(QCocoaNativeInterface)

 public:
   QCocoaNativeInterface();

#ifndef QT_NO_OPENGL
   void *nativeResourceForContext(const QByteArray &resourceString, QOpenGLContext *context) override;
#endif
   void *nativeResourceForWindow(const QByteArray &resourceString, QWindow *window) override;

   FP_Integration nativeResourceFunctionForIntegration(const QByteArray &resource) override;

   void beep();

#ifndef QT_NO_OPENGL
   static void *cglContextForContext(QOpenGLContext *context);
   static void *nsOpenGLContextForContext(QOpenGLContext *context);
#endif

   FP_Void platformFunction(const QByteArray &function) const override;

   CS_SLOT_1(Public, void onAppFocusWindowChanged(QWindow *window))
   CS_SLOT_2(onAppFocusWindowChanged)

 private:
   /*
       "Virtual" function to create the platform printer support
       implementation.

       We use an invokable function instead of a virtual one, we do not want
       this in the QPlatform* API yet.

       This was added here only because QPlatformNativeInterface is a QObject
       and allow us to use QMetaObject::indexOfMethod() from the printsupport
       plugin.
   */
   CS_INVOKABLE_METHOD_1(Private, QPlatformPrinterSupport *createPlatformPrinterSupport())
   CS_INVOKABLE_METHOD_2(createPlatformPrinterSupport)

   /*
       Function to return the NSPrintInfo * from QMacPaintEnginePrivate.
       Needed by the native print dialog in the Print Support module.
   */
   CS_INVOKABLE_METHOD_1(Private, void *NSPrintInfoForPrintEngine(QPrintEngine *printEngine))
   CS_INVOKABLE_METHOD_2(NSPrintInfoForPrintEngine)

   /*
       Function to return the default background pixmap.
       Needed by QWizard
   */
   CS_INVOKABLE_METHOD_1(Private, QPixmap defaultBackgroundPixmapForQWizard())
   CS_INVOKABLE_METHOD_2(defaultBackgroundPixmapForQWizard)

   CS_INVOKABLE_METHOD_1(Private, void clearCurrentThreadCocoaEventDispatcherInterruptFlag())
   CS_INVOKABLE_METHOD_2(clearCurrentThreadCocoaEventDispatcherInterruptFlag)

   // QMacPastebardMime support. The mac pasteboard void pointers are
   // QMacPastebardMime instances from the cocoa plugin or qtmacextras
   // These two classes are kept in sync and can be casted between.
   static void addToMimeList(void *macPasteboardMime);
   static void removeFromMimeList(void *macPasteboardMime);
   static void registerDraggedTypes(const QStringList &types);

   // Dock menu support
   static void setDockMenu(QPlatformMenu *platformMenu);

   // Function to return NSMenu * from QPlatformMenu
   static void *qMenuToNSMenu(QPlatformMenu *platformMenu);

   // Function to return NSMenu * from QPlatformMenuBar
   static void *qMenuBarToNSMenu(QPlatformMenuBar *platformMenuBar);

   // QImage <-> CGImage conversion functions
   static CGImageRef qImageToCGImage(const QImage &image);
   static QImage cgImageToQImage(CGImageRef image);

   // Embedding NSViews as child QWindows
   static void setWindowContentView(QPlatformWindow *window, void *nsViewContentView);

   // Set a QWindow as a "guest" (subwindow) of a non-QWindow
   static void setEmbeddedInForeignView(QPlatformWindow *window, bool embedded);

   // Register if a window should deliver touch events. Enabling
   // touch events has implications for delivery of other events,
   // for example by causing scrolling event lag.
   //
   // The registration is ref-counted: multiple widgets can enable
   // touch events, which then will be delivered until the widget
   // deregisters.
   static void registerTouchWindow(QWindow *window,  bool enable);

   // Enable the unified title and toolbar area for a window.
   static void setContentBorderEnabled(QWindow *window, bool enable);

   // Set the size of the unified title and toolbar area.
   static void setContentBorderThickness(QWindow *window, int topThickness, int bottomThickness);

   // Set the size for a unified toolbar content border area.
   // Multiple callers can register areas and the platform plugin
   // will extend the "unified" area to cover them.
   static void registerContentBorderArea(QWindow *window, quintptr identifer, int upper, int lower);

   // Enables or disiables a content border area.
   static void setContentBorderAreaEnabled(QWindow *window, quintptr identifier, bool enable);

   // Returns true if the given coordinate is inside the current
   // content border.
   static bool testContentBorderPosition(QWindow *window, int position);

   // Sets a NSToolbar instance for the given QWindow. The
   // toolbar will be attached to the native NSWindow when
   // that is created;
   static void setNSToolbar(QWindow *window, void *nsToolbar);

};

#endif
