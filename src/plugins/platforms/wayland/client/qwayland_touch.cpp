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

#include <qwayland_touch_p.h>

#include <qwayland_display_p.h>
#include <qwayland_inputdevice_p.h>
#include <qwayland_window_p.h>

namespace QtWaylandClient {

QWaylandTouchExtension::QWaylandTouchExtension(QWaylandDisplay *display, uint32_t id)
   : QtWayland::qt_touch_extension(display->wl_registry(), id, 1),
     m_display(display), m_touchDevice(nullptr), m_targetWindow(nullptr), m_inputDevice(nullptr),
     m_timestamp(0), m_flags(0), m_pointsLeft(0), m_mouseSourceId(-1)
{
}

void QWaylandTouchExtension::registerDevice(int caps)
{
   m_touchDevice = new QTouchDevice;
   m_touchDevice->setType(QTouchDevice::TouchScreen);
   m_touchDevice->setCapabilities(QTouchDevice::Capabilities(caps));

   QWindowSystemInterface::registerTouchDevice(m_touchDevice);
}

static inline qreal fromFixed(int f)
{
   return f / qreal(10000);
}

void QWaylandTouchExtension::touch_extension_touch(uint32_t time, uint32_t id, uint32_t state, int32_t x, int32_t y,
      int32_t normalized_x, int32_t normalized_y, int32_t width, int32_t height, uint32_t pressure,
      int32_t velocity_x, int32_t velocity_y, uint32_t flags, wl_array *rawdata)
{
   if (m_inputDevice == nullptr) {
      QList<QWaylandInputDevice *> inputDevices = m_display->inputDevices();

      if (inputDevices.isEmpty()) {
         qWarning("QWaylandTouchExtension::touch_extension_touch() No input devices");
         return;
      }

      m_inputDevice = inputDevices.first();
   }

   // pending implementation
}

void QWaylandTouchExtension::sendTouchEvent()
{
   // Copy all points which are in the previous list but not in the current list
   for (const auto &previousItem : m_prevTouchPoints) {
      if (previousItem.state == Qt::TouchPointReleased) {
         continue;
      }

      bool found = false;

      for (const auto &item : m_touchPoints) {
         if (item.id == previousItem.id) {
            found = true;
            break;
         }
      }

      if (! found) {
         QWindowSystemInterface::TouchPoint p = previousItem;
         p.state = Qt::TouchPointStationary;
         m_touchPoints.append(p);
      }
   }

   if (m_touchPoints.isEmpty()) {
      m_prevTouchPoints.clear();
      return;
   }

   QWindowSystemInterface::handleTouchEvent(m_targetWindow, m_timestamp, m_touchDevice, m_touchPoints);

   Qt::TouchPointStates states = nullptr;

   for (const auto &item : m_touchPoints) {
      states |= item.state;
   }

   if (m_flags & QT_TOUCH_EXTENSION_FLAGS_MOUSE_FROM_TOUCH) {
      if (states == Qt::TouchPointPressed) {
         m_mouseSourceId = m_touchPoints.first().id;
      }

      for (int i = 0; i < m_touchPoints.count(); ++i) {
         const QWindowSystemInterface::TouchPoint &tp(m_touchPoints.at(i));

         if (tp.id == m_mouseSourceId) {
            Qt::MouseButtons buttons = tp.state == Qt::TouchPointReleased ? Qt::NoButton : Qt::LeftButton;
            m_lastMouseGlobal = tp.area.center();

            QPoint globalPoint = m_lastMouseGlobal.toPoint();
            QPointF delta = m_lastMouseGlobal - globalPoint;

            m_lastMouseLocal = m_targetWindow->mapFromGlobal(globalPoint) + delta;
            QWindowSystemInterface::handleMouseEvent(m_targetWindow, m_timestamp, m_lastMouseLocal, m_lastMouseGlobal, buttons);

            if (buttons == Qt::NoButton) {
               m_mouseSourceId = -1;
            }

            break;
         }
      }
   }

   m_prevTouchPoints = m_touchPoints;
   m_touchPoints.clear();

   if (states == Qt::TouchPointReleased) {
      m_prevTouchPoints.clear();
   }
}

void QWaylandTouchExtension::touchCanceled()
{
   m_touchPoints.clear();
   m_prevTouchPoints.clear();

   if (m_mouseSourceId != -1) {
      QWindowSystemInterface::handleMouseEvent(m_targetWindow, m_timestamp, m_lastMouseLocal, m_lastMouseGlobal, Qt::NoButton);
   }
}

void QWaylandTouchExtension::touch_extension_configure(uint32_t flags)
{
   m_flags = flags;
}

}
