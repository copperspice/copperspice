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

#ifndef QAUDIODEVICEINFO_MAC_P_H
#define QAUDIODEVICEINFO_MAC_P_H

#include <CoreAudio/CoreAudio.h>
#include <QtMultimedia/qaudioengine.h>

QT_BEGIN_NAMESPACE

class QAudioDeviceInfoInternal : public QAbstractAudioDeviceInfo
{
 public:
   AudioDeviceID   deviceId;
   QString         name;
   QAudio::Mode   mode;

   QAudioDeviceInfoInternal(QByteArray const &handle, QAudio::Mode mode);

   bool isFormatSupported(const QAudioFormat &format) const;
   QAudioFormat preferredFormat() const;
   QAudioFormat nearestFormat(const QAudioFormat &format) const;

   QString deviceName() const;

   QStringList codecList();
   QList<int> frequencyList();
   QList<int> channelsList();
   QList<int> sampleSizeList();
   QList<QAudioFormat::Endian> byteOrderList();
   QList<QAudioFormat::SampleType> sampleTypeList();

   static QByteArray defaultInputDevice();
   static QByteArray defaultOutputDevice();

   static QList<QByteArray> availableDevices(QAudio::Mode mode);
};

QT_END_NAMESPACE

#endif  // QDEVICEINFO_MAC_P_H
