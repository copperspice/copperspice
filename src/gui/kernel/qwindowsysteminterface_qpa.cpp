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

#include <qwindowsysteminterface_qpa.h>
#include <qwindowsysteminterface_qpa_p.h>
#include <qapplication_p.h>
#include <QAbstractEventDispatcher>
#include <qlocale_p.h>

QT_BEGIN_NAMESPACE

QTime QWindowSystemInterfacePrivate::eventTime;

// Callback functions for plugins:

QList<QWindowSystemInterfacePrivate::WindowSystemEvent *> QWindowSystemInterfacePrivate::windowSystemEventQueue;
QMutex QWindowSystemInterfacePrivate::queueMutex;

extern QPointer<QWidget> qt_last_mouse_receiver;


void QWindowSystemInterface::handleEnterEvent(QWidget *tlw)
{
   if (tlw) {
      QWidgetData *data = qt_qwidget_data(tlw);
      if (data->in_destructor) {
         return;
      }

      QWindowSystemInterfacePrivate::EnterEvent *e = new QWindowSystemInterfacePrivate::EnterEvent(tlw);
      QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
   }
}

void QWindowSystemInterface::handleLeaveEvent(QWidget *tlw)
{
   if (tlw) {
      QWidgetData *data = qt_qwidget_data(tlw);
      if (data->in_destructor) {
         return;
      }
   }
   QWindowSystemInterfacePrivate::LeaveEvent *e = new QWindowSystemInterfacePrivate::LeaveEvent(tlw);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handleWindowActivated(QWidget *tlw)
{
   QWindowSystemInterfacePrivate::ActivatedWindowEvent *e = new QWindowSystemInterfacePrivate::ActivatedWindowEvent(tlw);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handleWindowStateChanged(QWidget *tlw, Qt::WindowState newState)
{
   QWindowSystemInterfacePrivate::WindowStateChangedEvent *e =
      new QWindowSystemInterfacePrivate::WindowStateChangedEvent(tlw, newState);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handleGeometryChange(QWidget *tlw, const QRect &newRect)
{
   if (tlw) {
      QWidgetData *data = qt_qwidget_data(tlw);
      if (data->in_destructor) {
         return;
      }
   }
   QWindowSystemInterfacePrivate::GeometryChangeEvent *e = new QWindowSystemInterfacePrivate::GeometryChangeEvent(tlw,
         newRect);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}


void QWindowSystemInterface::handleCloseEvent(QWidget *tlw)
{
   if (tlw) {
      QWindowSystemInterfacePrivate::CloseEvent *e =
         new QWindowSystemInterfacePrivate::CloseEvent(tlw);
      QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
   }
}

/*!

\a tlw == 0 means that \a ev is in global coords only


*/
void QWindowSystemInterface::handleMouseEvent(QWidget *w, const QPoint &local, const QPoint &global,
      Qt::MouseButtons b)
{
   unsigned long time = QWindowSystemInterfacePrivate::eventTime.elapsed();
   handleMouseEvent(w, time, local, global, b);
}

void QWindowSystemInterface::handleMouseEvent(QWidget *tlw, ulong timestamp, const QPoint &local, const QPoint &global,
      Qt::MouseButtons b)
{
   if (tlw) {
      QWidgetData *data = qt_qwidget_data(tlw);
      if (data->in_destructor) {
         tlw = 0;
      }
   }
   QWindowSystemInterfacePrivate::MouseEvent *e =
      new QWindowSystemInterfacePrivate::MouseEvent(tlw, timestamp, local, global, b);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handleKeyEvent(QWidget *w, QEvent::Type t, int k, Qt::KeyboardModifiers mods,
      const QString &text, bool autorep, ushort count)
{
   unsigned long time = QWindowSystemInterfacePrivate::eventTime.elapsed();
   handleKeyEvent(w, time, t, k, mods, text, autorep, count);
}

void QWindowSystemInterface::handleKeyEvent(QWidget *tlw, ulong timestamp, QEvent::Type t, int k,
      Qt::KeyboardModifiers mods, const QString &text, bool autorep, ushort count)
{
   if (tlw) {
      QWidgetData *data = qt_qwidget_data(tlw);
      if (data->in_destructor) {
         tlw = 0;
      }
   }

   QWindowSystemInterfacePrivate::KeyEvent *e =
      new QWindowSystemInterfacePrivate::KeyEvent(tlw, timestamp, t, k, mods, text, autorep, count);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handleExtendedKeyEvent(QWidget *w, QEvent::Type type, int key,
      Qt::KeyboardModifiers modifiers,
      quint32 nativeScanCode, quint32 nativeVirtualKey,
      quint32 nativeModifiers,
      const QString &text, bool autorep,
      ushort count)
{
   unsigned long time = QWindowSystemInterfacePrivate::eventTime.elapsed();
   handleExtendedKeyEvent(w, time, type, key, modifiers, nativeScanCode, nativeVirtualKey, nativeModifiers,
                          text, autorep, count);
}

void QWindowSystemInterface::handleExtendedKeyEvent(QWidget *tlw, ulong timestamp, QEvent::Type type, int key,
      Qt::KeyboardModifiers modifiers,
      quint32 nativeScanCode, quint32 nativeVirtualKey,
      quint32 nativeModifiers,
      const QString &text, bool autorep,
      ushort count)
{
   if (tlw) {
      QWidgetData *data = qt_qwidget_data(tlw);
      if (data->in_destructor) {
         tlw = 0;
      }
   }

   QWindowSystemInterfacePrivate::KeyEvent *e =
      new QWindowSystemInterfacePrivate::KeyEvent(tlw, timestamp, type, key, modifiers,
            nativeScanCode, nativeVirtualKey, nativeModifiers, text, autorep, count);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handleWheelEvent(QWidget *w, const QPoint &local, const QPoint &global, int d,
      Qt::Orientation o)
{
   unsigned long time = QWindowSystemInterfacePrivate::eventTime.elapsed();
   handleWheelEvent(w, time, local, global, d, o);
}

void QWindowSystemInterface::handleWheelEvent(QWidget *tlw, ulong timestamp, const QPoint &local, const QPoint &global,
      int d, Qt::Orientation o)
{
   if (tlw) {
      QWidgetData *data = qt_qwidget_data(tlw);
      if (data->in_destructor) {
         tlw = 0;
      }
   }

   QWindowSystemInterfacePrivate::WheelEvent *e =
      new QWindowSystemInterfacePrivate::WheelEvent(tlw, timestamp, local, global, d, o);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

int QWindowSystemInterfacePrivate::windowSystemEventsQueued()
{
   queueMutex.lock();
   int ret = windowSystemEventQueue.count();
   queueMutex.unlock();
   return ret;
}

QWindowSystemInterfacePrivate::WindowSystemEvent *QWindowSystemInterfacePrivate::getWindowSystemEvent()
{
   queueMutex.lock();
   QWindowSystemInterfacePrivate::WindowSystemEvent *ret;
   if (windowSystemEventQueue.isEmpty()) {
      ret = 0;
   } else {
      ret = windowSystemEventQueue.takeFirst();
   }
   queueMutex.unlock();
   return ret;
}

void QWindowSystemInterfacePrivate::queueWindowSystemEvent(QWindowSystemInterfacePrivate::WindowSystemEvent *ev)
{
   queueMutex.lock();
   windowSystemEventQueue.append(ev);
   queueMutex.unlock();

   QAbstractEventDispatcher *dispatcher = QApplicationPrivate::qt_qpa_core_dispatcher();
   if (dispatcher) {
      dispatcher->wakeUp();
   }
}

void QWindowSystemInterface::handleTouchEvent(QWidget *w, QEvent::Type type, QTouchEvent::DeviceType devType,
      const QList<struct TouchPoint> &points)
{
   unsigned long time = QWindowSystemInterfacePrivate::eventTime.elapsed();
   handleTouchEvent(w, time, type, devType, points);
}

void QWindowSystemInterface::handleTouchEvent(QWidget *tlw, ulong timestamp, QEvent::Type type,
      QTouchEvent::DeviceType devType, const QList<struct TouchPoint> &points)
{
   if (!points.size()) { // Touch events must have at least one point
      return;
   }

   QList<QTouchEvent::TouchPoint> touchPoints;
   Qt::TouchPointStates states;
   QTouchEvent::TouchPoint p;

   QList<struct TouchPoint>::const_iterator point = points.constBegin();
   QList<struct TouchPoint>::const_iterator end = points.constEnd();
   while (point != end) {
      p.setId(point->id);
      p.setPressure(point->pressure);
      states |= point->state;
      Qt::TouchPointStates state = point->state;
      if (point->isPrimary) {
         state |= Qt::TouchPointPrimary;
      }
      p.setState(state);
      p.setRect(point->area);
      p.setScreenPos(point->area.center());
      p.setNormalizedPos(point->normalPosition);

      touchPoints.append(p);
      ++point;
   }

   QWindowSystemInterfacePrivate::TouchEvent *e =
      new QWindowSystemInterfacePrivate::TouchEvent(tlw, timestamp, type, devType, touchPoints);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handleScreenGeometryChange(int screenIndex)
{
   QWindowSystemInterfacePrivate::ScreenGeometryEvent *e =
      new QWindowSystemInterfacePrivate::ScreenGeometryEvent(screenIndex);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handleScreenAvailableGeometryChange(int screenIndex)
{
   QWindowSystemInterfacePrivate::ScreenAvailableGeometryEvent *e =
      new QWindowSystemInterfacePrivate::ScreenAvailableGeometryEvent(screenIndex);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handleScreenCountChange(int count)
{
   QWindowSystemInterfacePrivate::ScreenCountEvent *e =
      new QWindowSystemInterfacePrivate::ScreenCountEvent(count);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handleLocaleChange()
{
   QWindowSystemInterfacePrivate::LocaleChangeEvent *e =
      new QWindowSystemInterfacePrivate::LocaleChangeEvent();

   QLocalePrivate::updateSystemPrivate();
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

void QWindowSystemInterface::handlePlatformPanelEvent(QWidget *w)
{
   QWindowSystemInterfacePrivate::PlatformPanelEvent *e =
      new QWindowSystemInterfacePrivate::PlatformPanelEvent(w);
   QWindowSystemInterfacePrivate::queueWindowSystemEvent(e);
}

QT_END_NAMESPACE
