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

#ifndef QWINDOWSYSTEMINTERFACE_P_H
#define QWINDOWSYSTEMINTERFACE_P_H

#include <qwindowsysteminterface.h>

#include <qalgorithms.h>
#include <QAtomicInt>
#include <QElapsedTimer>
#include <QPointer>
#include <QMutex>
#include <QList>
#include <QWaitCondition>

class QWindowSystemEventHandler;

class Q_GUI_EXPORT QWindowSystemInterfacePrivate
{
 public:
   enum EventType {
      UserInputEvent = 0x100,
      Close = UserInputEvent | 0x01,
      GeometryChange = 0x02,
      Enter = UserInputEvent | 0x03,
      Leave = UserInputEvent | 0x04,
      ActivatedWindow = 0x05,
      WindowStateChanged = 0x06,
      Mouse = UserInputEvent | 0x07,
      FrameStrutMouse = UserInputEvent | 0x08,
      Wheel = UserInputEvent | 0x09,
      Key = UserInputEvent | 0x0a,
      Touch = UserInputEvent | 0x0b,
      ScreenOrientation = 0x0c,
      ScreenGeometry = 0x0d,
      ScreenAvailableGeometry = 0x0e,
      ScreenLogicalDotsPerInch = 0x0f,
      ScreenRefreshRate = 0x10,
      ThemeChange = 0x11,
      Expose = 0x12,
      FileOpen = UserInputEvent | 0x13,
      Tablet = UserInputEvent | 0x14,
      TabletEnterProximity = UserInputEvent | 0x15,
      TabletLeaveProximity = UserInputEvent | 0x16,
      PlatformPanel = UserInputEvent | 0x17,
      ContextMenu = UserInputEvent | 0x18,
      EnterWhatsThisMode = UserInputEvent | 0x19,
#ifndef QT_NO_GESTURES
      Gesture = UserInputEvent | 0x1a,
#endif
      ApplicationStateChanged = 0x19,
      FlushEvents = 0x20,
      WindowScreenChanged = 0x21
   };

   class WindowSystemEvent
   {
    public:
      enum EventSource {
         Synthetic  = 0x1,
         NullWindow = 0x2
      };

      explicit WindowSystemEvent(EventType t)
         : type(t), flags(0), eventAccepted(true)
      {
      }

      virtual ~WindowSystemEvent()
      {
      }

      bool synthetic() const  {
         return flags & Synthetic;
      }

      bool nullWindow() const {
         return flags & NullWindow;
      }

      EventType type;
      int flags;
      bool eventAccepted;
   };

   class CloseEvent : public WindowSystemEvent
   {
    public:
      explicit CloseEvent(QWindow *w, bool *isAccepted = nullptr)
         : WindowSystemEvent(Close), window(w), accepted(isAccepted)
      {
      }

      QPointer<QWindow> window;
      bool *accepted;
   };

   class GeometryChangeEvent : public WindowSystemEvent
   {
    public:
      GeometryChangeEvent(QWindow *window, const QRect &newRect, const QRect &oldRect)
         : WindowSystemEvent(GeometryChange), tlw(window), newGeometry(newRect), oldGeometry(oldRect)
      {
      }

      QPointer<QWindow> tlw;
      QRect newGeometry;
      QRect oldGeometry;
   };

   class EnterEvent : public WindowSystemEvent
   {
    public:
      explicit EnterEvent(QWindow *window, const QPointF &localPoint, const QPointF &globalPoint)
         : WindowSystemEvent(Enter), enter(window), localPos(localPoint), globalPos(globalPoint)
      {
      }

      QPointer<QWindow> enter;
      const QPointF localPos;
      const QPointF globalPos;
   };

   class LeaveEvent : public WindowSystemEvent
   {
    public:
      explicit LeaveEvent(QWindow *window)
         : WindowSystemEvent(Leave), leave(window)
      {
      }

      QPointer<QWindow> leave;
   };

   class ActivatedWindowEvent : public WindowSystemEvent
   {
    public:
      explicit ActivatedWindowEvent(QWindow *activatedWindow, Qt::FocusReason r)
         : WindowSystemEvent(ActivatedWindow), activated(activatedWindow), reason(r)
      {
      }

      QPointer<QWindow> activated;
      Qt::FocusReason reason;
   };

   class WindowStateChangedEvent : public WindowSystemEvent
   {
    public:
      WindowStateChangedEvent(QWindow *eventWindow, Qt::WindowState newWindowState)
         : WindowSystemEvent(WindowStateChanged), window(eventWindow), newState(newWindowState)
      {
      }

      QPointer<QWindow> window;
      Qt::WindowState newState;
   };

   class WindowScreenChangedEvent : public WindowSystemEvent
   {
    public:
      WindowScreenChangedEvent(QWindow *w, QScreen *s)
         : WindowSystemEvent(WindowScreenChanged), window(w), screen(s)
      {
      }

      QPointer<QWindow> window;
      QPointer<QScreen> screen;
   };

   class ApplicationStateChangedEvent : public WindowSystemEvent
   {
    public:
      ApplicationStateChangedEvent(Qt::ApplicationState newAppState, bool isForcePropagate = false)
         : WindowSystemEvent(ApplicationStateChanged), newState(newAppState), forcePropagate(isForcePropagate)
      {
      }

      Qt::ApplicationState newState;
      bool forcePropagate;
   };

   class FlushEventsEvent : public WindowSystemEvent
   {
    public:
      FlushEventsEvent(QEventLoop::ProcessEventsFlags f = QEventLoop::AllEvents)
         : WindowSystemEvent(FlushEvents), flags(f)
      {
      }

      QEventLoop::ProcessEventsFlags flags;
   };

   class UserEvent : public WindowSystemEvent
   {
    public:
      UserEvent(QWindow *w, ulong time, EventType t)
         : WindowSystemEvent(t), window(w), timestamp(time)
      {
         if (! w) {
            flags |= NullWindow;
         }
      }

      QPointer<QWindow> window;
      unsigned long timestamp;
   };

   class InputEvent: public UserEvent
   {
    public:
      InputEvent(QWindow *w, ulong time, EventType t, Qt::KeyboardModifiers mods)
         : UserEvent(w, time, t), modifiers(mods)
      {
      }

      Qt::KeyboardModifiers modifiers;
   };

   class MouseEvent : public InputEvent
   {
    public:
      MouseEvent(QWindow *w, ulong time, const QPointF &local, const QPointF &global,
         Qt::MouseButtons b, Qt::KeyboardModifiers mods,
         Qt::MouseEventSource src = Qt::MouseEventNotSynthesized)
         : InputEvent(w, time, Mouse, mods), localPos(local), globalPos(global), buttons(b), source(src)
      {
      }

      MouseEvent(QWindow *w, ulong time, EventType t, const QPointF &local, const QPointF &global,
         Qt::MouseButtons b, Qt::KeyboardModifiers mods,
         Qt::MouseEventSource src = Qt::MouseEventNotSynthesized)
         : InputEvent(w, time, t, mods), localPos(local), globalPos(global), buttons(b), source(src)
      {
      }

      QPointF localPos;
      QPointF globalPos;
      Qt::MouseButtons buttons;
      Qt::MouseEventSource source;
   };

   class WheelEvent : public InputEvent
   {
    public:
      WheelEvent(QWindow *w, ulong time, const QPointF &local, const QPointF &global, QPoint pixelD, QPoint angleD,
         Qt::KeyboardModifiers mods, Qt::ScrollPhase phase = Qt::NoScrollPhase,
         Qt::MouseEventSource src = Qt::MouseEventNotSynthesized);

      QPoint pixelDelta;
      QPoint angleDelta;
      QPointF localPos;
      QPointF globalPos;
      Qt::ScrollPhase phase;
      Qt::MouseEventSource source;
   };

   class KeyEvent : public InputEvent
   {
    public:
      KeyEvent(QWindow *w, ulong time, QEvent::Type t, int k, Qt::KeyboardModifiers mods, const QString &text = QString(),
         bool autorep = false, ushort count = 1)
         : InputEvent(w, time, Key, mods), key(k), unicode(text), repeat(autorep),
           repeatCount(count), keyType(t), nativeScanCode(0), nativeVirtualKey(0), nativeModifiers(0)
      {
      }

      KeyEvent(QWindow *w, ulong time, QEvent::Type t, int k, Qt::KeyboardModifiers mods,
         quint32 nativeSC, quint32 nativeVK, quint32 nativeMods,
         const QString &text = QString(), bool autorep = false, ushort count = 1)
         : InputEvent(w, time, Key, mods), key(k), unicode(text), repeat(autorep),
           repeatCount(count), keyType(t), nativeScanCode(nativeSC), nativeVirtualKey(nativeVK),
           nativeModifiers(nativeMods)
      {
      }

      int key;
      QString unicode;
      bool repeat;
      ushort repeatCount;
      QEvent::Type keyType;
      quint32 nativeScanCode;
      quint32 nativeVirtualKey;
      quint32 nativeModifiers;
   };

   class TouchEvent : public InputEvent
   {
    public:
      TouchEvent(QWindow *w, ulong time, QEvent::Type t, QTouchDevice *dev,
         const QList<QTouchEvent::TouchPoint> &p, Qt::KeyboardModifiers mods)
         : InputEvent(w, time, Touch, mods), device(dev), points(p), touchType(t)
      {
      }

      QTouchDevice *device;
      QList<QTouchEvent::TouchPoint> points;
      QEvent::Type touchType;
   };

   class ScreenOrientationEvent : public WindowSystemEvent
   {
    public:
      ScreenOrientationEvent(QScreen *s, Qt::ScreenOrientation o)
         : WindowSystemEvent(ScreenOrientation), screen(s), orientation(o)
      {
      }

      QPointer<QScreen> screen;
      Qt::ScreenOrientation orientation;
   };

   class ScreenGeometryEvent : public WindowSystemEvent
   {
    public:
      ScreenGeometryEvent(QScreen *s, const QRect &g, const QRect &ag)
         : WindowSystemEvent(ScreenGeometry), screen(s), geometry(g), availableGeometry(ag)
      {
      }

      QPointer<QScreen> screen;
      QRect geometry;
      QRect availableGeometry;
   };

   class ScreenLogicalDotsPerInchEvent : public WindowSystemEvent
   {
    public:
      ScreenLogicalDotsPerInchEvent(QScreen *s, qreal dx, qreal dy)
         : WindowSystemEvent(ScreenLogicalDotsPerInch), screen(s), dpiX(dx), dpiY(dy)
      {
      }

      QPointer<QScreen> screen;
      qreal dpiX;
      qreal dpiY;
   };

   class ScreenRefreshRateEvent : public WindowSystemEvent
   {
    public:
      ScreenRefreshRateEvent(QScreen *s, qreal r)
         : WindowSystemEvent(ScreenRefreshRate), screen(s), rate(r)
      {
      }

      QPointer<QScreen> screen;
      qreal rate;
   };

   class ThemeChangeEvent : public WindowSystemEvent
   {
    public:
      explicit ThemeChangeEvent(QWindow *w)
         : WindowSystemEvent(ThemeChange), window(w) { }
      QPointer<QWindow> window;
   };

   class ExposeEvent : public WindowSystemEvent
   {
    public:
      ExposeEvent(QWindow *exposed, const QRegion &region);
      QPointer<QWindow> exposed;
      bool isExposed;
      QRegion region;
   };

   class FileOpenEvent : public WindowSystemEvent
   {
    public:
      FileOpenEvent(const QString &fileName)
         : WindowSystemEvent(FileOpen), m_url(QUrl::fromLocalFile(fileName))
      {
      }

      FileOpenEvent(const QUrl &url)
         : WindowSystemEvent(FileOpen), m_url(url)
      {
      }

      QUrl m_url;
   };

   class TabletEvent : public InputEvent
   {
    public:
      static void handleTabletEvent(QWindow *w, const QPointF &local, const QPointF &global,
            int device, int pointerType, Qt::MouseButtons buttons, qreal pressure, int xTilt, int yTilt,
            qreal tangentialPressure, qreal rotation, int z, qint64 uid,
            Qt::KeyboardModifiers modifiers = Qt::NoModifier);

      TabletEvent(QWindow *w, ulong time, const QPointF &localPoint, const QPointF &globalPoint,
            int deviceId, int pointerId, Qt::MouseButtons b, qreal penPressure, int xTilt_Value, int yTilt_Value,
            qreal tanPressure, qreal rotationValue, int zValue, qint64 userId, Qt::KeyboardModifiers mods)
         : InputEvent(w, time, Tablet, mods),
           buttons(b), local(localPoint), global(globalPoint), device(deviceId), pointerType(pointerId),
           pressure(penPressure), xTilt(xTilt_Value), yTilt(yTilt_Value), tangentialPressure(tanPressure),
           rotation(rotationValue), z(zValue), uid(userId)
      {
      }

      Qt::MouseButtons buttons;
      QPointF local;
      QPointF global;
      int device;
      int pointerType;
      qreal pressure;
      int xTilt;
      int yTilt;
      qreal tangentialPressure;
      qreal rotation;
      int z;
      qint64 uid;
   };

   class TabletEnterProximityEvent : public InputEvent
   {
    public:
      TabletEnterProximityEvent(ulong time, int deviceId, int pointerId, qint64 userId)
         : InputEvent(nullptr, time, TabletEnterProximity, Qt::NoModifier),
           device(deviceId), pointerType(pointerId), uid(userId)
      {
      }

      int device;
      int pointerType;
      qint64 uid;
   };

   class TabletLeaveProximityEvent : public InputEvent
   {
    public:
      TabletLeaveProximityEvent(ulong time, int deviceId, int pointerId, qint64 userId)
         : InputEvent(nullptr, time, TabletLeaveProximity, Qt::NoModifier),
           device(deviceId), pointerType(pointerId), uid(userId)
      {
      }

      int device;
      int pointerType;
      qint64 uid;
   };

   class PlatformPanelEvent : public WindowSystemEvent
   {
    public:
      explicit PlatformPanelEvent(QWindow *w)
         : WindowSystemEvent(PlatformPanel), window(w)
      {
      }

      QPointer<QWindow> window;
   };

#ifndef QT_NO_CONTEXTMENU
   class ContextMenuEvent : public WindowSystemEvent
   {
    public:
      explicit ContextMenuEvent(QWindow *w, bool isMouseTriggered, const QPoint &posPoint,
         const QPoint &globalPoint, Qt::KeyboardModifiers keyModifiers)
         : WindowSystemEvent(ContextMenu), window(w), mouseTriggered(isMouseTriggered), pos(posPoint),
           globalPos(globalPoint), modifiers(keyModifiers)
      {
      }

      QPointer<QWindow> window;
      bool mouseTriggered;
      QPoint pos;       // Only valid if triggered by mouse
      QPoint globalPos; // Only valid if triggered by mouse
      Qt::KeyboardModifiers modifiers;
   };
#endif

#ifndef QT_NO_GESTURES
   class GestureEvent : public InputEvent
   {
    public:
      GestureEvent(QWindow *w, ulong time, Qt::NativeGestureType gestureType, QPointF posPoint, QPointF globalPoint)
         : InputEvent(w, time, Gesture, Qt::NoModifier), type(gestureType), pos(posPoint), globalPos(globalPoint),
           realValue(0), sequenceId(0), intValue(0)
      {
      }

      Qt::NativeGestureType type;
      QPointF pos;
      QPointF globalPos;

      // Mac
      qreal realValue;

      // Windows
      ulong sequenceId;
      quint64 intValue;
   };
#endif

   class WindowSystemEventList
   {
      QList<WindowSystemEvent *> impl;
      mutable QMutex mutex;

    public:
      WindowSystemEventList()
         : impl(), mutex()
      {
      }

      WindowSystemEventList(const WindowSystemEventList &) = delete;
      WindowSystemEventList &operator=(const WindowSystemEventList &) = delete;

      ~WindowSystemEventList() {
         clear();
      }

      void clear() {
         const QMutexLocker locker(&mutex);
         qDeleteAll(impl);
         impl.clear();
      }

      void prepend(WindowSystemEvent *e) {
         const QMutexLocker locker(&mutex);
         impl.prepend(e);
      }

      WindowSystemEvent *takeFirstOrReturnNull() {
         const QMutexLocker locker(&mutex);
         return impl.empty() ? nullptr : impl.takeFirst();
      }

      WindowSystemEvent *takeFirstNonUserInputOrReturnNull() {
         const QMutexLocker locker(&mutex);
         for (int i = 0; i < impl.size(); ++i) {
            if (!(impl.at(i)->type & QWindowSystemInterfacePrivate::UserInputEvent)) {
               return impl.takeAt(i);
            }
         }

         return nullptr;
      }

      void append(WindowSystemEvent *e) {
         const QMutexLocker locker(&mutex);
         impl.append(e);
      }

      int count() const {
         const QMutexLocker locker(&mutex);
         return impl.count();
      }

      WindowSystemEvent *peekAtFirstOfType(EventType t) const {
         const QMutexLocker locker(&mutex);

         for (int i = 0; i < impl.size(); ++i) {
            if (impl.at(i)->type == t) {
               return impl.at(i);
            }
         }

         return nullptr;
      }

      void remove(const WindowSystemEvent *e) {
         const QMutexLocker locker(&mutex);
         for (int i = 0; i < impl.size(); ++i) {
            if (impl.at(i) == e) {
               delete impl.takeAt(i);
               break;
            }
         }
      }
   };

   static WindowSystemEventList windowSystemEventQueue;

   static int windowSystemEventsQueued();
   static WindowSystemEvent *getWindowSystemEvent();
   static WindowSystemEvent *getNonUserInputWindowSystemEvent();
   static WindowSystemEvent *peekWindowSystemEvent(EventType t);
   static void removeWindowSystemEvent(WindowSystemEvent *event);
   static void postWindowSystemEvent(WindowSystemEvent *event);
   static bool handleWindowSystemEvent(WindowSystemEvent *event);

   static QElapsedTimer eventTime;
   static bool synchronousWindowSystemEvents;

   static QWaitCondition eventsFlushed;
   static QMutex flushEventMutex;
   static QAtomicInt eventAccepted;

   static QList<QTouchEvent::TouchPoint> fromNativeTouchPoints(const QList<QWindowSystemInterface::TouchPoint> &points,
      const QWindow *window, QEvent::Type *type = nullptr);

   static QList<QWindowSystemInterface::TouchPoint> toNativeTouchPoints(const QList<QTouchEvent::TouchPoint> &pointList,
      const QWindow *window);

   static void installWindowSystemEventHandler(QWindowSystemEventHandler *handler);
   static void removeWindowSystemEventhandler(QWindowSystemEventHandler *handler);
   static QWindowSystemEventHandler *eventHandler;
};

class Q_GUI_EXPORT QWindowSystemEventHandler
{
 public:
   virtual ~QWindowSystemEventHandler();
   virtual bool sendEvent(QWindowSystemInterfacePrivate::WindowSystemEvent *event);
};

#endif
