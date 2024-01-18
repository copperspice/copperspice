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

#include <qaudiodevicefactory_p.h>

#include <qaudiosystem.h>
#include <qaudiodeviceinfo.h>
#include <qmap.h>

class QAudioDeviceInfoPrivate : public QSharedData
{
 public:
   QAudioDeviceInfoPrivate()
      : mode(QAudio::AudioOutput), info(nullptr)
   {
   }

   QAudioDeviceInfoPrivate(const QString &r, const QString &h, QAudio::Mode m):
      realm(r), handle(h), mode(m)
   {
      if (! handle.isEmpty()) {
         info = QAudioDeviceFactory::audioDeviceInfo(realm, handle, mode);
      } else {
         info = nullptr;
      }
   }

   QAudioDeviceInfoPrivate(const QAudioDeviceInfoPrivate &other):
      QSharedData(other),
      realm(other.realm), handle(other.handle), mode(other.mode) {
      info = QAudioDeviceFactory::audioDeviceInfo(realm, handle, mode);
   }

   QAudioDeviceInfoPrivate &operator=(const QAudioDeviceInfoPrivate &other) {
      delete info;

      realm = other.realm;
      handle = other.handle;
      mode = other.mode;
      info = QAudioDeviceFactory::audioDeviceInfo(realm, handle, mode);
      return *this;
   }

   ~QAudioDeviceInfoPrivate() {
      delete info;
   }

   QString  realm;
   QString  handle;
   QAudio::Mode mode;
   QAbstractAudioDeviceInfo *info;
};

/*!
    Constructs an empty QAudioDeviceInfo object.
*/

QAudioDeviceInfo::QAudioDeviceInfo():
   d(new QAudioDeviceInfoPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/

QAudioDeviceInfo::QAudioDeviceInfo(const QAudioDeviceInfo &other):
   d(other.d)
{
}

/*!
    Destroy this audio device info.
*/

QAudioDeviceInfo::~QAudioDeviceInfo()
{
}

/*!
    Sets the QAudioDeviceInfo object to be equal to \a other.
*/

QAudioDeviceInfo &QAudioDeviceInfo::operator=(const QAudioDeviceInfo &other)
{
   d = other.d;
   return *this;
}

/*!
    Returns whether this QAudioDeviceInfo object holds a device definition.
*/
bool QAudioDeviceInfo::operator ==(const QAudioDeviceInfo &other) const
{
   if (d == other.d) {
      return true;
   }
   if (d->realm == other.d->realm
      && d->mode == other.d->mode
      && d->handle == other.d->handle
      && deviceName() == other.deviceName()) {
      return true;
   }
   return false;
}

bool QAudioDeviceInfo::operator !=(const QAudioDeviceInfo &other) const
{
   return !operator==(other);
}
bool QAudioDeviceInfo::isNull() const
{
   return d->info == nullptr;
}

/*!
    Returns human readable name of audio device.

    Device names vary depending on platform/audio plugin being used.

    They are a unique string identifiers for the audio device.

    eg. default, Intel, U0x46d0x9a4
*/

QString QAudioDeviceInfo::deviceName() const
{
   return isNull() ? QString() : d->info->deviceName();
}

/*!
    Returns true if \a settings are supported by the audio device of this QAudioDeviceInfo.
*/

bool QAudioDeviceInfo::isFormatSupported(const QAudioFormat &settings) const
{
   return isNull() ? false : d->info->isFormatSupported(settings);
}

/*!
    Returns QAudioFormat of default settings.

    These settings are provided by the platform/audio plugin being used.

    They also are dependent on the QAudio::Mode being used.

    A typical audio system would provide something like:
    \list
    \o Input settings: 8000Hz mono 8 bit.
    \o Output settings: 44100Hz stereo 16 bit little endian.
    \endlist
*/

QAudioFormat QAudioDeviceInfo::preferredFormat() const
{
   return isNull() ? QAudioFormat() : d->info->preferredFormat();
}

/*!
    Returns closest QAudioFormat to \a settings that system audio supports.

    These settings are provided by the platform/audio plugin being used.

    They also are dependent on the QAudio::Mode being used.
*/

QAudioFormat QAudioDeviceInfo::nearestFormat(const QAudioFormat &settings) const
{
   if (isFormatSupported(settings)) {
      return settings;
   }

   QAudioFormat nearest = settings;

   QList<QString> testCodecs = supportedCodecs();
   QList<int> testChannels = supportedChannelCounts();
   QList<QAudioFormat::Endian> testByteOrders = supportedByteOrders();
   QList<QAudioFormat::SampleType> testSampleTypes;
   QList<QAudioFormat::SampleType> sampleTypesAvailable = supportedSampleTypes();
   QMap<int, int> testSampleRates;
   QList<int> sampleRatesAvailable = supportedSampleRates();
   QMap<int, int> testSampleSizes;
   QList<int> sampleSizesAvailable = supportedSampleSizes();

   // Get sorted lists for checking
   if (testCodecs.contains(settings.codec())) {
      testCodecs.removeAll(settings.codec());
      testCodecs.insert(0, settings.codec());
   }
   testChannels.removeAll(settings.channelCount());
   testChannels.insert(0, settings.channelCount());
   testByteOrders.removeAll(settings.byteOrder());
   testByteOrders.insert(0, settings.byteOrder());

   // Get sorted sampleSizes (equal to and ascending values only)
   if (sampleTypesAvailable.contains(settings.sampleType())) {
      testSampleTypes.append(settings.sampleType());
   }
   if (sampleTypesAvailable.contains(QAudioFormat::SignedInt)) {
      testSampleTypes.append(QAudioFormat::SignedInt);
   }
   if (sampleTypesAvailable.contains(QAudioFormat::UnSignedInt)) {
      testSampleTypes.append(QAudioFormat::UnSignedInt);
   }
   if (sampleTypesAvailable.contains(QAudioFormat::Float)) {
      testSampleTypes.append(QAudioFormat::Float);
   }
   if (sampleSizesAvailable.contains(settings.sampleSize())) {
      testSampleSizes.insert(0, settings.sampleSize());
   }

   sampleSizesAvailable.removeAll(settings.sampleSize());
   for (int size : sampleSizesAvailable) {
      int larger  = (size > settings.sampleSize()) ? size : settings.sampleSize();
      int smaller = (size > settings.sampleSize()) ? settings.sampleSize() : size;

      bool isMultiple = ( 0 == (larger % smaller));
      int diff = larger - smaller;
      testSampleSizes.insert((isMultiple ? diff : diff + 100000), size);
   }

   if (sampleRatesAvailable.contains(settings.sampleRate())) {
      testSampleRates.insert(0, settings.sampleRate());
   }

   sampleRatesAvailable.removeAll(settings.sampleRate());
   for (int sampleRate : sampleRatesAvailable) {
      int larger  = (sampleRate > settings.sampleRate()) ? sampleRate : settings.sampleRate();
      int smaller = (sampleRate > settings.sampleRate()) ? settings.sampleRate() : sampleRate;
      bool isMultiple = ( 0 == (larger % smaller));
      int diff = larger - smaller;
      testSampleRates.insert((isMultiple ? diff : diff + 100000), sampleRate);

   }

   // Try to find nearest
   for (QString codec : testCodecs) {
      nearest.setCodec(codec);

      for (QAudioFormat::Endian order : testByteOrders) {
         nearest.setByteOrder(order);

         for (QAudioFormat::SampleType sample : testSampleTypes) {
            nearest.setSampleType(sample);
            QMapIterator<int, int> sz(testSampleSizes);

            while (sz.hasNext()) {
               sz.next();
               nearest.setSampleSize(sz.value());

               for (int channel : testChannels) {
                  nearest.setChannelCount(channel);
                  QMapIterator<int, int> i(testSampleRates);

                  while (i.hasNext()) {
                     i.next();
                     nearest.setSampleRate(i.value());
                     if (isFormatSupported(nearest)) {
                        return nearest;
                     }
                  }
               }
            }
         }
      }
   }

   //Fallback
   return preferredFormat();
}



QStringList QAudioDeviceInfo::supportedCodecs() const
{
   return isNull() ? QStringList() : d->info->supportedCodecs();
}


QList<int> QAudioDeviceInfo::supportedSampleRates() const
{
   return isNull() ? QList<int>() : d->info->supportedSampleRates();
}



QList<int> QAudioDeviceInfo::supportedChannelCounts() const
{
   return isNull() ? QList<int>() : d->info->supportedChannelCounts();
}



QList<int> QAudioDeviceInfo::supportedSampleSizes() const
{
   return isNull() ? QList<int>() : d->info->supportedSampleSizes();
}


QList<QAudioFormat::Endian> QAudioDeviceInfo::supportedByteOrders() const
{
   return isNull() ? QList<QAudioFormat::Endian>() : d->info->supportedByteOrders();
}

QList<QAudioFormat::SampleType> QAudioDeviceInfo::supportedSampleTypes() const
{
   return isNull() ? QList<QAudioFormat::SampleType>() : d->info->supportedSampleTypes();
}

QAudioDeviceInfo QAudioDeviceInfo::defaultInputDevice()
{
   return QAudioDeviceFactory::defaultInputDevice();
}
QAudioDeviceInfo QAudioDeviceInfo::defaultOutputDevice()
{
   return QAudioDeviceFactory::defaultOutputDevice();
}

QList<QAudioDeviceInfo> QAudioDeviceInfo::availableDevices(QAudio::Mode mode)
{
   return QAudioDeviceFactory::availableDevices(mode);
}


QAudioDeviceInfo::QAudioDeviceInfo(const QString &realm, const QString &handle, QAudio::Mode mode):
   d(new QAudioDeviceInfoPrivate(realm, handle, mode))
{
}


QString QAudioDeviceInfo::realm() const
{
   return d->realm;
}

QString QAudioDeviceInfo::handle() const
{
   return d->handle;
}


QAudio::Mode QAudioDeviceInfo::mode() const
{
   return d->mode;
}


