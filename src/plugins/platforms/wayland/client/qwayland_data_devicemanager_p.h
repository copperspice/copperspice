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

#ifndef QWAYLAND_DATA_DEVICEMANAGER_H
#define QWAYLAND_DATA_DEVICEMANAGER_H

#include <qwayland-wayland.h>

namespace QtWaylandClient {

class QWaylandDataDevice;
class QWaylandDataSource;
class QWaylandDisplay;
class QWaylandInputDevice;

class Q_WAYLAND_CLIENT_EXPORT QWaylandDataDeviceManager : public QtWayland::wl_data_device_manager
{
 public:
   QWaylandDataDeviceManager(QWaylandDisplay *display, uint32_t id);
   ~QWaylandDataDeviceManager();

   QWaylandDataDevice *getDataDevice(QWaylandInputDevice *inputDevice);

   QWaylandDisplay *display() const;

 private:
   QWaylandDisplay *m_display;
};

}

#endif
