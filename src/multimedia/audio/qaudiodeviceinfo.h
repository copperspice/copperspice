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

#ifndef QAUDIODEVICEINFO_H
#define QAUDIODEVICEINFO_H

#include <QtCore/qobject.h>
#include <QtCore/qglobal.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qlist.h>
#include <QtMultimedia/qaudio.h>
#include <QtMultimedia/qaudioformat.h>

QT_BEGIN_NAMESPACE

class QAudioDeviceFactory;
class QAudioDeviceInfoPrivate;

class Q_MULTIMEDIA_EXPORT QAudioDeviceInfo
{
   friend class QAudioDeviceFactory;

 public:
   QAudioDeviceInfo();
   QAudioDeviceInfo(const QAudioDeviceInfo &other);
   ~QAudioDeviceInfo();

   QAudioDeviceInfo &operator=(const QAudioDeviceInfo &other);

   bool isNull() const;

   QString deviceName() const;

   bool isFormatSupported(const QAudioFormat &format) const;
   QAudioFormat preferredFormat() const;
   QAudioFormat nearestFormat(const QAudioFormat &format) const;

   QStringList supportedCodecs() const;
   QList<int> supportedFrequencies() const;
   QList<int> supportedSampleRates() const;
   QList<int> supportedChannels() const;
   QList<int> supportedChannelCounts() const;
   QList<int> supportedSampleSizes() const;
   QList<QAudioFormat::Endian> supportedByteOrders() const;
   QList<QAudioFormat::SampleType> supportedSampleTypes() const;

   static QAudioDeviceInfo defaultInputDevice();
   static QAudioDeviceInfo defaultOutputDevice();

   static QList<QAudioDeviceInfo> availableDevices(QAudio::Mode mode);

 private:
   QAudioDeviceInfo(const QString &realm, const QByteArray &handle, QAudio::Mode mode);
   QString realm() const;
   QByteArray handle() const;
   QAudio::Mode mode() const;

   QSharedDataPointer<QAudioDeviceInfoPrivate> d;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QAudioDeviceInfo)

#endif // QAUDIODEVICEINFO_H
