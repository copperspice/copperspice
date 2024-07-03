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

#include <qxcb_systemtraytracker.h>

#include <qxcb_connection.h>
#include <qxcb_screen.h>
#include <qdebug.h>
#include <qrect.h>
#include <qscreen.h>

#include <qplatform_nativeinterface.h>

static constexpr const int SystemTrayRequestDock   = 0;
static constexpr const int SystemTrayBeginMessage  = 1;
static constexpr const int SystemTrayCancelMessage = 2;

// QXcbSystemTrayTracker provides API for accessing the tray window and tracks
// its lifecyle by listening for its destruction and recreation.
// See http://standards.freedesktop.org/systemtray-spec/systemtray-spec-latest.html

QXcbSystemTrayTracker *QXcbSystemTrayTracker::create(QXcbConnection *connection)
{
   // Selection, tray atoms for GNOME, NET WM Specification
   const xcb_atom_t trayAtom = connection->atom(QXcbAtom::_NET_SYSTEM_TRAY_OPCODE);
   if (! trayAtom) {
      return nullptr;
   }

   const QByteArray netSysTray = "_NET_SYSTEM_TRAY_S" + QByteArray::number(connection->primaryScreenNumber());
   const xcb_atom_t selection = connection->internAtom(netSysTray.constData());

   if (!selection) {
      return nullptr;
   }

   return new QXcbSystemTrayTracker(connection, trayAtom, selection);
}

QXcbSystemTrayTracker::QXcbSystemTrayTracker(QXcbConnection *connection, xcb_atom_t trayAtom, xcb_atom_t selection)
   : QObject(connection), m_selection(selection), m_trayAtom(trayAtom),
     m_connection(connection), m_trayWindow(0)
{
}

xcb_window_t QXcbSystemTrayTracker::locateTrayWindow(const QXcbConnection *connection, xcb_atom_t selection)
{
   xcb_get_selection_owner_cookie_t cookie = xcb_get_selection_owner(connection->xcb_connection(), selection);
   xcb_get_selection_owner_reply_t *reply  = xcb_get_selection_owner_reply(connection->xcb_connection(), cookie, nullptr);

   if (! reply) {
      return 0;
   }

   const xcb_window_t result = reply->owner;
   free(reply);

   return result;
}

// API for QPlatformNativeInterface/QPlatformSystemTrayIcon: Request a window
// to be docked on the tray.
void QXcbSystemTrayTracker::requestSystemTrayWindowDock(xcb_window_t window) const
{
   xcb_client_message_event_t trayRequest;
   trayRequest.response_type = XCB_CLIENT_MESSAGE;
   trayRequest.format = 32;
   trayRequest.sequence = 0;
   trayRequest.window = m_trayWindow;
   trayRequest.type = m_trayAtom;
   trayRequest.data.data32[0] = XCB_CURRENT_TIME;
   trayRequest.data.data32[1] = SystemTrayRequestDock;
   trayRequest.data.data32[2] = window;
   xcb_send_event(m_connection->xcb_connection(), 0, m_trayWindow, XCB_EVENT_MASK_NO_EVENT, (const char *)&trayRequest);
}

// API for QPlatformNativeInterface/QPlatformSystemTrayIcon: Return tray window.
xcb_window_t QXcbSystemTrayTracker::trayWindow()
{
   if (! m_trayWindow) {
      m_trayWindow = QXcbSystemTrayTracker::locateTrayWindow(m_connection, m_selection);
      if (m_trayWindow) { // Listen for DestroyNotify on tray.
         m_connection->addWindowEventListener(m_trayWindow, this);
         const quint32 mask = XCB_CW_EVENT_MASK;
         const quint32 value = XCB_EVENT_MASK_STRUCTURE_NOTIFY;
         Q_XCB_CALL2(xcb_change_window_attributes(m_connection->xcb_connection(), m_trayWindow, mask, &value), m_connection);
      }
   }

   return m_trayWindow;
}

// API for QPlatformNativeInterface/QPlatformSystemTrayIcon: Return the geometry of a
// a window parented on the tray. Determines the global geometry via XCB since mapToGlobal
// does not work for the QWindow parented on the tray.
QRect QXcbSystemTrayTracker::systemTrayWindowGlobalGeometry(xcb_window_t window) const
{

   xcb_connection_t *conn = m_connection->xcb_connection();
   xcb_get_geometry_reply_t *geomReply = xcb_get_geometry_reply(conn, xcb_get_geometry(conn, window), nullptr);

   if (! geomReply) {
      return QRect();
   }

   xcb_translate_coordinates_reply_t *translateReply =
      xcb_translate_coordinates_reply(conn, xcb_translate_coordinates(conn, window, m_connection->rootWindow(), 0, 0), nullptr);

   if (! translateReply) {
      free(geomReply);
      return QRect();
   }

   const QRect result(QPoint(translateReply->dst_x, translateReply->dst_y), QSize(geomReply->width, geomReply->height));
   free(translateReply);

   return result;
}

inline void QXcbSystemTrayTracker::emitSystemTrayWindowChanged()
{
   if (const QPlatformScreen *ps = m_connection->primaryScreen()) {
      emit systemTrayWindowChanged(ps->screen());
   }
}

// Client messages with the "MANAGER" atom on the root window indicate creation of a new tray.
void QXcbSystemTrayTracker::notifyManagerClientMessageEvent(const xcb_client_message_event_t *t)
{
   if (t->data.data32[1] == m_selection) {
      emitSystemTrayWindowChanged();
   }
}

// Listen for destruction of the tray.
void QXcbSystemTrayTracker::handleDestroyNotifyEvent(const xcb_destroy_notify_event_t *event)
{
   if (event->window == m_trayWindow) {
      m_connection->removeWindowEventListener(m_trayWindow);
      m_trayWindow = XCB_WINDOW_NONE;
      emitSystemTrayWindowChanged();
   }
}

bool QXcbSystemTrayTracker::visualHasAlphaChannel()
{
   if (m_trayWindow == XCB_WINDOW_NONE) {
      return false;
   }

   xcb_atom_t tray_atom = m_connection->atom(QXcbAtom::_NET_SYSTEM_TRAY_VISUAL);

   // Get the xcb property for the _NET_SYSTEM_TRAY_VISUAL atom
   xcb_get_property_cookie_t systray_atom_cookie;
   xcb_get_property_reply_t *systray_atom_reply;

   systray_atom_cookie = xcb_get_property_unchecked(m_connection->xcb_connection(), false, m_trayWindow,
         tray_atom, XCB_ATOM_VISUALID, 0, 1);

   systray_atom_reply = xcb_get_property_reply(m_connection->xcb_connection(), systray_atom_cookie, nullptr);

   if (!systray_atom_reply) {
      return false;
   }

   xcb_visualid_t systrayVisualId = XCB_NONE;
   if (systray_atom_reply->value_len > 0 && xcb_get_property_value_length(systray_atom_reply) > 0) {
      xcb_visualid_t *vids = (uint32_t *)xcb_get_property_value(systray_atom_reply);
      systrayVisualId = vids[0];
   }

   free(systray_atom_reply);

   if (systrayVisualId != XCB_NONE) {
      quint8 depth = m_connection->primaryScreen()->depthOfVisual(systrayVisualId);
      return depth == 32;
   }

   return false;
}

