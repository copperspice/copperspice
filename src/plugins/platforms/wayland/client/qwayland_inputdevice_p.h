/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QWAYLAND_INPUTDEVICE_H
#define QWAYLAND_INPUTDEVICE_H

#include <qobject.h>
#include <qplatform_integration.h>
#include <qplatform_screen.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qwindowsysteminterface.h>

#include <qwayland-wayland.h>
#include <wayland-client.h>
#include <wayland-cursor.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>

namespace QtWaylandClient {

class QWaylandBuffer;
class QWaylandDataDevice;
class QWaylandDisplay;
class QWaylandScreen;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandInputDevice : public QObject, public QtWayland::wl_seat
{
   CS_OBJECT(QWaylandInputDevice)

 public:
   class Keyboard;
   class Pointer;
   class Touch;

   QWaylandInputDevice(QWaylandDisplay *display, int version, uint32_t id);
   ~QWaylandInputDevice();

   uint32_t capabilities() const {
      return m_caps;
   }

   struct ::wl_seat *wl_seat() {
      return QtWayland::wl_seat::object();
   }

   QPointF cursorPosition() const;

   void setCursor(Qt::CursorShape cursor, QWaylandScreen *screen);

   void setCursor(const QCursor &cursor, QWaylandScreen *screen);
   void setCursor(struct wl_buffer *buffer, struct ::wl_cursor_image *image, qreal ratio);
   void setCursor(struct wl_buffer *buffer, const QPoint &hotSpot, const QSize &size, qreal ratio);
   void setCursor(const QSharedPointer<QWaylandBuffer> &buffer, const QPoint &hotSpot, qreal ratio);

   void handleWindowDestroyed(QWaylandWindow *window);
   void handleEndDrag();

   void setDataDevice(QWaylandDataDevice *device);
   QWaylandDataDevice *dataDevice() const;

   void removeMouseButtonFromState(Qt::MouseButton button);

   QWaylandWindow *pointerFocus() const;
   QWaylandWindow *keyboardFocus() const;
   QWaylandWindow *touchFocus() const;

   Qt::KeyboardModifiers modifiers() const;

   uint32_t serial() const;
   uint32_t cursorSerial() const;

   virtual Keyboard *createKeyboard(QWaylandInputDevice *device);
   virtual Pointer *createPointer(QWaylandInputDevice *device);
   virtual Touch *createTouch(QWaylandInputDevice *device);

 private:
   void handleTouchPoint(int id, double x, double y, Qt::TouchPointState state);
   void seat_capabilities(uint32_t caps) override;

   int m_version;

   uint32_t m_caps;
   uint32_t m_time;
   uint32_t m_serial;

   QWaylandDisplay *m_display;
   QWaylandDataDevice *m_dataDevice;
   QTouchDevice *m_touchDevice;

   QWaylandInputDevice::Keyboard *m_keyboard;
   QWaylandInputDevice::Pointer *m_pointer;
   QWaylandInputDevice::Touch *m_touch;

   QSharedPointer<QWaylandBuffer> m_pixmapCursor;

   struct wl_display *m_wl_display;
   struct wl_surface *m_wl_pointerSurface;

   friend class QWaylandTouchExtension;
   friend class QWaylandQtKeyExtension;
};

inline uint32_t QWaylandInputDevice::serial() const
{
   return m_serial;
}

class Q_WAYLAND_CLIENT_EXPORT QWaylandInputDevice::Keyboard : public QObject, public QtWayland::wl_keyboard
{
   CS_OBJECT(Keyboard)

 public:
   Keyboard(QWaylandInputDevice *p);
   virtual ~Keyboard();

   void stopRepeat();

   void keyboard_keymap(uint32_t format, int32_t fd, uint32_t size) override;
   void keyboard_enter(uint32_t time, struct wl_surface *surface, struct wl_array *keys) override;
   void keyboard_leave(uint32_t time, struct wl_surface *surface) override;

   void keyboard_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state) override;

   void keyboard_modifiers(uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
         uint32_t mods_locked, uint32_t group) override;

   Qt::KeyboardModifiers modifiers() const;

   int m_repeatKey;

   uint32_t m_nativeModifiers;
   uint32_t m_repeatCode;
   uint32_t m_repeatTime;

   QWaylandInputDevice *m_parent;
   QWaylandWindow *m_focus;

   QString m_repeatText;
   QTimer m_repeatTimer;

   xkb_context *m_xkbContext;
   xkb_keymap  *m_xkbMap;
   xkb_state   *m_xkbState;

   xkb_keysym_t m_repeatSym;

 private:
   CS_SLOT_1(Private, void repeatKey())
   CS_SLOT_2(repeatKey)

   bool createDefaultKeyMap();
   void releaseKeyMap();
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandInputDevice::Pointer : public QtWayland::wl_pointer
{
 public:
   Pointer(QWaylandInputDevice *p);
   virtual ~Pointer();

   void pointer_enter(uint32_t serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy) override;
   void pointer_leave(uint32_t time, struct wl_surface *surface) override;
   void pointer_motion(uint32_t time, wl_fixed_t sx, wl_fixed_t sy) override;
   void pointer_button(uint32_t serial, uint32_t time, uint32_t button, uint32_t state) override;
   void pointer_axis(uint32_t time, uint32_t axis, wl_fixed_t value) override;

   void releaseButtons();

   uint32_t m_enterSerial;
   uint32_t m_cursorSerial;

   QWaylandInputDevice *m_parent;
   QWaylandWindow *m_focus;

   wl_buffer *m_cursorBuffer;

   QPointF m_surfacePos;
   QPointF m_globalPos;

   Qt::MouseButtons m_buttons;
   Qt::CursorShape m_cursorShape = Qt::BitmapCursor;
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandInputDevice::Touch : public QtWayland::wl_touch
{
 public:
   Touch(QWaylandInputDevice *p);
   virtual ~Touch();

   void touch_down(uint32_t serial, uint32_t time, struct wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y) override;
   void touch_up(uint32_t serial, uint32_t time, int32_t id) override;

   void touch_motion(uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y) override;
   void touch_frame() override;
   void touch_cancel() override;

   bool allTouchPointsReleased();
   void releasePoints();

   QWaylandInputDevice *m_parent;
   QWaylandWindow *m_focus;

   QList<QWindowSystemInterface::TouchPoint> m_touchPoints;
   QList<QWindowSystemInterface::TouchPoint> m_prevTouchPoints;
};

class QWaylandPointerEvent
{
 public:
   enum EventType {
      Enter,
      Motion,
      Wheel
   };

   QWaylandPointerEvent(EventType t, ulong ts, const QPointF &l, const QPointF &g, Qt::MouseButtons b, Qt::KeyboardModifiers m)
      : m_timestamp(ts), m_type(t), m_buttons(b), m_modifiers(m), m_localPos(l), m_globalPos(g)
   { }

   QWaylandPointerEvent(EventType t, ulong ts, const QPointF &l, const QPointF &g, const QPoint &pd, const QPoint &ad)
      : m_timestamp(ts), m_type(t), m_pixelDelta(pd), m_angleDelta(ad), m_localPos(l), m_globalPos(g)
   { }

   ulong m_timestamp;

   EventType m_type;
   Qt::MouseButtons m_buttons;
   Qt::KeyboardModifiers m_modifiers;

   QPoint m_pixelDelta;
   QPoint m_angleDelta;
   QPointF m_localPos;
   QPointF m_globalPos;
};

}

#endif
