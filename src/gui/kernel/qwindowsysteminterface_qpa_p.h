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

#ifndef QWINDOWSYSTEMINTERFACE_QPA_P_H
#define QWINDOWSYSTEMINTERFACE_QPA_P_H

#include <qwindowsysteminterface_qpa.h>

QT_BEGIN_NAMESPACE

class QWindowSystemInterfacePrivate
{
 public:
   enum EventType {
      Close,
      GeometryChange,
      Enter,
      Leave,
      ActivatedWindow,
      WindowStateChanged,
      Mouse,
      Wheel,
      Key,
      Touch,
      ScreenGeometry,
      ScreenAvailableGeometry,
      ScreenCountChange,
      LocaleChange,
      PlatformPanel
   };

   class WindowSystemEvent
   {
    public:
      WindowSystemEvent(EventType t)
         : type(t) { }
      virtual ~WindowSystemEvent() {}
      EventType type;
   };

   class CloseEvent : public WindowSystemEvent
   {
    public:
      CloseEvent(QWidget *tlw)
         : WindowSystemEvent(Close), topLevel(tlw) { }
      QWeakPointer<QWidget> topLevel;
   };

   class GeometryChangeEvent : public WindowSystemEvent
   {
    public:
      GeometryChangeEvent(QWidget *tlw, const QRect &newGeometry)
         : WindowSystemEvent(GeometryChange), tlw(tlw), newGeometry(newGeometry) {
      }
      QWeakPointer<QWidget> tlw;
      QRect newGeometry;
   };

   class EnterEvent : public WindowSystemEvent
   {
    public:
      EnterEvent(QWidget *enter)
         : WindowSystemEvent(Enter), enter(enter) {
      }
      QWeakPointer<QWidget> enter;
   };

   class LeaveEvent : public WindowSystemEvent
   {
    public:
      LeaveEvent(QWidget *leave)
         : WindowSystemEvent(Leave), leave(leave) {
      }
      QWeakPointer<QWidget> leave;
   };

   class ActivatedWindowEvent : public WindowSystemEvent
   {
    public:
      ActivatedWindowEvent(QWidget *activatedWindow)
         : WindowSystemEvent(ActivatedWindow), activated(activatedWindow) {
      }
      QWeakPointer<QWidget> activated;
   };

   class WindowStateChangedEvent : public WindowSystemEvent
   {
    public:
      WindowStateChangedEvent(QWidget *_tlw, Qt::WindowState _newState)
         : WindowSystemEvent(WindowStateChanged), tlw(_tlw), newState(_newState) {
      }

      QWeakPointer<QWidget> tlw;
      Qt::WindowState newState;
   };

   class UserEvent : public WindowSystemEvent
   {
    public:
      UserEvent(QWidget *w, ulong time, EventType t)
         : WindowSystemEvent(t), widget(w), timestamp(time) { }
      QWeakPointer<QWidget> widget;
      unsigned long timestamp;
   };

   class MouseEvent : public UserEvent
   {
    public:
      MouseEvent(QWidget *w, ulong time, const QPoint &local, const QPoint &global, Qt::MouseButtons b)
         : UserEvent(w, time, Mouse), localPos(local), globalPos(global), buttons(b) { }
      QPoint localPos;
      QPoint globalPos;
      Qt::MouseButtons buttons;
   };

   class WheelEvent : public UserEvent
   {
    public:
      WheelEvent(QWidget *w, ulong time, const QPoint &local, const QPoint &global, int d, Qt::Orientation o)
         : UserEvent(w, time, Wheel), delta(d), localPos(local), globalPos(global), orient(o) { }
      int delta;
      QPoint localPos;
      QPoint globalPos;
      Qt::Orientation orient;
   };

   class KeyEvent : public UserEvent
   {
    public:
      KeyEvent(QWidget *w, ulong time, QEvent::Type t, int k, Qt::KeyboardModifiers mods, const QString &text = QString(),
               bool autorep = false, ushort count = 1)
         : UserEvent(w, time, Key), key(k), unicode(text), repeat(autorep),
           repeatCount(count), modifiers(mods), keyType(t),
           nativeScanCode(0), nativeVirtualKey(0), nativeModifiers(0) { }
      KeyEvent(QWidget *w, ulong time, QEvent::Type t, int k, Qt::KeyboardModifiers mods,
               quint32 nativeSC, quint32 nativeVK, quint32 nativeMods,
               const QString &text = QString(), bool autorep = false, ushort count = 1)
         : UserEvent(w, time, Key), key(k), unicode(text), repeat(autorep),
           repeatCount(count), modifiers(mods), keyType(t),
           nativeScanCode(nativeSC), nativeVirtualKey(nativeVK), nativeModifiers(nativeMods) { }
      int key;
      QString unicode;
      bool repeat;
      ushort repeatCount;
      Qt::KeyboardModifiers modifiers;
      QEvent::Type keyType;
      quint32 nativeScanCode;
      quint32 nativeVirtualKey;
      quint32 nativeModifiers;
   };

   class TouchEvent : public UserEvent
   {
    public:
      TouchEvent(QWidget *w, ulong time, QEvent::Type t, QTouchEvent::DeviceType d, const QList<QTouchEvent::TouchPoint> &p)
         : UserEvent(w, time, Touch), devType(d), points(p), touchType(t) { }
      QTouchEvent::DeviceType devType;
      QList<QTouchEvent::TouchPoint> points;
      QEvent::Type touchType;

   };

   class ScreenCountEvent : public WindowSystemEvent
   {
    public:
      ScreenCountEvent (int count)
         : WindowSystemEvent(ScreenCountChange) , count(count) { }
      int count;
   };

   class ScreenGeometryEvent : public WindowSystemEvent
   {
    public:
      ScreenGeometryEvent(int index)
         : WindowSystemEvent(ScreenGeometry), index(index) { }
      int index;
   };

   class ScreenAvailableGeometryEvent : public WindowSystemEvent
   {
    public:
      ScreenAvailableGeometryEvent(int index)
         : WindowSystemEvent(ScreenAvailableGeometry), index(index) { }
      int index;
   };

   class LocaleChangeEvent : public WindowSystemEvent
   {
    public:
      LocaleChangeEvent()
         : WindowSystemEvent(LocaleChange) { }
   };

   class PlatformPanelEvent : public WindowSystemEvent
   {
    public:
      explicit PlatformPanelEvent(QWidget *w)
         : WindowSystemEvent(PlatformPanel), widget(w) { }
      QWeakPointer<QWidget> widget;
   };

   static QList<WindowSystemEvent *> windowSystemEventQueue;
   static QMutex queueMutex;

   static int windowSystemEventsQueued();
   static WindowSystemEvent *getWindowSystemEvent();
   static void queueWindowSystemEvent(WindowSystemEvent *ev);

   static QTime eventTime;
};

QT_END_NAMESPACE

#endif // QWINDOWSYSTEMINTERFACE_QPA_P_H
