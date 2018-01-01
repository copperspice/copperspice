/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QT7_AUDIODEVICE_H
#define QT7_AUDIODEVICE_H

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include <QtCore/QList>
#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class AudioDevice
    {
        public:
            enum Scope {In, Out};

            static QList<AudioDeviceID> devices(Scope scope);
            static AudioDeviceID defaultDevice(Scope scope);
            static AudioDeviceID defaultSystemDevice(Scope scope);
            static AudioDeviceID currentDevice(AudioUnit unit, Scope scope);
            static bool setDevice(AudioUnit unit, AudioDeviceID deviceID, Scope scope);
            static QString deviceName(AudioDeviceID deviceId);
            static QString deviceSourceName(AudioDeviceID deviceID);
            static QString deviceSourceNameElseDeviceName(AudioDeviceID deviceID);
            static QString deviceNameElseDeviceSourceName(AudioDeviceID deviceID);
            static QString deviceUID(AudioDeviceID deviceID);
    };

}} //namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_AUDIODEVICE_H
