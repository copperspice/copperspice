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

#ifndef QXCB_SYSTEMTRAYTRACKER_H
#define QXCB_SYSTEMTRAYTRACKER_H

#include <qxcb_connection.h>
#include <qobject.h>

#include <xcb/xcb.h>

class QXcbConnection;
class QScreen;

class QXcbSystemTrayTracker : public QObject, public QXcbWindowEventListener
{
   CS_OBJECT_MULTIPLE(QXcbSystemTrayTracker, QObject)

 public:
   static QXcbSystemTrayTracker *create(QXcbConnection *connection);

   xcb_window_t trayWindow();
   void requestSystemTrayWindowDock(xcb_window_t window) const;
   QRect systemTrayWindowGlobalGeometry(xcb_window_t window) const;

   void notifyManagerClientMessageEvent(const xcb_client_message_event_t *);

   void handleDestroyNotifyEvent(const xcb_destroy_notify_event_t *) override;

   bool visualHasAlphaChannel();

   CS_SIGNAL_1(Public, void systemTrayWindowChanged(QScreen *screen))
   CS_SIGNAL_2(systemTrayWindowChanged, screen)

 private:
   explicit QXcbSystemTrayTracker(QXcbConnection *connection, xcb_atom_t trayAtom, xcb_atom_t selection);
   static xcb_window_t locateTrayWindow(const QXcbConnection *connection, xcb_atom_t selection);
   void emitSystemTrayWindowChanged();

   const xcb_atom_t m_selection;
   const xcb_atom_t m_trayAtom;
   QXcbConnection *m_connection;
   xcb_window_t m_trayWindow;
};

#endif
