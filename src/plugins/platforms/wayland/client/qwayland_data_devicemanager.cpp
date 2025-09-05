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

#include <qwayland_data_devicemanager_p.h>

#include <qwayland_data_device_p.h>
#include <qwayland_dataoffer_p.h>
#include <qwayland_display_p.h>
#include <qwayland_inputdevice_p.h>
namespace QtWaylandClient {

QWaylandDataDeviceManager::QWaylandDataDeviceManager(QWaylandDisplay *display, uint32_t id)
   : wl_data_device_manager(display->wl_registry(), id, 1), m_display(display)
{
   // Create transfer devices for all input devices
   QList<QWaylandInputDevice *> inputDevices = m_display->inputDevices();

   for (int i = 0; i < inputDevices.size(); i++) {
      inputDevices.at(i)->setDataDevice(getDataDevice(inputDevices.at(i)));
   }
}

QWaylandDataDeviceManager::~QWaylandDataDeviceManager()
{
   wl_data_device_manager_destroy(object());
}

QWaylandDataDevice *QWaylandDataDeviceManager::getDataDevice(QWaylandInputDevice *inputDevice)
{
   return new QWaylandDataDevice(this, inputDevice);
}

QWaylandDisplay *QWaylandDataDeviceManager::display() const
{
   return m_display;
}

}

