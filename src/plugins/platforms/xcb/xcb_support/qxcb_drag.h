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

#ifndef QXCB_DRAG_H
#define QXCB_DRAG_H

#include <qbackingstore.h>
#include <qplatform_drag.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsharedpointer.h>
#include <qpointer.h>
#include <qvector.h>
#include <qdatetime.h>
#include <qpixmap.h>
#include <qdebug.h>
#include <qxcb_object.h>

#include <qsimpledrag_p.h>

#include <xcb/xcb.h>

#ifndef QT_NO_DRAGANDDROP

class QWindow;
class QPlatformWindow;
class QXcbConnection;
class QXcbWindow;
class QXcbDropData;
class QXcbScreen;
class QDrag;
class QShapedPixmapWindow;

class QXcbDrag : public QXcbObject, public QBasicDrag
{
 public:
   QXcbDrag(QXcbConnection *c);
   ~QXcbDrag();

   QMimeData *platformDropData() override;
   bool eventFilter(QObject *o, QEvent *e) override;

   void startDrag() override;
   void cancel() override;
   void move(const QPoint &globalPos) override;
   void drop(const QPoint &globalPos) override;
   void endDrag() override;

   void handleEnter(QPlatformWindow *window, const xcb_client_message_event_t *event, xcb_window_t proxy = 0);
   void handlePosition(QPlatformWindow *w, const xcb_client_message_event_t *event);
   void handleLeave(QPlatformWindow *w, const xcb_client_message_event_t *event);
   void handleDrop(QPlatformWindow *, const xcb_client_message_event_t *event);

   void handleStatus(const xcb_client_message_event_t *event);
   void handleSelectionRequest(const xcb_selection_request_event_t *event);
   void handleFinished(const xcb_client_message_event_t *event);

   bool dndEnable(QXcbWindow *win, bool on);
   bool ownsDragObject() const override;

   void updatePixmap();
   xcb_timestamp_t targetTime() {
      return target_time;
   }

 protected:
   void timerEvent(QTimerEvent *e) override;

 private:
   // 10 minute timer used to discard old XdndDrop transactions
   static constexpr const int XdndDropTransactionTimeout = 600000;

   // the types in this drop. 100 is no good, but at least it's big.
   static constexpr const int xdnd_max_type = 100;

   friend class QXcbDropData;

   void init();

   void handle_xdnd_position(QPlatformWindow *w, const xcb_client_message_event_t *event);
   void handle_xdnd_status(const xcb_client_message_event_t *event);
   void send_leave();

   Qt::DropAction toDropAction(xcb_atom_t atom) const;
   xcb_atom_t toXdndAction(Qt::DropAction a) const;

   QPointer<QWindow> initiatorWindow;
   QPointer<QWindow> currentWindow;
   QPoint currentPosition;

   QXcbDropData *dropData;
   Qt::DropAction accepted_drop_action;

   QWindow *desktop_proxy;

   xcb_atom_t xdnd_dragsource;

   QVector<xcb_atom_t> xdnd_types;

   // timestamp from XdndPosition and XdndDroptime for retrieving the data
   xcb_timestamp_t target_time;
   xcb_timestamp_t source_time;

   // rectangle in which the answer will be the same
   QRect source_sameanswer;
   bool waiting_for_status;

   // top-level window we sent position to last.
   xcb_window_t current_target;
   // window to send events to (always valid if current_target)
   xcb_window_t current_proxy_target;

   QXcbVirtualDesktop *current_virtual_desktop;
   int cleanup_timer;

   QVector<xcb_atom_t> drag_types;

   struct Transaction {
      xcb_timestamp_t timestamp;
      xcb_window_t target;
      xcb_window_t proxy_target;
      QPlatformWindow *targetWindow;

      //      QWidget *embedding_widget;
      QPointer<QDrag> drag;
      QTime time;
   };
   QVector<Transaction> transactions;

   int transaction_expiry_timer;
   void restartDropExpiryTimer();
   int findTransactionByWindow(xcb_window_t window);
   int findTransactionByTime(xcb_timestamp_t timestamp);
   xcb_window_t findRealWindow(const QPoint &pos, xcb_window_t w, int md, bool ignoreNonXdndAwareWindows);
};

#endif // QT_NO_DRAGANDDROP


#endif
