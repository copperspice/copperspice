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

#include <qwayland_inputdevice_p.h>

#include <qapplication.h>
#include <qplatform_window.h>
#include <qwayland_xkb.h>

#include <qpixmap_raster_p.h>
#include <qwayland_buffer_p.h>
#include <qwayland_cursor_p.h>
#include <qwayland_data_device_p.h>
#include <qwayland_data_devicemanager_p.h>
#include <qwayland_integration_p.h>
#include <qwayland_screen_p.h>
#include <qwayland_shm_backingstore_p.h>
#include <qwayland_touch_p.h>

#include <unistd.h>
#include <sys/mman.h>

namespace QtWaylandClient {

class EnterEvent : public QWaylandPointerEvent
{
 public:
   EnterEvent(const QPointF &l, const QPointF &g)
      : QWaylandPointerEvent(QWaylandPointerEvent::Enter, 0, l, g, nullptr, Qt::NoModifier)
   { }
};

class MotionEvent : public QWaylandPointerEvent
{
 public:
   MotionEvent(ulong t, const QPointF &l, const QPointF &g, Qt::MouseButtons b, Qt::KeyboardModifiers m)
      : QWaylandPointerEvent(QWaylandPointerEvent::Motion, t, l, g, b, m) {
   }
};

class WheelEvent : public QWaylandPointerEvent
{
 public:
   WheelEvent(ulong t, const QPointF &l, const QPointF &g, const QPoint &pd, const QPoint &ad)
      : QWaylandPointerEvent(QWaylandPointerEvent::Wheel, t, l, g, pd, ad) {
   }
};

// **
QWaylandInputDevice::QWaylandInputDevice(QWaylandDisplay *display, int version, uint32_t id)
   : QtWayland::wl_seat(nullptr, id, qMin(version, 3)),
     m_version(qMin(version, 3)), m_caps(0), m_time(0), m_serial(0),
     m_display(nullptr), m_dataDevice(nullptr), m_touchDevice(nullptr),
     m_keyboard(nullptr), m_pointer(nullptr), m_touch(nullptr),
     m_wl_display(nullptr), m_wl_pointerSurface(nullptr)
{
   // pending implementation
}

QWaylandInputDevice::~QWaylandInputDevice()
{
   delete m_pointer;
   delete m_keyboard;
   delete m_touch;
}

QPointF QWaylandInputDevice::cursorPosition() const
{
   if (m_pointer != nullptr) {
      return m_pointer->m_surfacePos;
   }

   return QPointF();
}

void QWaylandInputDevice::handleTouchPoint(int id, double x, double y, Qt::TouchPointState state)
{
   QWindowSystemInterface::TouchPoint tp;

   // Find out the coordinates for Released events
   bool coordsOk = false;

   if (state == Qt::TouchPointReleased)
      for (auto item : m_touch->m_prevTouchPoints) {
         if (item.id == id) {
            tp.area  = item.area;
            coordsOk = true;
            break;
         }
      }

   if (! coordsOk) {
      // pending implementation
   }

   tp.state = state;
   tp.id    = id;

   tp.pressure = tp.state == Qt::TouchPointReleased ? 0 : 1;
   m_touch->m_touchPoints.append(tp);
}

void QWaylandInputDevice::seat_capabilities(uint32_t caps)
{
   m_caps = caps;

   if ((m_caps & WL_SEAT_CAPABILITY_KEYBOARD) && m_keyboard == nullptr) {
      m_keyboard = createKeyboard(this);
      m_keyboard->init(get_keyboard());

   } else if (! (m_caps & WL_SEAT_CAPABILITY_KEYBOARD) && m_keyboard != nullptr) {
      delete m_keyboard;
      m_keyboard = nullptr;
   }

   // pending implementation

   if (m_caps & WL_SEAT_CAPABILITY_TOUCH && m_touch == nullptr) {
      m_touch = createTouch(this);
      m_touch->init(get_touch());

      if (m_touchDevice == nullptr) {
         m_touchDevice = new QTouchDevice;
         m_touchDevice->setType(QTouchDevice::TouchScreen);
         m_touchDevice->setCapabilities(QTouchDevice::Position);

         QWindowSystemInterface::registerTouchDevice(m_touchDevice);
      }

   } else if (! (m_caps & WL_SEAT_CAPABILITY_TOUCH) && m_touch != nullptr) {
      delete m_touch;
      m_touch = nullptr;
   }
}

QWaylandInputDevice::Keyboard *QWaylandInputDevice::createKeyboard(QWaylandInputDevice *device)
{
   return new Keyboard(device);
}

QWaylandInputDevice::Pointer *QWaylandInputDevice::createPointer(QWaylandInputDevice *device)
{
   return new Pointer(device);
}

QWaylandInputDevice::Touch *QWaylandInputDevice::createTouch(QWaylandInputDevice *device)
{
   return new Touch(device);
}

void QWaylandInputDevice::handleWindowDestroyed(QWaylandWindow *window)
{
   if (m_pointer && window == m_pointer->m_focus) {
      m_pointer->m_focus = nullptr;
   }

   if (m_keyboard && window == m_keyboard->m_focus) {
      m_keyboard->m_focus = nullptr;
      m_keyboard->stopRepeat();
   }

   if (m_touch && window == m_touch->m_focus) {
      m_touch->m_focus = nullptr;
   }
}

void QWaylandInputDevice::handleEndDrag()
{
   if (m_touch != nullptr) {
      m_touch->releasePoints();
   }

   if (m_pointer != nullptr) {
      m_pointer->releaseButtons();
   }
}

void QWaylandInputDevice::setDataDevice(QWaylandDataDevice *device)
{
   m_dataDevice = device;
}

QWaylandDataDevice *QWaylandInputDevice::dataDevice() const
{
   Q_ASSERT(m_dataDevice);
   return m_dataDevice;
}

void QWaylandInputDevice::removeMouseButtonFromState(Qt::MouseButton button)
{
   if (m_pointer != nullptr) {
      m_pointer->m_buttons = m_pointer->m_buttons & ! button;
   }
}

QWaylandWindow *QWaylandInputDevice::pointerFocus() const
{
   return m_pointer != nullptr ? m_pointer->m_focus : nullptr;
}

QWaylandWindow *QWaylandInputDevice::keyboardFocus() const
{
   return m_keyboard != nullptr ? m_keyboard->m_focus : nullptr;
}

QWaylandWindow *QWaylandInputDevice::touchFocus() const
{
   return m_touch != nullptr ? m_touch->m_focus : nullptr;
}

Qt::KeyboardModifiers QWaylandInputDevice::modifiers() const
{
   if (m_keyboard == nullptr) {
      return Qt::NoModifier;
   }

   return m_keyboard->modifiers();
}

uint32_t QWaylandInputDevice::cursorSerial() const
{
   if (m_pointer != nullptr) {
      return m_pointer->m_cursorSerial;
   }

   return 0;
}

void QWaylandInputDevice::setCursor(Qt::CursorShape newShape, QWaylandScreen *screen)
{
   struct wl_cursor_image *image = screen->waylandCursor()->cursorImage(newShape);

   if (image == nullptr) {
      return;
   }

   struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);

   setCursor(buffer, image, screen->devicePixelRatio());
}

void QWaylandInputDevice::setCursor(const QCursor &cursor, QWaylandScreen *screen)
{
   if (m_pointer->m_cursorSerial >= m_pointer->m_enterSerial && (cursor.shape() != Qt::BitmapCursor && cursor.shape() == m_pointer->m_cursorShape)) {
      return;
   }

   m_pointer->m_cursorShape = cursor.shape();

   if (cursor.shape() == Qt::BitmapCursor) {
      setCursor(screen->waylandCursor()->cursorBitmapImage(&cursor), cursor.hotSpot(), screen->devicePixelRatio());
      return;
   }

   setCursor(cursor.shape(), screen);
}

void QWaylandInputDevice::setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image, qreal ratio)
{
   if (image == nullptr) {
      setCursor(buffer, QPoint(), QSize(), ratio);

   } else {
      QPoint hotspot = QPoint(image->hotspot_x, image->hotspot_y) / ratio;
      QSize size     = QSize(image->width, image->height) / ratio;

      setCursor(buffer, hotspot, size, ratio);
   }
}

void QWaylandInputDevice::setCursor(struct wl_buffer *buffer, const QPoint &hotSpot, const QSize &size, qreal ratio)
{
   if (m_caps & WL_SEAT_CAPABILITY_POINTER) {

      bool force = m_pointer->m_enterSerial > m_pointer->m_cursorSerial;

      if (! force && m_pointer->m_cursorBuffer == buffer) {
         return;
      }

      m_pixmapCursor.clear();
      m_pointer->m_cursorSerial = m_pointer->m_enterSerial;
      m_pointer->m_cursorBuffer = buffer;

      // hide cursor
      if (buffer != nullptr) {
         m_pointer->set_cursor(m_pointer->m_enterSerial, nullptr, 0, 0);
         return;
      }

      m_pointer->set_cursor(m_pointer->m_enterSerial, m_wl_pointerSurface, hotSpot.x(), hotSpot.y());
      wl_surface_attach(m_wl_pointerSurface, buffer, 0, 0);

      // pending implementation

      wl_surface_damage(m_wl_pointerSurface, 0, 0, size.width(), size.height());
      wl_surface_commit(m_wl_pointerSurface);
   }
}

void QWaylandInputDevice::setCursor(const QSharedPointer<QWaylandBuffer> &buffer, const QPoint &hotSpot, qreal ratio)
{
   setCursor(buffer->buffer(), hotSpot, buffer->size(), ratio);
   m_pixmapCursor = buffer;
}

// ** begin QWaylandInputDevice::Keyboard

QWaylandInputDevice::Keyboard::Keyboard(QWaylandInputDevice *p)
   : m_repeatKey(0), m_nativeModifiers(0), m_repeatCode(0), m_repeatTime(0),
     m_parent(p), m_focus(nullptr)

#ifndef QT_NO_WAYLAND_XKB
     , m_xkbContext(nullptr), m_xkbMap(nullptr), m_xkbState(nullptr)
#endif
{
   connect(&m_repeatTimer, &QTimer::timeout, this, &QWaylandInputDevice::Keyboard::repeatKey);
}

QWaylandInputDevice::Keyboard::~Keyboard()
{
#ifndef QT_NO_WAYLAND_XKB
   releaseKeyMap();
#endif

   if (m_focus != nullptr) {
      QWindowSystemInterface::handleWindowActivated(nullptr);
   }

   if (m_parent->m_version >= 3) {
      wl_keyboard_release(object());
   } else {
      wl_keyboard_destroy(object());
   }
}

#ifndef QT_NO_WAYLAND_XKB
bool QWaylandInputDevice::Keyboard::createDefaultKeyMap()
{
   if (m_xkbContext && m_xkbMap && m_xkbState) {
      return true;
   }

   xkb_rule_names names;
   names.rules   = strdup("evdev");
   names.model   = strdup("pc105");
   names.layout  = strdup("us");
   names.variant = strdup("");
   names.options = strdup("");

   m_xkbContext = xkb_context_new(xkb_context_flags(0));

   if (m_xkbContext) {
      m_xkbMap = xkb_map_new_from_names(m_xkbContext, &names, xkb_map_compile_flags(0));

      if (m_xkbMap) {
         m_xkbState = xkb_state_new(m_xkbMap);
      }
   }

   if (! m_xkbContext || ! m_xkbMap || ! m_xkbState) {
      qWarning("QWaylandInputDevice::Keyboard::createDefaultKeyMap()  Call to xkb_map_new_from_names() failed, no key input");
      return false;
   }

   return true;
}

void QWaylandInputDevice::Keyboard::releaseKeyMap()
{
   if (m_xkbState) {
      xkb_state_unref(m_xkbState);
   }

   if (m_xkbMap) {
      xkb_map_unref(m_xkbMap);
   }

   if (m_xkbContext) {
      xkb_context_unref(m_xkbContext);
   }
}
#endif

void QWaylandInputDevice::Keyboard::stopRepeat()
{
   m_repeatTimer.stop();
}

void QWaylandInputDevice::Keyboard::keyboard_keymap(uint32_t format, int32_t fd, uint32_t size)
{
#ifndef QT_NO_WAYLAND_XKB

   if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
      close(fd);
      return;
   }

   char *map_str = (char *)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

   if (map_str == MAP_FAILED) {
      close(fd);
      return;
   }

   // release keymap resources if they were already created
   releaseKeyMap();

   m_xkbContext = xkb_context_new(xkb_context_flags(0));
   m_xkbMap     = xkb_map_new_from_string(m_xkbContext, map_str, XKB_KEYMAP_FORMAT_TEXT_V1, (xkb_keymap_compile_flags)0);

   munmap(map_str, size);
   close(fd);

   m_xkbState = xkb_state_new(m_xkbMap);
#else
   (void) format;
   (void) fd;
   (void) size;
#endif
}

void QWaylandInputDevice::Keyboard::keyboard_enter(uint32_t time, struct wl_surface *surface, struct wl_array *keys)
{
   (void) time;
   (void) keys;

   if (surface == nullptr) {
      return;
   }

   // pending implementation
}

void QWaylandInputDevice::Keyboard::keyboard_leave(uint32_t time, struct wl_surface *surface)
{
   (void) time;
   (void) surface;

   // pending implementation
}

void QWaylandInputDevice::Keyboard::keyboard_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
   // pending implementation
}

Qt::KeyboardModifiers QWaylandInputDevice::Keyboard::modifiers() const
{
   Qt::KeyboardModifiers retval = Qt::NoModifier;

#ifndef QT_NO_WAYLAND_XKB
   if (! m_xkbState) {
      return retval;
   }

   retval = QWaylandXkb::modifiers(m_xkbState);
#endif

   return retval;
}

void QWaylandInputDevice::Keyboard::repeatKey()
{
   // pending implementation
}

void QWaylandInputDevice::Keyboard::keyboard_modifiers(uint32_t serial,
      uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
   (void) serial;

#ifndef QT_NO_WAYLAND_XKB

   if (m_xkbState) {
      xkb_state_update_mask(m_xkbState, mods_depressed, mods_latched, mods_locked, 0, 0, group);
   }

   m_nativeModifiers = mods_depressed | mods_latched | mods_locked;

#else
   (void) serial;
   (void) mods_depressed;
   (void) mods_latched;
   (void) mods_locked;
   (void) group;
#endif
}

// ** begin QWaylandInputDevice::Pointer

QWaylandInputDevice::Pointer::Pointer(QWaylandInputDevice *p)
   : m_enterSerial(0), m_cursorSerial(0), m_parent(p), m_focus(nullptr), m_cursorBuffer(nullptr), m_buttons(Qt::EmptyFlag)
{
}

QWaylandInputDevice::Pointer::~Pointer()
{
   if (m_parent->m_version >= 3) {
      wl_pointer_release(object());
   } else {
      wl_pointer_destroy(object());
   }
}

void QWaylandInputDevice::Pointer::pointer_enter(uint32_t serial, struct wl_surface *surface,
      wl_fixed_t sx, wl_fixed_t sy)
{
   if (! surface) {
      return;
   }

   // pending implementation
}

void QWaylandInputDevice::Pointer::pointer_leave(uint32_t time, struct wl_surface *surface)
{
   // The event may arrive after destroying the window, indicated by a null surface.
   if (surface == nullptr) {
      return;
   }

   // pending implementation

   m_focus   = nullptr;
   m_buttons = Qt::NoButton;

   m_parent->m_time = time;
}

void QWaylandInputDevice::Pointer::pointer_motion(uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
   // pending implementation
}

void QWaylandInputDevice::Pointer::pointer_button(uint32_t serial, uint32_t time,
      uint32_t button, uint32_t state)
{
   // pending implementation

   Qt::MouseButton qt_button;

   // translate from kernel (input.h) 'button' to corresponding Qt:MouseButton.
   // The range of mouse values is 0x110 <= mouse_button < 0x120, the first Joystick button.
   switch (button) {
      case 0x110:
         qt_button = Qt::LeftButton;
         break;    // kernel BTN_LEFT

      case 0x111:
         qt_button = Qt::RightButton;
         break;

      case 0x112:
         qt_button = Qt::MiddleButton;
         break;

      case 0x113:
         qt_button = Qt::ExtraButton1;
         break;  // AKA Qt::BackButton

      case 0x114:
         qt_button = Qt::ExtraButton2;
         break;  // AKA Qt::ForwardButton

      case 0x115:
         qt_button = Qt::ExtraButton3;
         break;  // AKA Qt::TaskButton

      case 0x116:
         qt_button = Qt::ExtraButton4;
         break;

      case 0x117:
         qt_button = Qt::ExtraButton5;
         break;

      case 0x118:
         qt_button = Qt::ExtraButton6;
         break;

      case 0x119:
         qt_button = Qt::ExtraButton7;
         break;

      case 0x11a:
         qt_button = Qt::ExtraButton8;
         break;

      case 0x11b:
         qt_button = Qt::ExtraButton9;
         break;

      case 0x11c:
         qt_button = Qt::ExtraButton10;
         break;

      case 0x11d:
         qt_button = Qt::ExtraButton11;
         break;

      case 0x11e:
         qt_button = Qt::ExtraButton12;
         break;

      case 0x11f:
         qt_button = Qt::ExtraButton13;
         break;

      default:
         return; // invalid button number
   }

   // pending implementation
}

void QWaylandInputDevice::Pointer::releaseButtons()
{
   // pending implementation
}

void QWaylandInputDevice::Pointer::pointer_axis(uint32_t time, uint32_t axis, int32_t value)
{

   // pending implementation
}

// ** begin QWaylandInputDevice::Touch

QWaylandInputDevice::Touch::Touch(QWaylandInputDevice *p)
   : m_parent(p), m_focus(nullptr)
{
}

QWaylandInputDevice::Touch::~Touch()
{
   if (m_parent->m_version >= 3) {
      wl_touch_release(object());
   } else {
      wl_touch_destroy(object());
   }
}

void QWaylandInputDevice::Touch::touch_down(uint32_t serial, uint32_t time, struct wl_surface *surface,
      int32_t id, wl_fixed_t x, wl_fixed_t y)
{
   if (surface == nullptr) {
      return;
   }

   m_parent->m_time   = time;
   m_parent->m_serial = serial;

   // pending implementation
}

void QWaylandInputDevice::Touch::touch_up(uint32_t serial, uint32_t time, int32_t id)
{
   (void) serial;
   (void) time;

   m_focus = nullptr;
   m_parent->handleTouchPoint(id, 0, 0, Qt::TouchPointReleased);

   // As of Weston 1.5.90 there is no touch_frame after the last touch_up
   // (i.e. when the last finger is released). To accommodate for this, issue a
   // touch_frame. This cannot hurt since it is safe to call the touch_frame
   // handler multiple times when there are no points left.
   if (allTouchPointsReleased()) {
      touch_frame();
   }
}

void QWaylandInputDevice::Touch::touch_motion(uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
   (void) time;
   m_parent->handleTouchPoint(id, wl_fixed_to_double(x), wl_fixed_to_double(y), Qt::TouchPointMoved);
}

void QWaylandInputDevice::Touch::touch_cancel()
{
   m_prevTouchPoints.clear();
   m_touchPoints.clear();

   // pending implementation

}

bool QWaylandInputDevice::Touch::allTouchPointsReleased()
{
   for (auto item : m_touchPoints) {
      if (item.state != Qt::TouchPointReleased) {
         return false;
      }
   }

   return true;
}

void QWaylandInputDevice::Touch::releasePoints()
{
   for (const QWindowSystemInterface::TouchPoint &previousPoint : m_prevTouchPoints) {
      QWindowSystemInterface::TouchPoint tp = previousPoint;
      tp.state = Qt::TouchPointReleased;
      m_touchPoints.append(tp);
   }

   touch_frame();
}

void QWaylandInputDevice::Touch::touch_frame()
{
   // copy all points which are in the previous but not in the current list, as stationary.
   for (int i = 0; i < m_prevTouchPoints.count(); ++i) {
      const QWindowSystemInterface::TouchPoint &prevPoint(m_prevTouchPoints.at(i));

      if (prevPoint.state == Qt::TouchPointReleased) {
         continue;
      }

      bool found = false;

      for (int j = 0; j < m_touchPoints.count(); ++j)
         if (m_touchPoints.at(j).id == prevPoint.id) {
            found = true;
            break;
         }

      if (! found) {
         QWindowSystemInterface::TouchPoint p = prevPoint;
         p.state = Qt::TouchPointStationary;
         m_touchPoints.append(p);
      }
   }

   if (m_touchPoints.isEmpty()) {
      m_prevTouchPoints.clear();
      return;
   }

   // pending implementation
}

}
