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

#ifndef QAUDIODEVICEFACTORY_P_H
#define QAUDIODEVICEFACTORY_P_H

#include <qglobal.h>
#include <qlist.h>
#include <qmultimedia.h>
#include <qaudiodeviceinfo.h>

class QAbstractAudioInput;
class QAbstractAudioOutput;
class QAbstractAudioDeviceInfo;

class QAudioDeviceFactory
{
 public:
   static QList<QAudioDeviceInfo> availableDevices(QAudio::Mode mode);

   static QAudioDeviceInfo defaultInputDevice();
   static QAudioDeviceInfo defaultOutputDevice();

   static QAbstractAudioDeviceInfo *audioDeviceInfo(const QString &realm, const QString &handle, QAudio::Mode mode);

   static QAbstractAudioInput *createDefaultInputDevice(QAudioFormat const &format);
   static QAbstractAudioOutput *createDefaultOutputDevice(QAudioFormat const &format);

   static QAbstractAudioInput *createInputDevice(QAudioDeviceInfo const &device, QAudioFormat const &format);
   static QAbstractAudioOutput *createOutputDevice(QAudioDeviceInfo const &device, QAudioFormat const &format);

   static QAbstractAudioInput *createNullInput();
   static QAbstractAudioOutput *createNullOutput();
};

#endif

