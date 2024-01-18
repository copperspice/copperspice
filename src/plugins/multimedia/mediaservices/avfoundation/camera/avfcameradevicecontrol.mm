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

#include "avfcameradebug.h"
#include "avfcameradevicecontrol.h"
#include "avfcameraservice.h"
#include "avfcamerasession.h"

AVFCameraDeviceControl::AVFCameraDeviceControl(AVFCameraService *service, QObject *parent)
   : QVideoDeviceSelectorControl(parent)
   , m_service(service)
   , m_selectedDevice(0)
   , m_dirty(true)
{
    (void) m_service;
}

AVFCameraDeviceControl::~AVFCameraDeviceControl()
{
}

int AVFCameraDeviceControl::deviceCount() const
{
    return AVFCameraSession::availableCameraDevices().count();
}

QString AVFCameraDeviceControl::deviceName(int index) const
{
    const QList<AVFCameraInfo> &devices = AVFCameraSession::availableCameraDevices();
    if (index < 0 || index >= devices.count())
        return QString();

    return devices.at(index).deviceId;
}

QString AVFCameraDeviceControl::deviceDescription(int index) const
{
    const QList<AVFCameraInfo> &devices = AVFCameraSession::availableCameraDevices();
    if (index < 0 || index >= devices.count())
        return QString();

    return devices.at(index).description;
}

int AVFCameraDeviceControl::defaultDevice() const
{
    return AVFCameraSession::defaultCameraIndex();
}

int AVFCameraDeviceControl::selectedDevice() const
{
    return m_selectedDevice;
}

void AVFCameraDeviceControl::setSelectedDevice(int index)
{
    if (index >= 0 && index < deviceCount() && index != m_selectedDevice) {
        m_dirty = true;
        m_selectedDevice = index;
        Q_EMIT selectedDeviceChanged(index);
        Q_EMIT selectedDeviceChanged(deviceName(index));
    }
}

AVCaptureDevice *AVFCameraDeviceControl::createCaptureDevice()
{
    m_dirty = false;
    AVCaptureDevice *device = nullptr;

    QString deviceId = deviceName(m_selectedDevice);

    if (! deviceId.isEmpty()) {
        device = [AVCaptureDevice deviceWithUniqueID:
                    [NSString stringWithUTF8String:
                        deviceId.toUtf8().constData()]];
    }

    if (! device)
        device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];

    return device;
}
