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

#include <windows.h>
#include <mmsystem.h>
#include <qaudiodeviceinfo_win32_p.h>

QT_BEGIN_NAMESPACE

// For mingw toolchain mmsystem.h only defines half the defines, so add if needed.
#ifndef WAVE_FORMAT_44M08
#define WAVE_FORMAT_44M08 0x00000100
#define WAVE_FORMAT_44S08 0x00000200
#define WAVE_FORMAT_44M16 0x00000400
#define WAVE_FORMAT_44S16 0x00000800
#define WAVE_FORMAT_48M08 0x00001000
#define WAVE_FORMAT_48S08 0x00002000
#define WAVE_FORMAT_48M16 0x00004000
#define WAVE_FORMAT_48S16 0x00008000
#define WAVE_FORMAT_96M08 0x00010000
#define WAVE_FORMAT_96S08 0x00020000
#define WAVE_FORMAT_96M16 0x00040000
#define WAVE_FORMAT_96S16 0x00080000
#endif


QAudioDeviceInfoInternal::QAudioDeviceInfoInternal(QByteArray dev, QAudio::Mode mode)
{
   device = QString::fromUtf8(dev);
   this->mode = mode;

   updateLists();
}

QAudioDeviceInfoInternal::~QAudioDeviceInfoInternal()
{
   close();
}

bool QAudioDeviceInfoInternal::isFormatSupported(const QAudioFormat &format) const
{
   return testSettings(format);
}

QAudioFormat QAudioDeviceInfoInternal::preferredFormat() const
{
   QAudioFormat nearest;
   if (mode == QAudio::AudioOutput) {
      nearest.setFrequency(44100);
      nearest.setChannelCount(2);
      nearest.setByteOrder(QAudioFormat::LittleEndian);
      nearest.setSampleType(QAudioFormat::SignedInt);
      nearest.setSampleSize(16);
      nearest.setCodec(QLatin1String("audio/pcm"));
   } else {
      nearest.setFrequency(11025);
      nearest.setChannelCount(1);
      nearest.setByteOrder(QAudioFormat::LittleEndian);
      nearest.setSampleType(QAudioFormat::SignedInt);
      nearest.setSampleSize(8);
      nearest.setCodec(QLatin1String("audio/pcm"));
   }
   return nearest;
}

QAudioFormat QAudioDeviceInfoInternal::nearestFormat(const QAudioFormat &format) const
{
   if (testSettings(format)) {
      return format;
   } else {
      return preferredFormat();
   }
}

QString QAudioDeviceInfoInternal::deviceName() const
{
   return device;
}

QStringList QAudioDeviceInfoInternal::codecList()
{
   updateLists();
   return codecz;
}

QList<int> QAudioDeviceInfoInternal::frequencyList()
{
   updateLists();
   return freqz;
}

QList<int> QAudioDeviceInfoInternal::channelsList()
{
   updateLists();
   return channelz;
}

QList<int> QAudioDeviceInfoInternal::sampleSizeList()
{
   updateLists();
   return sizez;
}

QList<QAudioFormat::Endian> QAudioDeviceInfoInternal::byteOrderList()
{
   updateLists();
   return byteOrderz;
}

QList<QAudioFormat::SampleType> QAudioDeviceInfoInternal::sampleTypeList()
{
   updateLists();
   return typez;
}


bool QAudioDeviceInfoInternal::open()
{
   return true;
}

void QAudioDeviceInfoInternal::close()
{
}

bool QAudioDeviceInfoInternal::testSettings(const QAudioFormat &format) const
{
   // Set nearest to closest settings that do work.
   // See if what is in settings will work (return value).

   bool failed = false;
   bool match = false;

   // check codec
   for ( int i = 0; i < codecz.count(); i++) {
      if (format.codec() == codecz.at(i)) {
         match = true;
      }
   }
   if (!match) {
      failed = true;
   }

   // check channel
   match = false;
   if (!failed) {
      for ( int i = 0; i < channelz.count(); i++) {
         if (format.channels() == channelz.at(i)) {
            match = true;
            break;
         }
      }
      if (!match) {
         failed = true;
      }
   }

   // check frequency
   match = false;
   if (!failed) {
      for ( int i = 0; i < freqz.count(); i++) {
         if (format.frequency() == freqz.at(i)) {
            match = true;
            break;
         }
      }
      if (!match) {
         failed = true;
      }
   }

   // check sample size
   match = false;
   if (!failed) {
      for ( int i = 0; i < sizez.count(); i++) {
         if (format.sampleSize() == sizez.at(i)) {
            match = true;
            break;
         }
      }
      if (!match) {
         failed = true;
      }
   }

   // check byte order
   match = false;
   if (!failed) {
      for ( int i = 0; i < byteOrderz.count(); i++) {
         if (format.byteOrder() == byteOrderz.at(i)) {
            match = true;
            break;
         }
      }
      if (!match) {
         failed = true;
      }
   }

   // check sample type
   match = false;
   if (!failed) {
      for ( int i = 0; i < typez.count(); i++) {
         if (format.sampleType() == typez.at(i)) {
            match = true;
            break;
         }
      }
      if (!match) {
         failed = true;
      }
   }

   if (!failed) {
      // settings work
      return true;
   }
   return false;
}

void QAudioDeviceInfoInternal::updateLists()
{
   // redo all lists based on current settings
   bool base = false;
   bool match = false;
   DWORD fmt = 0;
   QString tmp;

   if (device.compare(QLatin1String("default")) == 0) {
      base = true;
   }

   if (mode == QAudio::AudioOutput) {
      WAVEOUTCAPS woc;
      unsigned long iNumDevs, i;
      iNumDevs = waveOutGetNumDevs();
      for (i = 0; i < iNumDevs; i++) {
         if (waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS))
               == MMSYSERR_NOERROR) {
            tmp = QString((const QChar *)woc.szPname);
            if (tmp.compare(device) == 0) {
               match = true;
               fmt = woc.dwFormats;
               break;
            }
            if (base) {
               match = true;
               fmt = woc.dwFormats;
               break;
            }
         }
      }
   } else {
      WAVEINCAPS woc;
      unsigned long iNumDevs, i;
      iNumDevs = waveInGetNumDevs();
      for (i = 0; i < iNumDevs; i++) {
         if (waveInGetDevCaps(i, &woc, sizeof(WAVEINCAPS))
               == MMSYSERR_NOERROR) {
            tmp = QString((const QChar *)woc.szPname);
            if (tmp.compare(device) == 0) {
               match = true;
               fmt = woc.dwFormats;
               break;
            }
            if (base) {
               match = true;
               fmt = woc.dwFormats;
               break;
            }
         }
      }
   }
   sizez.clear();
   freqz.clear();
   channelz.clear();
   byteOrderz.clear();
   typez.clear();
   codecz.clear();

   if (match) {
      if ((fmt && WAVE_FORMAT_1M08)
            || (fmt && WAVE_FORMAT_1S08)
            || (fmt && WAVE_FORMAT_2M08)
            || (fmt && WAVE_FORMAT_2S08)
            || (fmt && WAVE_FORMAT_4M08)
            || (fmt && WAVE_FORMAT_4S08)
            || (fmt && WAVE_FORMAT_48M08)
            || (fmt && WAVE_FORMAT_48S08)
            || (fmt && WAVE_FORMAT_96M08)
            || (fmt && WAVE_FORMAT_96S08)

         ) {
         sizez.append(8);
      }
      if ((fmt && WAVE_FORMAT_1M16)
            || (fmt && WAVE_FORMAT_1S16)
            || (fmt && WAVE_FORMAT_2M16)
            || (fmt && WAVE_FORMAT_2S16)
            || (fmt && WAVE_FORMAT_4M16)
            || (fmt && WAVE_FORMAT_4S16)
            || (fmt && WAVE_FORMAT_48M16)
            || (fmt && WAVE_FORMAT_48S16)
            || (fmt && WAVE_FORMAT_96M16)
            || (fmt && WAVE_FORMAT_96S16)

         ) {
         sizez.append(16);
      }
      if ((fmt && WAVE_FORMAT_1M08)
            || (fmt && WAVE_FORMAT_1S08)
            || (fmt && WAVE_FORMAT_1M16)
            || (fmt && WAVE_FORMAT_1S16)) {
         freqz.append(11025);
      }
      if ((fmt && WAVE_FORMAT_2M08)
            || (fmt && WAVE_FORMAT_2S08)
            || (fmt && WAVE_FORMAT_2M16)
            || (fmt && WAVE_FORMAT_2S16)) {
         freqz.append(22050);
      }
      if ((fmt && WAVE_FORMAT_4M08)
            || (fmt && WAVE_FORMAT_4S08)
            || (fmt && WAVE_FORMAT_4M16)
            || (fmt && WAVE_FORMAT_4S16)) {
         freqz.append(44100);
      }

      if ((fmt && WAVE_FORMAT_48M08)
            || (fmt && WAVE_FORMAT_48S08)
            || (fmt && WAVE_FORMAT_48M16)
            || (fmt && WAVE_FORMAT_48S16)) {
         freqz.append(48000);
      }
      if ((fmt && WAVE_FORMAT_96M08)
            || (fmt && WAVE_FORMAT_96S08)
            || (fmt && WAVE_FORMAT_96M16)
            || (fmt && WAVE_FORMAT_96S16)) {
         freqz.append(96000);
      }

      channelz.append(1);
      channelz.append(2);
      if (mode == QAudio::AudioOutput) {
         channelz.append(4);
         channelz.append(6);
         channelz.append(8);
      }

      byteOrderz.append(QAudioFormat::LittleEndian);

      typez.append(QAudioFormat::SignedInt);
      typez.append(QAudioFormat::UnSignedInt);

      codecz.append(QLatin1String("audio/pcm"));
   }
   if (freqz.count() > 0) {
      freqz.prepend(8000);
   }
}

QList<QByteArray> QAudioDeviceInfoInternal::availableDevices(QAudio::Mode mode)
{
   Q_UNUSED(mode)

   QList<QByteArray> devices;

   if (mode == QAudio::AudioOutput) {
      WAVEOUTCAPS woc;
      unsigned long iNumDevs, i;
      iNumDevs = waveOutGetNumDevs();
      for (i = 0; i < iNumDevs; i++) {
         if (waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS))
               == MMSYSERR_NOERROR) {
            devices.append(QString((const QChar *)woc.szPname).toUtf8().constData());
         }
      }
   } else {
      WAVEINCAPS woc;
      unsigned long iNumDevs, i;
      iNumDevs = waveInGetNumDevs();
      for (i = 0; i < iNumDevs; i++) {
         if (waveInGetDevCaps(i, &woc, sizeof(WAVEINCAPS))
               == MMSYSERR_NOERROR) {
            devices.append(QString((const QChar *)woc.szPname).toUtf8().constData());
         }
      }

   }
   if (devices.count() > 0) {
      devices.append("default");
   }

   return devices;
}

QByteArray QAudioDeviceInfoInternal::defaultOutputDevice()
{
   return QByteArray("default");
}

QByteArray QAudioDeviceInfoInternal::defaultInputDevice()
{
   return QByteArray("default");
}

QT_END_NAMESPACE
