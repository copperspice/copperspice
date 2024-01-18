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
#include "avfaudioinputselectorcontrol.h"
#include "avfcameraservice.h"

#import <AVFoundation/AVFoundation.h>

AVFAudioInputSelectorControl::AVFAudioInputSelectorControl(AVFCameraService *service, QObject *parent)
   : QAudioInputSelectorControl(parent), m_dirty(true)
{
    (void) service;
    NSArray *videoDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeAudio];
    for (AVCaptureDevice *device in videoDevices) {
        QString deviceId = QString::fromUtf8([[device uniqueID] UTF8String]);
        m_devices << deviceId;
        m_deviceDescriptions.insert(deviceId,
                                    QString::fromUtf8([[device localizedName] UTF8String]));
    }

    AVCaptureDevice *defaultDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeAudio];
    if (defaultDevice) {
        m_defaultDevice = QString::fromUtf8([defaultDevice.uniqueID UTF8String]);
        m_activeInput = m_defaultDevice;
    }
}

AVFAudioInputSelectorControl::~AVFAudioInputSelectorControl()
{
}

QList<QString> AVFAudioInputSelectorControl::availableInputs() const
{
    return m_devices;
}

QString AVFAudioInputSelectorControl::inputDescription(const QString &name) const
{
    return m_deviceDescriptions.value(name);
}

QString AVFAudioInputSelectorControl::defaultInput() const
{
    return m_defaultDevice;
}

QString AVFAudioInputSelectorControl::activeInput() const
{
    return m_activeInput;
}

void AVFAudioInputSelectorControl::setActiveInput(const QString &name)
{
    if (name != m_activeInput) {
        m_activeInput = name;
        m_dirty = true;

        Q_EMIT activeInputChanged(m_activeInput);
    }
}

AVCaptureDevice *AVFAudioInputSelectorControl::createCaptureDevice()
{
    m_dirty = false;
    AVCaptureDevice *device = nullptr;

    if (! m_activeInput.isEmpty()) {
        device = [AVCaptureDevice deviceWithUniqueID:
                    [NSString stringWithUTF8String:
                        m_activeInput.toUtf8().constData()]];
    }

    if (! device)
        device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeAudio];

    return device;
}
