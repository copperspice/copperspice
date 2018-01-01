/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QEVENT_P_H
#define QEVENT_P_H

#include <QtCore/qglobal.h>
#include <QtCore/qurl.h>
#include <QtGui/qevent.h>

QT_BEGIN_NAMESPACE

// ### Qt5/remove
class QKeyEventEx : public QKeyEvent
{
 public:
   QKeyEventEx(Type type, int key, Qt::KeyboardModifiers modifiers,
               const QString &text, bool autorep, ushort count,
               quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers);
   QKeyEventEx(const QKeyEventEx &other);

   ~QKeyEventEx();

 protected:
   quint32 nScanCode;
   quint32 nVirtualKey;
   quint32 nModifiers;
   friend class QKeyEvent;
};

// ### Qt5/remove
class QMouseEventEx : public QMouseEvent
{
 public:
   QMouseEventEx(Type type, const QPointF &pos, const QPoint &globalPos,
                 Qt::MouseButton button, Qt::MouseButtons buttons,
                 Qt::KeyboardModifiers modifiers);
   ~QMouseEventEx();

 protected:
   QPointF posF;
   friend class QMouseEvent;
};

class QTouchEventTouchPointPrivate
{
 public:
   inline QTouchEventTouchPointPrivate(int id)
      : ref(1),
        id(id),
        state(Qt::TouchPointReleased),
        pressure(qreal(-1.)) {
   }

   inline QTouchEventTouchPointPrivate *detach() {
      QTouchEventTouchPointPrivate *d = new QTouchEventTouchPointPrivate(*this);
      d->ref = 1;
      if (!this->ref.deref()) {
         delete this;
      }
      return d;
   }

   QAtomicInt ref;
   int id;
   Qt::TouchPointStates state;
   QRectF rect, sceneRect, screenRect;
   QPointF normalizedPos,
           startPos, startScenePos, startScreenPos, startNormalizedPos,
           lastPos, lastScenePos, lastScreenPos, lastNormalizedPos;
   qreal pressure;
};

#ifndef QT_NO_GESTURES
class QNativeGestureEvent : public QEvent
{
 public:
   enum Type {
      None,
      GestureBegin,
      GestureEnd,
      Pan,
      Zoom,
      Rotate,
      Swipe
   };

   QNativeGestureEvent()
      : QEvent(QEvent::NativeGesture), gestureType(None), percentage(0)
#ifdef Q_OS_WIN
      , sequenceId(0), argument(0)
#endif
   {
   }

   Type gestureType;
   float percentage;
   QPoint position;
   float angle;
#ifdef Q_OS_WIN
   ulong sequenceId;
   quint64 argument;
#endif
};

class QGestureEventPrivate
{
 public:
   inline QGestureEventPrivate(const QList<QGesture *> &list)
      : gestures(list), widget(0) {
   }

   QList<QGesture *> gestures;
   QWidget *widget;
   QMap<Qt::GestureType, bool> accepted;
   QMap<Qt::GestureType, QWidget *> targetWidgets;
};
#endif // QT_NO_GESTURES

class QFileOpenEventPrivate
{
 public:
   inline QFileOpenEventPrivate(const QUrl &url)
      : url(url) {
   }
   ~QFileOpenEventPrivate();

   QUrl url;
};

QT_END_NAMESPACE

#endif // QEVENT_P_H
