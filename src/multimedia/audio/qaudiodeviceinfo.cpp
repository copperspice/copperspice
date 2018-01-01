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

#include <qaudiodevicefactory_p.h>
#include <qaudioengine.h>
#include <qaudiodeviceinfo.h>

#include <qmap.h>

QT_BEGIN_NAMESPACE

class QAudioDeviceInfoPrivate : public QSharedData
{
 public:
   QAudioDeviceInfoPrivate(): info(0) {}
   QAudioDeviceInfoPrivate(const QString &r, const QByteArray &h, QAudio::Mode m):
      realm(r), handle(h), mode(m) {
      info = QAudioDeviceFactory::audioDeviceInfo(realm, handle, mode);
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

   QString     realm;
   QByteArray  handle;
   QAudio::Mode mode;
   QAbstractAudioDeviceInfo   *info;
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

bool QAudioDeviceInfo::isNull() const
{
   return d->info == 0;
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

   nearest.setCodec(QLatin1String("audio/pcm"));

   if (nearest.sampleType() == QAudioFormat::Unknown) {
      QAudioFormat preferred = preferredFormat();
      nearest.setSampleType(preferred.sampleType());
   }

   QMap<int, int> testFrequencies;
   QList<int> frequenciesAvailable = supportedFrequencies();
   QMap<int, int> testSampleSizes;
   QList<int> sampleSizesAvailable = supportedSampleSizes();

   // Get sorted sampleSizes (equal to and ascending values only)
   if (sampleSizesAvailable.contains(settings.sampleSize())) {
      testSampleSizes.insert(0, settings.sampleSize());
   }
   sampleSizesAvailable.removeAll(settings.sampleSize());
   for (int size : sampleSizesAvailable) {
      int larger  = (size > settings.sampleSize()) ? size : settings.sampleSize();
      int smaller = (size > settings.sampleSize()) ? settings.sampleSize() : size;
      if (size >= settings.sampleSize()) {
         int diff = larger - smaller;
         testSampleSizes.insert(diff, size);
      }
   }

   // Get sorted frequencies (equal to and ascending values only)
   if (frequenciesAvailable.contains(settings.frequency())) {
      testFrequencies.insert(0, settings.frequency());
   }

   frequenciesAvailable.removeAll(settings.frequency());
   for (int frequency : frequenciesAvailable) {
      int larger  = (frequency > settings.frequency()) ? frequency : settings.frequency();
      int smaller = (frequency > settings.frequency()) ? settings.frequency() : frequency;
      if (frequency >= settings.frequency()) {
         int diff = larger - smaller;
         testFrequencies.insert(diff, frequency);
      }
   }

   // Try to find nearest
   // Check ascending frequencies, ascending sampleSizes
   QMapIterator<int, int> sz(testSampleSizes);
   while (sz.hasNext()) {
      sz.next();
      nearest.setSampleSize(sz.value());
      QMapIterator<int, int> i(testFrequencies);
      while (i.hasNext()) {
         i.next();
         nearest.setFrequency(i.value());
         if (isFormatSupported(nearest)) {
            return nearest;
         }
      }
   }

   //Fallback
   return preferredFormat();
}

/*!
    Returns a list of supported codecs.

    All platform and plugin implementations should provide support for:

    "audio/pcm" - Linear PCM

    For writing plugins to support additional codecs refer to:

    http://www.iana.org/assignments/media-types/audio/
*/

QStringList QAudioDeviceInfo::supportedCodecs() const
{
   return isNull() ? QStringList() : d->info->codecList();
}

/*!
    Returns a list of supported sample rates.

    \since 4.7
*/

QList<int> QAudioDeviceInfo::supportedSampleRates() const
{
   return supportedFrequencies();
}

/*!
    \obsolete

    Use supportedSampleRates() instead.
*/

QList<int> QAudioDeviceInfo::supportedFrequencies() const
{
   return isNull() ? QList<int>() : d->info->frequencyList();
}

/*!
    Returns a list of supported channel counts.

    \since 4.7
*/

QList<int> QAudioDeviceInfo::supportedChannelCounts() const
{
   return supportedChannels();
}

/*!
    \obsolete

    Use supportedChannelCount() instead.
*/

QList<int> QAudioDeviceInfo::supportedChannels() const
{
   return isNull() ? QList<int>() : d->info->channelsList();
}

/*!
    Returns a list of supported sample sizes.
*/

QList<int> QAudioDeviceInfo::supportedSampleSizes() const
{
   return isNull() ? QList<int>() : d->info->sampleSizeList();
}

/*!
    Returns a list of supported byte orders.
*/

QList<QAudioFormat::Endian> QAudioDeviceInfo::supportedByteOrders() const
{
   return isNull() ? QList<QAudioFormat::Endian>() : d->info->byteOrderList();
}

/*!
    Returns a list of supported sample types.
*/

QList<QAudioFormat::SampleType> QAudioDeviceInfo::supportedSampleTypes() const
{
   return isNull() ? QList<QAudioFormat::SampleType>() : d->info->sampleTypeList();
}

/*!
    Returns the name of the default input audio device.
    All platform and audio plugin implementations provide a default audio device to use.
*/

QAudioDeviceInfo QAudioDeviceInfo::defaultInputDevice()
{
   return QAudioDeviceFactory::defaultInputDevice();
}

/*!
    Returns the name of the default output audio device.
    All platform and audio plugin implementations provide a default audio device to use.
*/

QAudioDeviceInfo QAudioDeviceInfo::defaultOutputDevice()
{
   return QAudioDeviceFactory::defaultOutputDevice();
}

/*!
    Returns a list of audio devices that support \a mode.
*/

QList<QAudioDeviceInfo> QAudioDeviceInfo::availableDevices(QAudio::Mode mode)
{
   return QAudioDeviceFactory::availableDevices(mode);
}


/*!
    \internal
*/

QAudioDeviceInfo::QAudioDeviceInfo(const QString &realm, const QByteArray &handle, QAudio::Mode mode):
   d(new QAudioDeviceInfoPrivate(realm, handle, mode))
{
}

/*!
    \internal
*/

QString QAudioDeviceInfo::realm() const
{
   return d->realm;
}

/*!
    \internal
*/

QByteArray QAudioDeviceInfo::handle() const
{
   return d->handle;
}


/*!
    \internal
*/

QAudio::Mode QAudioDeviceInfo::mode() const
{
   return d->mode;
}

QT_END_NAMESPACE

