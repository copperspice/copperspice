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

#ifndef QWINDOWSYSTEMINTERFACE_H
#define QWINDOWSYSTEMINTERFACE_H

#include <QTime>
#include <qwindowdefs.h>
#include <qcoreevent.h>
#include <QAbstractEventDispatcher>
#include <QScreen>
#include <QWindow>
#include <QWeakPointer>
#include <QMutex>
#include <QTouchEvent>
#include <QEventLoop>
#include <QVector2D>

class QMimeData;
class QTouchDevice;
class QPlatformDragQtResponse;
class QPlatformDropQtResponse;

class Q_GUI_EXPORT QWindowSystemInterface
{
 public:
   static void handleMouseEvent(QWindow *w, const QPointF &local, const QPointF &global, Qt::MouseButtons b,
      Qt::KeyboardModifiers mods = Qt::NoModifier,
      Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);
   static void handleMouseEvent(QWindow *w, ulong timestamp, const QPointF &local, const QPointF &global, Qt::MouseButtons b,
      Qt::KeyboardModifiers mods = Qt::NoModifier,
      Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);
   static void handleFrameStrutMouseEvent(QWindow *w, const QPointF &local, const QPointF &global, Qt::MouseButtons b,
      Qt::KeyboardModifiers mods = Qt::NoModifier,
      Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);
   static void handleFrameStrutMouseEvent(QWindow *w, ulong timestamp, const QPointF &local, const QPointF &global, Qt::MouseButtons b,
      Qt::KeyboardModifiers mods = Qt::NoModifier,
      Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);

   static bool handleShortcutEvent(QWindow *w, ulong timestamp, int k, Qt::KeyboardModifiers mods, quint32 nativeScanCode,
      quint32 nativeVirtualKey, quint32 nativeModifiers, const QString &text = QString(), bool autorep = false, ushort count = 1);

   static bool handleKeyEvent(QWindow *w, QEvent::Type t, int k, Qt::KeyboardModifiers mods, const QString &text = QString(),
      bool autorep = false, ushort count = 1);
   static bool handleKeyEvent(QWindow *w, ulong timestamp, QEvent::Type t, int k, Qt::KeyboardModifiers mods,
      const QString &text = QString(),
      bool autorep = false, ushort count = 1);

   static bool handleExtendedKeyEvent(QWindow *w, QEvent::Type type, int key, Qt::KeyboardModifiers modifiers,
      quint32 nativeScanCode, quint32 nativeVirtualKey,
      quint32 nativeModifiers,
      const QString &text = QString(), bool autorep = false,
      ushort count = 1, bool tryShortcutOverride = true);
   static bool handleExtendedKeyEvent(QWindow *w, ulong timestamp, QEvent::Type type, int key, Qt::KeyboardModifiers modifiers,
      quint32 nativeScanCode, quint32 nativeVirtualKey,
      quint32 nativeModifiers,
      const QString &text = QString(), bool autorep = false,
      ushort count = 1, bool tryShortcutOverride = true);
   static void handleWheelEvent(QWindow *w, const QPointF &local, const QPointF &global,
      QPoint pixelDelta, QPoint angleDelta,
      Qt::KeyboardModifiers mods = Qt::NoModifier,
      Qt::ScrollPhase phase = Qt::NoScrollPhase,
      Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);
   static void handleWheelEvent(QWindow *w, ulong timestamp, const QPointF &local, const QPointF &global,
      QPoint pixelDelta, QPoint angleDelta,
      Qt::KeyboardModifiers mods = Qt::NoModifier,
      Qt::ScrollPhase phase = Qt::NoScrollPhase,
      Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);

   // Wheel event compatibility functions. Will be removed: do not use.
   static void handleWheelEvent(QWindow *w, const QPointF &local, const QPointF &global, int d, Qt::Orientation o,
      Qt::KeyboardModifiers mods = Qt::NoModifier);
   static void handleWheelEvent(QWindow *w, ulong timestamp, const QPointF &local, const QPointF &global, int d, Qt::Orientation o,
      Qt::KeyboardModifiers mods = Qt::NoModifier);

   struct TouchPoint {
      TouchPoint() : id(0), pressure(0), state(Qt::TouchPointStationary) { }
      int id;                 // for application use
      QPointF normalPosition; // touch device coordinates, (0 to 1, 0 to 1)
      QRectF area;            // the touched area, centered at position in screen coordinates
      qreal pressure;         // 0 to 1
      Qt::TouchPointState state; //Qt::TouchPoint{Pressed|Moved|Stationary|Released}
      QVector2D velocity;     // in screen coordinate system, pixels / seconds
      QTouchEvent::TouchPoint::InfoFlags flags;
      QVector<QPointF> rawPositions; // in screen coordinates
   };

   static void registerTouchDevice(const QTouchDevice *device);
   static void unregisterTouchDevice(const QTouchDevice *device);
   static bool isTouchDeviceRegistered(const QTouchDevice *device);
   static void handleTouchEvent(QWindow *w, QTouchDevice *device,
      const QList<struct TouchPoint> &points, Qt::KeyboardModifiers mods = Qt::NoModifier);
   static void handleTouchEvent(QWindow *w, ulong timestamp, QTouchDevice *device,
      const QList<struct TouchPoint> &points, Qt::KeyboardModifiers mods = Qt::NoModifier);
   static void handleTouchCancelEvent(QWindow *w, QTouchDevice *device, Qt::KeyboardModifiers mods = Qt::NoModifier);
   static void handleTouchCancelEvent(QWindow *w, ulong timestamp, QTouchDevice *device, Qt::KeyboardModifiers mods = Qt::NoModifier);

   // rect is relative to parent
   static void handleGeometryChange(QWindow *w, const QRect &newRect, const QRect &oldRect = QRect());
   static void handleCloseEvent(QWindow *w, bool *accepted = nullptr);
   static void handleEnterEvent(QWindow *w, const QPointF &local = QPointF(), const QPointF &global = QPointF());
   static void handleLeaveEvent(QWindow *w);
   static void handleEnterLeaveEvent(QWindow *enter, QWindow *leave, const QPointF &local = QPointF(), const QPointF &global = QPointF());
   static void handleWindowActivated(QWindow *w, Qt::FocusReason r = Qt::OtherFocusReason);

   static void handleWindowStateChanged(QWindow *w, Qt::WindowState newState);
   static void handleWindowScreenChanged(QWindow *w, QScreen *newScreen);

   static void handleApplicationStateChanged(Qt::ApplicationState newState, bool forcePropagate = false);

   // region is in local coordinates, do not confuse with geometry which is parent-relative
   static void handleExposeEvent(QWindow *tlw, const QRegion &region);

#ifndef QT_NO_DRAGANDDROP
   // Drag and drop. These events are sent immediately.
   static QPlatformDragQtResponse handleDrag(QWindow *w, const QMimeData *dropData, const QPoint &p, Qt::DropActions supportedActions);
   static QPlatformDropQtResponse handleDrop(QWindow *w, const QMimeData *dropData, const QPoint &p, Qt::DropActions supportedActions);
#endif

   static bool handleNativeEvent(QWindow *window, const QByteArray &eventType, void *message, long *result);

   // Changes to the screen
   static void handleScreenOrientationChange(QScreen *screen, Qt::ScreenOrientation newOrientation);
   static void handleScreenGeometryChange(QScreen *screen, const QRect &newGeometry, const QRect &newAvailableGeometry);
   static void handleScreenLogicalDotsPerInchChange(QScreen *screen, qreal newDpiX, qreal newDpiY);
   static void handleScreenRefreshRateChange(QScreen *screen, qreal newRefreshRate);

   static void handleThemeChange(QWindow *tlw);

   static void handleFileOpenEvent(const QString &fileName);
   static void handleFileOpenEvent(const QUrl &url);

   static void handleTabletEvent(QWindow *w, ulong timestamp, const QPointF &local, const QPointF &global,
      int device, int pointerType, Qt::MouseButtons buttons, qreal pressure, int xTilt, int yTilt,
      qreal tangentialPressure, qreal rotation, int z, qint64 uid,
      Qt::KeyboardModifiers modifiers = Qt::NoModifier);

   static void handleTabletEvent(QWindow *w, const QPointF &local, const QPointF &global,
      int device, int pointerType, Qt::MouseButtons buttons, qreal pressure, int xTilt, int yTilt,
      qreal tangentialPressure, qreal rotation, int z, qint64 uid,
      Qt::KeyboardModifiers modifiers = Qt::NoModifier);

   static void handleTabletEvent(QWindow *w, ulong timestamp, bool down, const QPointF &local, const QPointF &global,
      int device, int pointerType, qreal pressure, int xTilt, int yTilt,
      qreal tangentialPressure, qreal rotation, int z, qint64 uid,
      Qt::KeyboardModifiers modifiers = Qt::NoModifier); // TODO: consider removing

   static void handleTabletEvent(QWindow *w, bool down, const QPointF &local, const QPointF &global,
      int device, int pointerType, qreal pressure, int xTilt, int yTilt,
      qreal tangentialPressure, qreal rotation, int z, qint64 uid,
      Qt::KeyboardModifiers modifiers = Qt::NoModifier); // TODO: consider removing

   static void handleTabletEnterProximityEvent(ulong timestamp, int device, int pointerType, qint64 uid);
   static void handleTabletEnterProximityEvent(int device, int pointerType, qint64 uid);
   static void handleTabletLeaveProximityEvent(ulong timestamp, int device, int pointerType, qint64 uid);
   static void handleTabletLeaveProximityEvent(int device, int pointerType, qint64 uid);

#ifndef QT_NO_GESTURES
   static void handleGestureEvent(QWindow *window,  ulong timestamp, Qt::NativeGestureType type,
      QPointF &local, QPointF &global);
   static void handleGestureEventWithRealValue(QWindow *window,  ulong timestamp, Qt::NativeGestureType type,
      qreal value, QPointF &local, QPointF &global);
   static void handleGestureEventWithSequenceIdAndValue(QWindow *window, ulong timestamp, Qt::NativeGestureType type,
      ulong sequenceId, quint64 value, QPointF &local, QPointF &global);
#endif // QT_NO_GESTURES

   static void handlePlatformPanelEvent(QWindow *w);

#ifndef QT_NO_CONTEXTMENU
   static void handleContextMenuEvent(QWindow *w, bool mouseTriggered,
      const QPoint &pos, const QPoint &globalPos,
      Qt::KeyboardModifiers modifiers);
#endif

#ifndef QT_NO_WHATSTHIS
   static void handleEnterWhatsThisEvent();
#endif

   // For event dispatcher implementations
   static bool sendWindowSystemEvents(QEventLoop::ProcessEventsFlags flags);
   static void setSynchronousWindowSystemEvents(bool enable);
   static bool flushWindowSystemEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
   static void deferredFlushWindowSystemEvents(QEventLoop::ProcessEventsFlags flags);
   static int windowSystemEventsQueued();
};

Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QWindowSystemInterface::TouchPoint &p);

#endif
