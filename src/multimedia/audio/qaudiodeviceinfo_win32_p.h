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

#ifndef QAUDIODEVICEINFO_WIN32_P_H
#define QAUDIODEVICEINFO_WIN32_P_H

#include <QtCore/qbytearray.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qlist.h>
#include <QtCore/qdebug.h>

#include <QtMultimedia/qaudiodeviceinfo.h>
#include <QtMultimedia/qaudioengine.h>


QT_BEGIN_NAMESPACE

const unsigned int MAX_SAMPLE_RATES = 5;
const unsigned int SAMPLE_RATES[] = { 8000, 11025, 22050, 44100, 48000 };

class QAudioDeviceInfoInternal : public QAbstractAudioDeviceInfo
{
   MULTI_CS_OBJECT(QAudioDeviceInfoInternal)

 public:
   QAudioDeviceInfoInternal(QByteArray dev, QAudio::Mode mode);
   ~QAudioDeviceInfoInternal();

   bool open();
   void close();

   bool testSettings(const QAudioFormat &format) const;
   void updateLists();
   QAudioFormat preferredFormat() const override;
   bool isFormatSupported(const QAudioFormat &format) const override;
   QAudioFormat nearestFormat(const QAudioFormat &format) const override;
   QString deviceName() const override;
   QStringList codecList() override;
   QList<int> frequencyList() override;
   QList<int> channelsList() override;
   QList<int> sampleSizeList() override;
   QList<QAudioFormat::Endian> byteOrderList() override;
   QList<QAudioFormat::SampleType> sampleTypeList() override;

   static QByteArray defaultInputDevice();
   static QByteArray defaultOutputDevice();
   static QList<QByteArray> availableDevices(QAudio::Mode);

 private:
   QAudio::Mode mode;
   QString device;
   QAudioFormat nearest;
   QList<int> freqz;
   QList<int> channelz;
   QList<int> sizez;
   QList<QAudioFormat::Endian> byteOrderz;
   QStringList codecz;
   QList<QAudioFormat::SampleType> typez;
};

QT_END_NAMESPACE

#endif
