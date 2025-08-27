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

#ifndef QWAYLAND_TOUCH_H
#define QWAYLAND_TOUCH_H

#include <qwindowsysteminterface.h>

#include <qwayland-touch-extension.h>

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandInputDevice;

class Q_WAYLAND_CLIENT_EXPORT QWaylandTouchExtension : public QtWayland::qt_touch_extension
{
 public:
   QWaylandTouchExtension(QWaylandDisplay *display, uint32_t id);

   void touchCanceled();

 private:
   void registerDevice(int caps);

   void sendTouchEvent();

   void touch_extension_touch(uint32_t time, uint32_t id, uint32_t state, int32_t x, int32_t y,
         int32_t normalized_x, int32_t normalized_y, int32_t width, int32_t height,
         uint32_t pressure, int32_t velocity_x, int32_t velocity_y, uint32_t flags, struct wl_array *rawdata) override;

   void touch_extension_configure(uint32_t flags) override;

   QList<QWindowSystemInterface::TouchPoint> m_touchPoints;
   QList<QWindowSystemInterface::TouchPoint> m_prevTouchPoints;

   QWaylandDisplay *m_display;
   QTouchDevice *m_touchDevice;
   QWindow *m_targetWindow;
   QWaylandInputDevice *m_inputDevice;

   QPointF m_lastMouseLocal;
   QPointF m_lastMouseGlobal;

   uint32_t m_timestamp;
   uint32_t m_flags;
   int m_pointsLeft;
   int m_mouseSourceId;
};

}

#endif
