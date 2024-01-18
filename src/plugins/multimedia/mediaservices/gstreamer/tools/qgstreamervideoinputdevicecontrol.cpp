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

#include <qgstreamervideoinputdevicecontrol_p.h>

#include <qdir.h>
#include <qdebug.h>

#include <qgstutils_p.h>

QGstreamerVideoInputDeviceControl::QGstreamerVideoInputDeviceControl(QObject *parent)
   : QVideoDeviceSelectorControl(parent), m_factory(nullptr), m_selectedDevice(0)
{
}

QGstreamerVideoInputDeviceControl::QGstreamerVideoInputDeviceControl(GstElementFactory *factory, QObject *parent)
   : QVideoDeviceSelectorControl(parent), m_factory(factory), m_selectedDevice(0)
{
   if (m_factory) {
      gst_object_ref(GST_OBJECT(m_factory));
   }
}

QGstreamerVideoInputDeviceControl::~QGstreamerVideoInputDeviceControl()
{
   if (m_factory) {
      gst_object_unref(GST_OBJECT(m_factory));
   }
}

int QGstreamerVideoInputDeviceControl::deviceCount() const
{
   return QGstUtils::enumerateCameras(m_factory).count();
}

QString QGstreamerVideoInputDeviceControl::deviceName(int index) const
{
   return QGstUtils::enumerateCameras(m_factory).value(index).name;
}

QString QGstreamerVideoInputDeviceControl::deviceDescription(int index) const
{
   return QGstUtils::enumerateCameras(m_factory).value(index).description;
}

int QGstreamerVideoInputDeviceControl::defaultDevice() const
{
   return 0;
}

int QGstreamerVideoInputDeviceControl::selectedDevice() const
{
   return m_selectedDevice;
}

void QGstreamerVideoInputDeviceControl::setSelectedDevice(int index)
{
   if (index != m_selectedDevice) {
      m_selectedDevice = index;
      emit selectedDeviceChanged(index);
      emit selectedDeviceChanged(deviceName(index));
   }
}
