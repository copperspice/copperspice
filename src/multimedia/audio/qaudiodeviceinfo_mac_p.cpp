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

#include <QtCore/qstringlist.h>
#include <QtCore/qlist.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <qcore_mac_p.h>
#include <QtMultimedia/qaudiodeviceinfo.h>
#include <qaudio_mac_p.h>
#include <qaudiodeviceinfo_mac_p.h>

QT_BEGIN_NAMESPACE

QAudioDeviceInfoInternal::QAudioDeviceInfoInternal(QByteArray const &handle, QAudio::Mode)
{
   QDataStream ds(handle);
   quint32 did, tm;

   ds >> did >> tm >> name;
   deviceId = AudioDeviceID(did);
   mode = QAudio::Mode(tm);
}

bool QAudioDeviceInfoInternal::isFormatSupported(const QAudioFormat &format) const
{
   QAudioDeviceInfoInternal *self = const_cast<QAudioDeviceInfoInternal *>(this);

   return format.isValid()
          && format.codec() == QString::fromLatin1("audio/pcm")
          && self->frequencyList().contains(format.frequency())
          && self->channelsList().contains(format.channels())
          && self->sampleSizeList().contains(format.sampleSize());
}

QAudioFormat QAudioDeviceInfoInternal::preferredFormat() const
{
   QAudioFormat    rc;

   UInt32  propSize = 0;

   if (AudioDeviceGetPropertyInfo(deviceId,
                                  0,
                                  mode == QAudio::AudioInput,
                                  kAudioDevicePropertyStreams,
                                  &propSize,
                                  0) == noErr) {

      const int sc = propSize / sizeof(AudioStreamID);

      if (sc > 0) {
         AudioStreamID  *streams = new AudioStreamID[sc];

         if (AudioDeviceGetProperty(deviceId,
                                    0,
                                    mode == QAudio::AudioInput,
                                    kAudioDevicePropertyStreams,
                                    &propSize,
                                    streams) == noErr) {

            for (int i = 0; i < sc; ++i) {
               if (AudioStreamGetPropertyInfo(streams[i],
                                              0,
                                              kAudioStreamPropertyPhysicalFormat,
                                              &propSize,
                                              0) == noErr) {

                  AudioStreamBasicDescription sf;

                  if (AudioStreamGetProperty(streams[i],
                                             0,
                                             kAudioStreamPropertyPhysicalFormat,
                                             &propSize,
                                             &sf) == noErr) {
                     rc = toQAudioFormat(sf);
                     break;
                  }
               }
            }
         }

         delete[] streams;
      }
   }

   return rc;
}

QAudioFormat QAudioDeviceInfoInternal::nearestFormat(const QAudioFormat &format) const
{
   QAudioFormat    rc(format);
   QAudioFormat    target = preferredFormat();

   if (!format.codec().isEmpty() && format.codec() != QString::fromLatin1("audio/pcm")) {
      return QAudioFormat();
   }

   rc.setCodec(QString::fromLatin1("audio/pcm"));

   if (rc.frequency() != target.frequency()) {
      rc.setFrequency(target.frequency());
   }
   if (rc.channels() != target.channels()) {
      rc.setChannels(target.channels());
   }
   if (rc.sampleSize() != target.sampleSize()) {
      rc.setSampleSize(target.sampleSize());
   }
   if (rc.byteOrder() != target.byteOrder()) {
      rc.setByteOrder(target.byteOrder());
   }
   if (rc.sampleType() != target.sampleType()) {
      rc.setSampleType(target.sampleType());
   }

   return rc;
}

QString QAudioDeviceInfoInternal::deviceName() const
{
   return name;
}

QStringList QAudioDeviceInfoInternal::codecList()
{
   return QStringList() << QString::fromLatin1("audio/pcm");
}

QList<int> QAudioDeviceInfoInternal::frequencyList()
{
   QSet<int>  rc;

   // Add some common frequencies
   rc << 8000 << 11025 << 22050 << 44100;

   //
   UInt32  propSize = 0;

   if (AudioDeviceGetPropertyInfo(deviceId,
                                  0,
                                  mode == QAudio::AudioInput,
                                  kAudioDevicePropertyAvailableNominalSampleRates,
                                  &propSize,
                                  0) == noErr) {

      const int pc = propSize / sizeof(AudioValueRange);

      if (pc > 0) {
         AudioValueRange    *vr = new AudioValueRange[pc];

         if (AudioDeviceGetProperty(deviceId,
                                    0,
                                    mode == QAudio::AudioInput,
                                    kAudioDevicePropertyAvailableNominalSampleRates,
                                    &propSize,
                                    vr) == noErr) {

            for (int i = 0; i < pc; ++i) {
               rc << vr[i].mMaximum;
            }
         }

         delete[] vr;
      }
   }

   return rc.toList();
}

QList<int> QAudioDeviceInfoInternal::channelsList()
{
   QList<int>  rc;

   // Can mix down to 1 channel
   rc << 1;

   UInt32  propSize = 0;
   int     channels = 0;

   if (AudioDeviceGetPropertyInfo(deviceId,
                                  0,
                                  mode == QAudio::AudioInput,
                                  kAudioDevicePropertyStreamConfiguration,
                                  &propSize,
                                  0) == noErr) {

      AudioBufferList *audioBufferList = static_cast<AudioBufferList *>(qMalloc(propSize));

      if (audioBufferList != 0) {
         if (AudioDeviceGetProperty(deviceId,
                                    0,
                                    mode == QAudio::AudioInput,
                                    kAudioDevicePropertyStreamConfiguration,
                                    &propSize,
                                    audioBufferList) == noErr) {

            for (int i = 0; i < int(audioBufferList->mNumberBuffers); ++i) {
               channels += audioBufferList->mBuffers[i].mNumberChannels;
               rc << channels;
            }
         }

         qFree(audioBufferList);
      }
   }

   return rc;
}

QList<int> QAudioDeviceInfoInternal::sampleSizeList()
{
   return QList<int>() << 8 << 16 << 24 << 32 << 64;
}

QList<QAudioFormat::Endian> QAudioDeviceInfoInternal::byteOrderList()
{
   return QList<QAudioFormat::Endian>() << QAudioFormat::LittleEndian << QAudioFormat::BigEndian;
}

QList<QAudioFormat::SampleType> QAudioDeviceInfoInternal::sampleTypeList()
{
   return QList<QAudioFormat::SampleType>() << QAudioFormat::SignedInt << QAudioFormat::UnSignedInt << QAudioFormat::Float;
}

static QByteArray get_device_info(AudioDeviceID audioDevice, QAudio::Mode mode)
{
   UInt32      size;
   QByteArray  device;
   QDataStream ds(&device, QIODevice::WriteOnly);
   AudioStreamBasicDescription     sf;
   CFStringRef name;
   Boolean     isInput = mode == QAudio::AudioInput;

   // Id
   ds << quint32(audioDevice);

   // Mode
   size = sizeof(AudioStreamBasicDescription);
   if (AudioDeviceGetProperty(audioDevice, 0, isInput, kAudioDevicePropertyStreamFormat,
                              &size, &sf) != noErr) {
      return QByteArray();
   }
   ds << quint32(mode);

   // Name
   size = sizeof(CFStringRef);
   if (AudioDeviceGetProperty(audioDevice, 0, isInput, kAudioObjectPropertyName,
                              &size, &name) != noErr) {
      qWarning() << "QAudioDeviceInfo: Unable to find device name";
   }
   ds << QCFString::toQString(name);

   CFRelease(name);

   return device;
}

QByteArray QAudioDeviceInfoInternal::defaultInputDevice()
{
   AudioDeviceID   audioDevice;
   UInt32          size = sizeof(audioDevice);

   if (AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &size,
                                &audioDevice) != noErr) {
      qWarning() << "QAudioDeviceInfo: Unable to find default input device";
      return QByteArray();
   }

   return get_device_info(audioDevice, QAudio::AudioInput);
}

QByteArray QAudioDeviceInfoInternal::defaultOutputDevice()
{
   AudioDeviceID audioDevice;
   UInt32        size = sizeof(audioDevice);

   if (AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size,
                                &audioDevice) != noErr) {
      qWarning() << "QAudioDeviceInfo: Unable to find default output device";
      return QByteArray();
   }

   return get_device_info(audioDevice, QAudio::AudioOutput);
}

QList<QByteArray> QAudioDeviceInfoInternal::availableDevices(QAudio::Mode mode)
{
   QList<QByteArray>   devices;

   UInt32  propSize = 0;

   if (AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &propSize, 0) == noErr) {

      const int dc = propSize / sizeof(AudioDeviceID);

      if (dc > 0) {
         AudioDeviceID  *audioDevices = new AudioDeviceID[dc];

         if (AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &propSize, audioDevices) == noErr) {
            for (int i = 0; i < dc; ++i) {
               QByteArray info = get_device_info(audioDevices[i], mode);
               if (!info.isNull()) {
                  devices << info;
               }
            }
         }

         delete[] audioDevices;
      }
   }

   return devices;
}


QT_END_NAMESPACE

