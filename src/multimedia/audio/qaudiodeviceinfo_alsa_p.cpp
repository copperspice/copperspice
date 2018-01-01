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

#include <qaudiodeviceinfo_alsa_p.h>
#include <alsa/version.h>

QT_BEGIN_NAMESPACE

QAudioDeviceInfoInternal::QAudioDeviceInfoInternal(QByteArray dev, QAudio::Mode mode)
{
   handle = 0;

   device = QLatin1String(dev);
   this->mode = mode;

#if (SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 14)
   checkSurround();
#endif
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
      nearest.setChannels(2);
      nearest.setByteOrder(QAudioFormat::LittleEndian);
      nearest.setSampleType(QAudioFormat::SignedInt);
      nearest.setSampleSize(16);
      nearest.setCodec(QLatin1String("audio/pcm"));
   } else {
      nearest.setFrequency(8000);
      nearest.setChannels(1);
      nearest.setSampleType(QAudioFormat::UnSignedInt);
      nearest.setSampleSize(8);
      nearest.setCodec(QLatin1String("audio/pcm"));
      if (!testSettings(nearest)) {
         nearest.setChannels(2);
         nearest.setSampleSize(16);
         nearest.setSampleType(QAudioFormat::SignedInt);
      }
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
   int err = 0;
   QString dev = device;
   QList<QByteArray> devices = availableDevices(mode);

   if (dev.compare(QLatin1String("default")) == 0) {
#if(SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 14)
      dev = QLatin1String(devices.first().constData());
#else
      dev = QLatin1String("hw:0,0");
#endif
   } else {
#if(SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 14)
      dev = device;
#else
      int idx = 0;
      char *name;

      QString shortName = device.mid(device.indexOf(QLatin1String("="), 0) + 1);

      while (snd_card_get_name(idx, &name) == 0) {
         if (dev.contains(QLatin1String(name))) {
            break;
         }
         idx++;
      }
      dev = QString(QLatin1String("hw:%1,0")).arg(idx);
#endif
   }
   if (mode == QAudio::AudioOutput) {
      err = snd_pcm_open( &handle, dev.toLocal8Bit().constData(), SND_PCM_STREAM_PLAYBACK, 0);
   } else {
      err = snd_pcm_open( &handle, dev.toLocal8Bit().constData(), SND_PCM_STREAM_CAPTURE, 0);
   }
   if (err < 0) {
      handle = 0;
      return false;
   }
   return true;
}

void QAudioDeviceInfoInternal::close()
{
   if (handle) {
      snd_pcm_close(handle);
   }
   handle = 0;
}

bool QAudioDeviceInfoInternal::testSettings(const QAudioFormat &format) const
{
   // Set nearest to closest settings that do work.
   // See if what is in settings will work (return value).
   int err = 0;
   snd_pcm_t *handle;
   snd_pcm_hw_params_t *params;
   QString dev = device;

   QList<QByteArray> devices = QAudioDeviceInfoInternal::availableDevices(QAudio::AudioOutput);

   if (dev.compare(QLatin1String("default")) == 0) {
#if(SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 14)
      dev = QLatin1String(devices.first().constData());
#else
      dev = QLatin1String("hw:0,0");
#endif
   } else {
#if(SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 14)
      dev = device;
#else
      int idx = 0;
      char *name;

      QString shortName = device.mid(device.indexOf(QLatin1String("="), 0) + 1);

      while (snd_card_get_name(idx, &name) == 0) {
         if (shortName.compare(QLatin1String(name)) == 0) {
            break;
         }
         idx++;
      }
      dev = QString(QLatin1String("hw:%1,0")).arg(idx);
#endif
   }
   if (mode == QAudio::AudioOutput) {
      err = snd_pcm_open( &handle, dev.toLocal8Bit().constData(), SND_PCM_STREAM_PLAYBACK, 0);
   } else {
      err = snd_pcm_open( &handle, dev.toLocal8Bit().constData(), SND_PCM_STREAM_CAPTURE, 0);
   }
   if (err < 0) {
      handle = 0;
      return false;
   }

   bool testChannel = false;
   bool testCodec = false;
   bool testFreq = false;
   bool testType = false;
   bool testSize = false;

   int  dir = 0;

   snd_pcm_nonblock( handle, 0 );
   snd_pcm_hw_params_alloca( &params );
   snd_pcm_hw_params_any( handle, params );

   // set the values!
   snd_pcm_hw_params_set_channels(handle, params, format.channels());
   snd_pcm_hw_params_set_rate(handle, params, format.frequency(), dir);

   err = -1;

   switch (format.sampleSize()) {
      case 8:
         if (format.sampleType() == QAudioFormat::SignedInt) {
            err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S8);
         } else if (format.sampleType() == QAudioFormat::UnSignedInt) {
            err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
         }
         break;
      case 16:
         if (format.sampleType() == QAudioFormat::SignedInt) {
            if (format.byteOrder() == QAudioFormat::LittleEndian) {
               err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
            } else if (format.byteOrder() == QAudioFormat::BigEndian) {
               err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_BE);
            }
         } else if (format.sampleType() == QAudioFormat::UnSignedInt) {
            if (format.byteOrder() == QAudioFormat::LittleEndian) {
               err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U16_LE);
            } else if (format.byteOrder() == QAudioFormat::BigEndian) {
               err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U16_BE);
            }
         }
         break;
      case 32:
         if (format.sampleType() == QAudioFormat::SignedInt) {
            if (format.byteOrder() == QAudioFormat::LittleEndian) {
               err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);
            } else if (format.byteOrder() == QAudioFormat::BigEndian) {
               err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_BE);
            }
         } else if (format.sampleType() == QAudioFormat::UnSignedInt) {
            if (format.byteOrder() == QAudioFormat::LittleEndian) {
               err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U32_LE);
            } else if (format.byteOrder() == QAudioFormat::BigEndian) {
               err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U32_BE);
            }
         }
   }

   // For now, just accept only audio/pcm codec
   if (!format.codec().startsWith(QLatin1String("audio/pcm"))) {
      err = -1;
   } else {
      testCodec = true;
   }

   if (err >= 0 && format.channels() != -1) {
      err = snd_pcm_hw_params_test_channels(handle, params, format.channels());
      if (err >= 0) {
         err = snd_pcm_hw_params_set_channels(handle, params, format.channels());
      }
      if (err >= 0) {
         testChannel = true;
      }
   }

   if (err >= 0 && format.frequency() != -1) {
      err = snd_pcm_hw_params_test_rate(handle, params, format.frequency(), 0);
      if (err >= 0) {
         err = snd_pcm_hw_params_set_rate(handle, params, format.frequency(), dir);
      }
      if (err >= 0) {
         testFreq = true;
      }
   }

   if ((err >= 0 && format.sampleSize() != -1) &&
         (format.sampleType() != QAudioFormat::Unknown)) {
      switch (format.sampleSize()) {
         case 8:
            if (format.sampleType() == QAudioFormat::SignedInt) {
               err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S8);
            } else if (format.sampleType() == QAudioFormat::UnSignedInt) {
               err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
            }
            break;
         case 16:
            if (format.sampleType() == QAudioFormat::SignedInt) {
               if (format.byteOrder() == QAudioFormat::LittleEndian) {
                  err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
               } else if (format.byteOrder() == QAudioFormat::BigEndian) {
                  err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_BE);
               }
            } else if (format.sampleType() == QAudioFormat::UnSignedInt) {
               if (format.byteOrder() == QAudioFormat::LittleEndian) {
                  err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U16_LE);
               } else if (format.byteOrder() == QAudioFormat::BigEndian) {
                  err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U16_BE);
               }
            }
            break;
         case 32:
            if (format.sampleType() == QAudioFormat::SignedInt) {
               if (format.byteOrder() == QAudioFormat::LittleEndian) {
                  err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);
               } else if (format.byteOrder() == QAudioFormat::BigEndian) {
                  err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_BE);
               }
            } else if (format.sampleType() == QAudioFormat::UnSignedInt) {
               if (format.byteOrder() == QAudioFormat::LittleEndian) {
                  err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U32_LE);
               } else if (format.byteOrder() == QAudioFormat::BigEndian) {
                  err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U32_BE);
               }
            }
      }
      if (err >= 0) {
         testSize = true;
         testType = true;
      }
   }
   if (err >= 0) {
      err = snd_pcm_hw_params(handle, params);
   }

   if (err == 0) {
      // settings work
      // close()
      if (handle) {
         snd_pcm_close(handle);
      }
      return true;
   }
   if (handle) {
      snd_pcm_close(handle);
   }

   return false;
}

void QAudioDeviceInfoInternal::updateLists()
{
   // redo all lists based on current settings
   freqz.clear();
   channelz.clear();
   sizez.clear();
   byteOrderz.clear();
   typez.clear();
   codecz.clear();

   if (!handle) {
      open();
   }

   if (!handle) {
      return;
   }

   for (int i = 0; i < (int)MAX_SAMPLE_RATES; i++) {
      //if(snd_pcm_hw_params_test_rate(handle, params, SAMPLE_RATES[i], dir) == 0)
      freqz.append(SAMPLE_RATES[i]);
   }
   channelz.append(1);
   channelz.append(2);
#if (SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 14)
   if (surround40) {
      channelz.append(4);
   }
   if (surround51) {
      channelz.append(6);
   }
   if (surround71) {
      channelz.append(8);
   }
#endif
   sizez.append(8);
   sizez.append(16);
   sizez.append(32);
   byteOrderz.append(QAudioFormat::LittleEndian);
   byteOrderz.append(QAudioFormat::BigEndian);
   typez.append(QAudioFormat::SignedInt);
   typez.append(QAudioFormat::UnSignedInt);
   typez.append(QAudioFormat::Float);
   codecz.append(QLatin1String("audio/pcm"));
   close();
}

QList<QByteArray> QAudioDeviceInfoInternal::availableDevices(QAudio::Mode mode)
{
   QList<QByteArray> allDevices;
   QList<QByteArray> devices;
   QByteArray filter;

#if(SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 14)
   // Create a list of all current audio devices that support mode
   void **hints, **n;
   char *name, *descr, *io;

   if (snd_device_name_hint(-1, "pcm", &hints) < 0) {
      qWarning() << "no alsa devices available";
      return devices;
   }
   n = hints;

   if (mode == QAudio::AudioInput) {
      filter = "Input";
   } else {
      filter = "Output";
   }

   while (*n != NULL) {
      name = snd_device_name_get_hint(*n, "NAME");
      if (name != 0 && qstrcmp(name, "null") != 0) {
         descr = snd_device_name_get_hint(*n, "DESC");
         io = snd_device_name_get_hint(*n, "IOID");

         if ((descr != NULL) && ((io == NULL) || (io == filter))) {
            QString deviceName = QLatin1String(name);
            QString deviceDescription = QLatin1String(descr);
            allDevices.append(deviceName.toLocal8Bit().constData());
            if (deviceDescription.contains(QLatin1String("Default Audio Device"))) {
               devices.append(deviceName.toLocal8Bit().constData());
            }
         }

         free(name);
         if (descr != NULL) {
            free(descr);
         }
         if (io != NULL) {
            free(io);
         }
      }
      ++n;
   }
   snd_device_name_free_hint(hints);

   if (devices.size() > 0) {
      devices.append("default");
   }
#else
   int idx = 0;
   char *name;

   while (snd_card_get_name(idx, &name) == 0) {
      devices.append(name);
      idx++;
   }
   if (idx > 0) {
      devices.append("default");
   }
#endif
   if (devices.size() == 0 && allDevices.size() > 0) {
      return allDevices;
   }

   return devices;
}

QByteArray QAudioDeviceInfoInternal::defaultInputDevice()
{
   QList<QByteArray> devices = availableDevices(QAudio::AudioInput);
   if (devices.size() == 0) {
      return QByteArray();
   }

   return devices.first();
}

QByteArray QAudioDeviceInfoInternal::defaultOutputDevice()
{
   QList<QByteArray> devices = availableDevices(QAudio::AudioOutput);
   if (devices.size() == 0) {
      return QByteArray();
   }

   return devices.first();
}

#if (SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 14)
void QAudioDeviceInfoInternal::checkSurround()
{
   QList<QByteArray> devices;
   surround40 = false;
   surround51 = false;
   surround71 = false;

   void **hints, **n;
   char *name, *descr, *io;

   if (snd_device_name_hint(-1, "pcm", &hints) < 0) {
      return;
   }

   n = hints;

   while (*n != NULL) {
      name = snd_device_name_get_hint(*n, "NAME");
      descr = snd_device_name_get_hint(*n, "DESC");
      io = snd_device_name_get_hint(*n, "IOID");
      if ((name != NULL) && (descr != NULL)) {
         QString deviceName = QLatin1String(name);
         if (mode == QAudio::AudioOutput) {
            if (deviceName.contains(QLatin1String("surround40"))) {
               surround40 = true;
            }
            if (deviceName.contains(QLatin1String("surround51"))) {
               surround51 = true;
            }
            if (deviceName.contains(QLatin1String("surround71"))) {
               surround71 = true;
            }
         }
      }
      if (name != NULL) {
         free(name);
      }
      if (descr != NULL) {
         free(descr);
      }
      if (io != NULL) {
         free(io);
      }
      ++n;
   }
   snd_device_name_free_hint(hints);
}
#endif

QT_END_NAMESPACE
