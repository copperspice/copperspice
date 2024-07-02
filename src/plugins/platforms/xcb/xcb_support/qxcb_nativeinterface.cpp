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

#include <qxcb_nativeinterface.h>

#include <qmap.h>
#include <qdebug.h>
#include <qopenglcontext.h>
#include <qscreen.h>
#include <qxcb_screen.h>
#include <qxcb_window.h>
#include <qxcb_integration.h>
#include <qxcb_systemtraytracker.h>
#include <qxcb_nativeinterfacehandler.h>
#include <qxcbscreenfunctions.h>
#include <qxcbintegrationfunctions.h>
#include <qxcbwindowfunctions.h>

#include <qapplication_p.h>

#ifndef QT_NO_DBUS
#include <qdbusmenuconnection_p.h>
#endif

#ifdef XCB_USE_XLIB
#  include <X11/Xlib.h>
#else
#  include <stdio.h>
#endif

#include <algorithm>

// return QXcbNativeInterface::ResourceType for the key.
static int resourceType(const QByteArray &key)
{
   static const QByteArray names[] = {
      // match QXcbNativeInterface::ResourceType

      "display",
      "connection",
      "screen",
      "apptime",
      "appusertime",
      "hintstyle",
      "startupid",
      "traywindow",
      "gettimestamp",
      "x11screen",
      "rootwindow",
      "subpixeltype",
      "antialiasingenabled",
      "nofonthinting",
      "atspibus"
   };

   const QByteArray *end = names + sizeof(names) / sizeof(names[0]);
   const QByteArray *result = std::find(names, end, key);

   return int(result - names);
}

QXcbNativeInterface::QXcbNativeInterface() :
   m_genericEventFilterType("xcb_generic_event_t"),
   m_sysTraySelectionAtom(XCB_ATOM_NONE)
{
}

void QXcbNativeInterface::beep()
{
   QScreen *priScreen = QApplication::primaryScreen();
   if (! priScreen) {
      return;
   }

   QPlatformScreen *screen = priScreen->handle();
   if (!screen) {
      return;
   }
   xcb_connection_t *connection = static_cast<QXcbScreen *>(screen)->xcb_connection();
   xcb_bell(connection, 0);
}

static inline QXcbSystemTrayTracker *systemTrayTracker(const QScreen *s)
{
   if (! s) {
      return nullptr;
   }

   return static_cast<const QXcbScreen *>(s->handle())->connection()->systemTrayTracker();
}

bool QXcbNativeInterface::systemTrayAvailable(const QScreen *screen) const
{
   return systemTrayTracker(screen);
}

bool QXcbNativeInterface::requestSystemTrayWindowDock(const QWindow *window)
{
   return QXcbWindow::requestSystemTrayWindowDockStatic(window);
}

QRect QXcbNativeInterface::systemTrayWindowGlobalGeometry(const QWindow *window)
{
   return QXcbWindow::systemTrayWindowGlobalGeometryStatic(window);
}

xcb_window_t QXcbNativeInterface::locateSystemTray(xcb_connection_t *conn, const QXcbScreen *screen)
{
   if (m_sysTraySelectionAtom == XCB_ATOM_NONE) {
      const QString net_sys_tray = QString("_NET_SYSTEM_TRAY_S%1").formatArg(screen->screenNumber());

      xcb_intern_atom_cookie_t intern_c = xcb_intern_atom_unchecked(conn, true, net_sys_tray.size_storage(), net_sys_tray.constData());
      xcb_intern_atom_reply_t *intern_r = xcb_intern_atom_reply(conn, intern_c, nullptr);

      if (! intern_r) {
         return XCB_WINDOW_NONE;
      }

      m_sysTraySelectionAtom = intern_r->atom;
      free(intern_r);
   }

   xcb_get_selection_owner_cookie_t sel_owner_c = xcb_get_selection_owner_unchecked(conn, m_sysTraySelectionAtom);
   xcb_get_selection_owner_reply_t *sel_owner_r = xcb_get_selection_owner_reply(conn, sel_owner_c, nullptr);

   if (! sel_owner_r) {
      return XCB_WINDOW_NONE;
   }

   xcb_window_t selection_window = sel_owner_r->owner;
   free(sel_owner_r);

   return selection_window;
}

bool QXcbNativeInterface::systrayVisualHasAlphaChannel()
{
   return QXcbConnection::xEmbedSystemTrayVisualHasAlphaChannel();
}

void QXcbNativeInterface::setParentRelativeBackPixmap(QWindow *window)
{
   QXcbWindow::setParentRelativeBackPixmapStatic(window);
}

void *QXcbNativeInterface::nativeResourceForIntegration(const QByteArray &resourceString)
{
   QByteArray lowerCaseResource = resourceString.toLower();
   void *result = handlerNativeResourceForIntegration(lowerCaseResource);

   if (result) {
      return result;
   }

   switch (resourceType(lowerCaseResource)) {
      case StartupId:
         result = startupId();
         break;

      case X11Screen:
         result = x11Screen();
         break;

      case RootWindow:
         result = rootWindow();
         break;

      case Display:
         result = display();
         break;

      case AtspiBus:
         result = atspiBus();
         break;

      case Connection:
         result = connection();
         break;

      default:
         break;
   }

   return result;
}

void *QXcbNativeInterface::nativeResourceForContext(const QByteArray &resourceString, QOpenGLContext *context)
{
   QByteArray lowerCaseResource = resourceString.toLower();
   void *result = handlerNativeResourceForContext(lowerCaseResource, context);
   return result;
}

void *QXcbNativeInterface::nativeResourceForScreen(const QByteArray &resourceString, QScreen *screen)
{
   if (!screen) {
      qWarning("QXcbNativeInterface::nativeResourceForContext() Screen is invalid (nullptr)");
      return nullptr;
   }

   QByteArray lowerCaseResource = resourceString.toLower();
   void *result = handlerNativeResourceForScreen(lowerCaseResource, screen);

   if (result) {
      return result;
   }

   const QXcbScreen *xcbScreen = static_cast<QXcbScreen *>(screen->handle());

   switch (resourceType(lowerCaseResource)) {
      case Display:
#ifdef XCB_USE_XLIB
         result = xcbScreen->connection()->xlib_display();
#endif
         break;

      case AppTime:
         result = appTime(xcbScreen);
         break;

      case AppUserTime:
         result = appUserTime(xcbScreen);
         break;

      case ScreenHintStyle:
         result = reinterpret_cast<void *>(xcbScreen->hintStyle() + 1);
         break;

      case ScreenSubpixelType:
         result = reinterpret_cast<void *>(xcbScreen->subpixelType() + 1);
         break;

      case ScreenAntialiasingEnabled:
         result = reinterpret_cast<void *>(xcbScreen->antialiasingEnabled() + 1);
         break;

      case TrayWindow:
         if (QXcbSystemTrayTracker *s = systemTrayTracker(screen)) {
            result = (void *)quintptr(s->trayWindow());
         }
         break;

      case GetTimestamp:
         result = getTimestamp(xcbScreen);
         break;

      case NoFontHinting:
         result = xcbScreen->noFontHinting() ? this : nullptr;
         break;

      case RootWindow:
         result = reinterpret_cast<void *>(xcbScreen->root());
         break;

      default:
         break;
   }

   return result;
}

void *QXcbNativeInterface::nativeResourceForWindow(const QByteArray &resourceString, QWindow *window)
{
   QByteArray lowerCaseResource = resourceString.toLower();
   void *result = handlerNativeResourceForWindow(lowerCaseResource, window);

   if (result) {
      return result;
   }

   switch (resourceType(lowerCaseResource)) {
      case Display:
         result = displayForWindow(window);
         break;

      case Connection:
         result = connectionForWindow(window);
         break;

      case Screen:
         result = screenForWindow(window);
         break;

      default:
         break;
   }

   return result;
}

void *QXcbNativeInterface::nativeResourceForBackingStore(const QByteArray &resourceString, QBackingStore *backingStore)
{
   const QByteArray lowerCaseResource = resourceString.toLower();
   void *result = handlerNativeResourceForBackingStore(lowerCaseResource, backingStore);

   return result;
}

QPlatformNativeInterface::FP_Integration QXcbNativeInterface::nativeResourceFunctionForIntegration(
   const QByteArray &resource)
{
   const QByteArray lowerCaseResource = resource.toLower();
   QPlatformNativeInterface::FP_Integration func = handlerNativeResourceFunctionForIntegration(lowerCaseResource);

   if (func) {
      return func;
   }

   if (lowerCaseResource == "setstartupid") {
      return FP_Integration(setStartupId);
   }

   return nullptr;
}

QPlatformNativeInterface::FP_Context QXcbNativeInterface::nativeResourceFunctionForContext(
   const QByteArray &resource)
{
   const QByteArray lowerCaseResource = resource.toLower();
   QPlatformNativeInterface::FP_Context func = handlerNativeResourceFunctionForContext(lowerCaseResource);

   if (func) {
      return func;
   }

   return nullptr;
}

QPlatformNativeInterface::FP_Screen QXcbNativeInterface::nativeResourceFunctionForScreen(
   const QByteArray &resource)
{
   const QByteArray lowerCaseResource = resource.toLower();
   FP_Screen func = handlerNativeResourceFunctionForScreen(lowerCaseResource);

   if (func) {
      return func;
   }

   if (lowerCaseResource == "setapptime") {
      return FP_Screen(setAppTime);

   } else if (lowerCaseResource == "setappusertime") {
      return FP_Screen(setAppUserTime);

   }

   return nullptr;
}

QPlatformNativeInterface::FP_Window QXcbNativeInterface::nativeResourceFunctionForWindow(
   const QByteArray &resource)
{
   const QByteArray lowerCaseResource = resource.toLower();
   FP_Window func = handlerNativeResourceFunctionForWindow(lowerCaseResource);

   return func;
}

QPlatformNativeInterface::FP_BackingStore QXcbNativeInterface::nativeResourceFunctionForBackingStore(
   const QByteArray &resource)
{
   const QByteArray lowerCaseResource = resource.toLower();
   FP_BackingStore func = handlerNativeResourceFunctionForBackingStore(resource);

   return func;
}

QXcbNativeInterface::FP_Void QXcbNativeInterface::platformFunction(const QByteArray &function) const
{
   const QByteArray lowerCaseFunction = function.toLower();
   FP_Void func = handlerPlatformFunction(lowerCaseFunction);

   if (func) {
      return func;
   }

   //case sensitive
   if (function == QXcbWindowFunctions::setWmWindowTypeIdentifier()) {
      return FP_Void(QXcbWindowFunctions::SetWmWindowType(QXcbWindow::setWmWindowTypeStatic));
   }

   if (function == QXcbWindowFunctions::setWmWindowRoleIdentifier()) {
      return FP_Void(QXcbWindowFunctions::SetWmWindowRole(QXcbWindow::setWmWindowRoleStatic));
   }

   if (function == QXcbWindowFunctions::setWmWindowIconTextIdentifier()) {
      return FP_Void(QXcbWindowFunctions::SetWmWindowIconText(QXcbWindow::setWindowIconTextStatic));
   }

   if (function == QXcbWindowFunctions::setParentRelativeBackPixmapIdentifier()) {
      return FP_Void(QXcbWindowFunctions::SetParentRelativeBackPixmap(QXcbWindow::setParentRelativeBackPixmapStatic));
   }

   if (function == QXcbWindowFunctions::requestSystemTrayWindowDockIdentifier()) {
      return FP_Void(QXcbWindowFunctions::RequestSystemTrayWindowDock(QXcbWindow::requestSystemTrayWindowDockStatic));
   }

   if (function == QXcbWindowFunctions::systemTrayWindowGlobalGeometryIdentifier()) {
      return FP_Void(QXcbWindowFunctions::SystemTrayWindowGlobalGeometry(QXcbWindow::systemTrayWindowGlobalGeometryStatic));
   }

   if (function == QXcbIntegrationFunctions::xEmbedSystemTrayVisualHasAlphaChannelIdentifier()) {
      return FP_Void(QXcbIntegrationFunctions::XEmbedSystemTrayVisualHasAlphaChannel(
               QXcbConnection::xEmbedSystemTrayVisualHasAlphaChannel));
   }

   if (function == QXcbWindowFunctions::visualIdIdentifier()) {
      return FP_Void(QXcbWindowFunctions::VisualId(QXcbWindow::visualIdStatic));
   }

   if (function == QXcbScreenFunctions::virtualDesktopNumberIdentifier()) {
      return FP_Void(QXcbScreenFunctions::VirtualDesktopNumber(QXcbScreen::virtualDesktopNumberStatic));
   }

   return nullptr;
}

void *QXcbNativeInterface::appTime(const QXcbScreen *screen)
{
   if (! screen) {
      return nullptr;
   }

   return reinterpret_cast<void *>(quintptr(screen->connection()->time()));
}

void *QXcbNativeInterface::appUserTime(const QXcbScreen *screen)
{
   if (! screen) {
      return nullptr;
   }

   return reinterpret_cast<void *>(quintptr(screen->connection()->netWmUserTime()));
}

void *QXcbNativeInterface::getTimestamp(const QXcbScreen *screen)
{
   if (! screen) {
      return nullptr;
   }

   return reinterpret_cast<void *>(quintptr(screen->connection()->getTimestamp()));
}

void *QXcbNativeInterface::startupId()
{
   QXcbIntegration *integration = QXcbIntegration::instance();
   QXcbConnection *defaultConnection = integration->defaultConnection();

   if (defaultConnection) {
      return reinterpret_cast<void *>(const_cast<char *>(defaultConnection->startupId().constData()));
   }

   return nullptr;
}

void *QXcbNativeInterface::x11Screen()
{
   QXcbIntegration *integration = QXcbIntegration::instance();
   QXcbConnection *defaultConnection = integration->defaultConnection();

   if (defaultConnection) {
      return reinterpret_cast<void *>(defaultConnection->primaryScreenNumber());
   }

   return nullptr;
}

void *QXcbNativeInterface::rootWindow()
{
   QXcbIntegration *integration = QXcbIntegration::instance();
   QXcbConnection *defaultConnection = integration->defaultConnection();

   if (defaultConnection) {
      return reinterpret_cast<void *>(defaultConnection->rootWindow());
   }

   return nullptr;
}

void *QXcbNativeInterface::display()
{
#ifdef XCB_USE_XLIB
   QXcbIntegration *integration = QXcbIntegration::instance();
   QXcbConnection *defaultConnection = integration->defaultConnection();

   if (defaultConnection) {
      return defaultConnection->xlib_display();
   }
#endif

   return nullptr;
}

void *QXcbNativeInterface::connection()
{
   QXcbIntegration *integration = QXcbIntegration::instance();
   return integration->defaultConnection()->xcb_connection();
}

void *QXcbNativeInterface::atspiBus()
{
   QXcbIntegration *integration = static_cast<QXcbIntegration *>(QApplicationPrivate::platformIntegration());
   QXcbConnection *defaultConnection = integration->defaultConnection();

   if (defaultConnection != nullptr) {
      xcb_atom_t atspiBusAtom = defaultConnection->internAtom("AT_SPI_BUS");
      xcb_get_property_cookie_t cookie = Q_XCB_CALL2(xcb_get_property(defaultConnection->xcb_connection(), false,
               defaultConnection->rootWindow(), atspiBusAtom, XCB_ATOM_STRING, 0, 128), defaultConnection);

      xcb_get_property_reply_t *reply = Q_XCB_CALL2(xcb_get_property_reply(defaultConnection->xcb_connection(), cookie, nullptr), defaultConnection);

      Q_ASSERT(!reply->bytes_after);
      char *data = (char *)xcb_get_property_value(reply);

      int length = xcb_get_property_value_length(reply);
      QByteArray *busAddress = new QByteArray(data, length);
      free(reply);

      return busAddress;
   }

   return nullptr;
}

void QXcbNativeInterface::setAppTime(QScreen *screen, xcb_timestamp_t time)
{
   if (screen) {
      static_cast<QXcbScreen *>(screen->handle())->connection()->setTime(time);
   }
}

void QXcbNativeInterface::setAppUserTime(QScreen *screen, xcb_timestamp_t time)
{
   if (screen) {
      static_cast<QXcbScreen *>(screen->handle())->connection()->setNetWmUserTime(time);
   }
}

void QXcbNativeInterface::setStartupId(const char *data)
{
   QByteArray startupId(data);
   QXcbIntegration *integration = QXcbIntegration::instance();
   QXcbConnection *defaultConnection = integration->defaultConnection();

   if (defaultConnection) {
      defaultConnection->setStartupId(startupId);
   }
}

QXcbScreen *QXcbNativeInterface::qPlatformScreenForWindow(QWindow *window)
{
   QXcbScreen *screen;

   if (window) {
      QScreen *qs = window->screen();
      screen = static_cast<QXcbScreen *>(qs ? qs->handle() : nullptr);
   } else {
      QScreen *qs = QApplication::primaryScreen();
      screen = static_cast<QXcbScreen *>(qs ? qs->handle() : nullptr);
   }
   return screen;
}

void *QXcbNativeInterface::displayForWindow(QWindow *window)
{
#if defined(XCB_USE_XLIB)
   QXcbScreen *screen = qPlatformScreenForWindow(window);
   return screen ? screen->connection()->xlib_display() : nullptr;
#else
   return nullptr;
#endif
}

void *QXcbNativeInterface::connectionForWindow(QWindow *window)
{
   QXcbScreen *screen = qPlatformScreenForWindow(window);
   return screen ? screen->xcb_connection() : nullptr;
}

void *QXcbNativeInterface::screenForWindow(QWindow *window)
{
   QXcbScreen *screen = qPlatformScreenForWindow(window);
   return screen ? screen->screen() : nullptr;
}

void QXcbNativeInterface::addHandler(QXcbNativeInterfaceHandler *handler)
{
   m_handlers.removeAll(handler);
   m_handlers.prepend(handler);
}

void QXcbNativeInterface::removeHandler(QXcbNativeInterfaceHandler *handler)
{
   m_handlers.removeAll(handler);
}

QPlatformNativeInterface::FP_Integration QXcbNativeInterface::handlerNativeResourceFunctionForIntegration(
   const QByteArray &resource) const
{
   for (int i = 0; i < m_handlers.size(); i++) {
      QXcbNativeInterfaceHandler *handler = m_handlers.at(i);
      FP_Integration result = handler->nativeResourceFunctionForIntegration(resource);

      if (result) {
         return result;
      }
   }

   return nullptr;
}

QPlatformNativeInterface::FP_Context QXcbNativeInterface::handlerNativeResourceFunctionForContext(const QByteArray &resource) const
{
   for (int i = 0; i < m_handlers.size(); i++) {
      QXcbNativeInterfaceHandler *handler = m_handlers.at(i);
      FP_Context result = handler->nativeResourceFunctionForContext(resource);

      if (result) {
         return result;
      }
   }
   return nullptr;
}

QPlatformNativeInterface::FP_Screen QXcbNativeInterface::handlerNativeResourceFunctionForScreen(const QByteArray &resource) const
{
   for (int i = 0; i < m_handlers.size(); i++) {
      QXcbNativeInterfaceHandler *handler = m_handlers.at(i);
      FP_Screen result = handler->nativeResourceFunctionForScreen(resource);

      if (result) {
         return result;
      }
   }
   return nullptr;
}

QPlatformNativeInterface::FP_Window QXcbNativeInterface::handlerNativeResourceFunctionForWindow(const QByteArray &resource) const
{
   for (int i = 0; i < m_handlers.size(); i++) {
      QXcbNativeInterfaceHandler *handler = m_handlers.at(i);
      FP_Window result = handler->nativeResourceFunctionForWindow(resource);

      if (result) {
         return result;
      }
   }

   return nullptr;
}

QPlatformNativeInterface::FP_BackingStore QXcbNativeInterface::handlerNativeResourceFunctionForBackingStore(
   const QByteArray &resource) const
{
   for (int i = 0; i < m_handlers.size(); i++) {
      QXcbNativeInterfaceHandler *handler = m_handlers.at(i);
      FP_BackingStore result = handler->nativeResourceFunctionForBackingStore(resource);

      if (result) {
         return result;
      }
   }

   return nullptr;
}

QXcbNativeInterface::FP_Void QXcbNativeInterface::handlerPlatformFunction(const QByteArray &function) const
{
   for (int i = 0; i < m_handlers.size(); i++) {
      QXcbNativeInterfaceHandler *handler = m_handlers.at(i);
      FP_Void func = handler->platformFunction(function);

      if (func) {
         return func;
      }
   }
   return nullptr;
}

void *QXcbNativeInterface::handlerNativeResourceForIntegration(const QByteArray &resource) const
{
   FP_Integration func = handlerNativeResourceFunctionForIntegration(resource);

   if (func) {
      return func();
   }

   return nullptr;
}

void *QXcbNativeInterface::handlerNativeResourceForContext(const QByteArray &resource, QOpenGLContext *context) const
{
   FP_Context func = handlerNativeResourceFunctionForContext(resource);

   if (func) {
      return func(context);
   }

   return nullptr;
}

void *QXcbNativeInterface::handlerNativeResourceForScreen(const QByteArray &resource, QScreen *screen) const
{
   FP_Screen func = handlerNativeResourceFunctionForScreen(resource);

   if (func) {
      return func(screen);
   }

   return nullptr;
}

void *QXcbNativeInterface::handlerNativeResourceForWindow(const QByteArray &resource, QWindow *window) const
{
   FP_Window func = handlerNativeResourceFunctionForWindow(resource);

   if (func) {
      return func(window);
   }

   return nullptr;
}

void *QXcbNativeInterface::handlerNativeResourceForBackingStore(const QByteArray &resource, QBackingStore *backingStore) const
{
   FP_BackingStore func = handlerNativeResourceFunctionForBackingStore(resource);

   if (func) {
      return func(backingStore);
   }

   return nullptr;
}

