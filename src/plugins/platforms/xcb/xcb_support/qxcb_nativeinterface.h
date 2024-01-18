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

#ifndef QXCB_NATIVEINTERFACE_H
#define QXCB_NATIVEINTERFACE_H

#include <qplatform_nativeinterface.h>
#include <qrect.h>
#include <qxcb_export.h>

#include <xcb/xcb.h>

class QWidget;
class QXcbScreen;
class QXcbConnection;
class QXcbNativeInterfaceHandler;
class QDBusMenuConnection;

class Q_XCB_EXPORT QXcbNativeInterface : public QPlatformNativeInterface
{
   CS_OBJECT(QXcbNativeInterface)

 public:
   enum ResourceType {
      Display,
      Connection,
      Screen,
      AppTime,
      AppUserTime,
      ScreenHintStyle,
      StartupId,
      TrayWindow,
      GetTimestamp,
      X11Screen,
      RootWindow,
      ScreenSubpixelType,
      ScreenAntialiasingEnabled,
      NoFontHinting,
      AtspiBus
   };

   using FP_Void = void(*)();

   QXcbNativeInterface();

   void *nativeResourceForIntegration(const QByteArray &resource) override;
   void *nativeResourceForContext(const QByteArray &resourceString, QOpenGLContext *context) override;
   void *nativeResourceForScreen(const QByteArray &resource, QScreen *screen) override;
   void *nativeResourceForWindow(const QByteArray &resourceString, QWindow *window) override;
   void *nativeResourceForBackingStore(const QByteArray &resource, QBackingStore *backingStore) override;

   FP_Integration nativeResourceFunctionForIntegration(const QByteArray &resource) override;
   FP_Context nativeResourceFunctionForContext(const QByteArray &resource) override;
   FP_Screen nativeResourceFunctionForScreen(const QByteArray &resource) override;
   FP_Window nativeResourceFunctionForWindow(const QByteArray &resource) override;
   FP_BackingStore nativeResourceFunctionForBackingStore(const QByteArray &resource) override;

   FP_Void platformFunction(const QByteArray &function) const override;

   inline const QByteArray &genericEventFilterType() const {
      return m_genericEventFilterType;
   }

   void *displayForWindow(QWindow *window);
   void *connectionForWindow(QWindow *window);
   void *screenForWindow(QWindow *window);
   void *appTime(const QXcbScreen *screen);
   void *appUserTime(const QXcbScreen *screen);
   void *getTimestamp(const QXcbScreen *screen);
   void *startupId();
   void *x11Screen();
   void *rootWindow();
   void *display();
   void *atspiBus();
   void *connection();

   static void setStartupId(const char *);
   static void setAppTime(QScreen *screen, xcb_timestamp_t time);
   static void setAppUserTime(QScreen *screen, xcb_timestamp_t time);

   void beep();
   bool systemTrayAvailable(const QScreen *screen) const;
   void setParentRelativeBackPixmap(QWindow *window);
   bool systrayVisualHasAlphaChannel();
   bool requestSystemTrayWindowDock(const QWindow *window);
   QRect systemTrayWindowGlobalGeometry(const QWindow *window);

   void addHandler(QXcbNativeInterfaceHandler *handler);
   void removeHandler(QXcbNativeInterfaceHandler *handler);

   CS_SIGNAL_1(Public, void systemTrayWindowChanged(QScreen *screen))
   CS_SIGNAL_2(systemTrayWindowChanged, screen)

 private:
   xcb_window_t locateSystemTray(xcb_connection_t *conn, const QXcbScreen *screen);

   const QByteArray m_genericEventFilterType;

   xcb_atom_t m_sysTraySelectionAtom;

   static QXcbScreen *qPlatformScreenForWindow(QWindow *window);

   QList<QXcbNativeInterfaceHandler *> m_handlers;
   FP_Integration handlerNativeResourceFunctionForIntegration(const QByteArray &resource) const;
   FP_Context handlerNativeResourceFunctionForContext(const QByteArray &resource) const;
   FP_Screen handlerNativeResourceFunctionForScreen(const QByteArray &resource) const;
   FP_Window handlerNativeResourceFunctionForWindow(const QByteArray &resource) const;
   FP_BackingStore handlerNativeResourceFunctionForBackingStore(const QByteArray &resource) const;

   FP_Void handlerPlatformFunction(const QByteArray &function) const;

   void *handlerNativeResourceForIntegration(const QByteArray &resource) const;
   void *handlerNativeResourceForContext(const QByteArray &resource, QOpenGLContext *context) const;
   void *handlerNativeResourceForScreen(const QByteArray &resource, QScreen *screen) const;
   void *handlerNativeResourceForWindow(const QByteArray &resource, QWindow *window) const;
   void *handlerNativeResourceForBackingStore(const QByteArray &resource, QBackingStore *backingStore) const;
};

#endif
