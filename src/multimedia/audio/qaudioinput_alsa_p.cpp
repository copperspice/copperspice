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

#include <QtCore/qcoreapplication.h>
#include <qaudioinput_alsa_p.h>
#include <qaudiodeviceinfo_alsa_p.h>

QT_BEGIN_NAMESPACE

//#define DEBUG_AUDIO 1

QAudioInputPrivate::QAudioInputPrivate(const QByteArray &device, const QAudioFormat &audioFormat):
   settings(audioFormat)
{
   bytesAvailable = 0;
   handle = 0;
   ahandler = 0;
   access = SND_PCM_ACCESS_RW_INTERLEAVED;
   pcmformat = SND_PCM_FORMAT_S16;
   buffer_size = 0;
   period_size = 0;
   buffer_time = 100000;
   period_time = 20000;
   totalTimeValue = 0;
   intervalTime = 1000;
   audioBuffer = 0;
   errorState = QAudio::NoError;
   deviceState = QAudio::StoppedState;
   audioSource = 0;
   pullMode = true;
   resuming = false;

   m_device = device;

   timer = new QTimer(this);
   connect(timer, SIGNAL(timeout()), SLOT(userFeed()));
}

QAudioInputPrivate::~QAudioInputPrivate()
{
   close();
   disconnect(timer, SIGNAL(timeout()), 0, 0);
   QCoreApplication::processEvents();
   delete timer;
}

QAudio::Error QAudioInputPrivate::error() const
{
   return errorState;
}

QAudio::State QAudioInputPrivate::state() const
{
   return deviceState;
}


QAudioFormat QAudioInputPrivate::format() const
{
   return settings;
}

int QAudioInputPrivate::xrun_recovery(int err)
{
   int  count = 0;
   bool reset = false;

   if (err == -EPIPE) {
      errorState = QAudio::UnderrunError;
      err = snd_pcm_prepare(handle);
      if (err < 0) {
         reset = true;
      } else {
         bytesAvailable = checkBytesReady();
         if (bytesAvailable <= 0) {
            reset = true;
         }
      }

   } else if ((err == -ESTRPIPE) || (err == -EIO)) {
      errorState = QAudio::IOError;
      while ((err = snd_pcm_resume(handle)) == -EAGAIN) {
         usleep(100);
         count++;
         if (count > 5) {
            reset = true;
            break;
         }
      }
      if (err < 0) {
         err = snd_pcm_prepare(handle);
         if (err < 0) {
            reset = true;
         }
      }
   }
   if (reset) {
      close();
      open();
      snd_pcm_prepare(handle);
      return 0;
   }
   return err;
}

int QAudioInputPrivate::setFormat()
{
   snd_pcm_format_t format = SND_PCM_FORMAT_UNKNOWN;

   if (settings.sampleSize() == 8) {
      format = SND_PCM_FORMAT_U8;
   } else if (settings.sampleSize() == 16) {
      if (settings.sampleType() == QAudioFormat::SignedInt) {
         if (settings.byteOrder() == QAudioFormat::LittleEndian) {
            format = SND_PCM_FORMAT_S16_LE;
         } else {
            format = SND_PCM_FORMAT_S16_BE;
         }
      } else if (settings.sampleType() == QAudioFormat::UnSignedInt) {
         if (settings.byteOrder() == QAudioFormat::LittleEndian) {
            format = SND_PCM_FORMAT_U16_LE;
         } else {
            format = SND_PCM_FORMAT_U16_BE;
         }
      }
   } else if (settings.sampleSize() == 24) {
      if (settings.sampleType() == QAudioFormat::SignedInt) {
         if (settings.byteOrder() == QAudioFormat::LittleEndian) {
            format = SND_PCM_FORMAT_S24_LE;
         } else {
            format = SND_PCM_FORMAT_S24_BE;
         }
      } else if (settings.sampleType() == QAudioFormat::UnSignedInt) {
         if (settings.byteOrder() == QAudioFormat::LittleEndian) {
            format = SND_PCM_FORMAT_U24_LE;
         } else {
            format = SND_PCM_FORMAT_U24_BE;
         }
      }
   } else if (settings.sampleSize() == 32) {
      if (settings.sampleType() == QAudioFormat::SignedInt) {
         if (settings.byteOrder() == QAudioFormat::LittleEndian) {
            format = SND_PCM_FORMAT_S32_LE;
         } else {
            format = SND_PCM_FORMAT_S32_BE;
         }
      } else if (settings.sampleType() == QAudioFormat::UnSignedInt) {
         if (settings.byteOrder() == QAudioFormat::LittleEndian) {
            format = SND_PCM_FORMAT_U32_LE;
         } else {
            format = SND_PCM_FORMAT_U32_BE;
         }
      } else if (settings.sampleType() == QAudioFormat::Float) {
         if (settings.byteOrder() == QAudioFormat::LittleEndian) {
            format = SND_PCM_FORMAT_FLOAT_LE;
         } else {
            format = SND_PCM_FORMAT_FLOAT_BE;
         }
      }
   } else if (settings.sampleSize() == 64) {
      if (settings.byteOrder() == QAudioFormat::LittleEndian) {
         format = SND_PCM_FORMAT_FLOAT64_LE;
      } else {
         format = SND_PCM_FORMAT_FLOAT64_BE;
      }
   }

   return format != SND_PCM_FORMAT_UNKNOWN
          ? snd_pcm_hw_params_set_format( handle, hwparams, format)
          : -1;
}

QIODevice *QAudioInputPrivate::start(QIODevice *device)
{
   if (deviceState != QAudio::StoppedState) {
      close();
   }

   if (!pullMode && audioSource) {
      delete audioSource;
   }

   if (device) {
      //set to pull mode
      pullMode = true;
      audioSource = device;
      deviceState = QAudio::ActiveState;
   } else {
      //set to push mode
      pullMode = false;
      deviceState = QAudio::IdleState;
      audioSource = new InputPrivate(this);
      audioSource->open(QIODevice::ReadOnly | QIODevice::Unbuffered);
   }

   if ( !open() ) {
      return 0;
   }

   emit stateChanged(deviceState);

   return audioSource;
}

void QAudioInputPrivate::stop()
{
   if (deviceState == QAudio::StoppedState) {
      return;
   }

   deviceState = QAudio::StoppedState;

   close();
   emit stateChanged(deviceState);
}

bool QAudioInputPrivate::open()
{
#ifdef DEBUG_AUDIO
   QTime now(QTime::currentTime());
   qDebug() << now.second() << "s " << now.msec() << "ms :open()";
#endif
   clockStamp.restart();
   timeStamp.restart();
   elapsedTimeOffset = 0;

   int dir;
   int err = 0;
   int count = 0;
   unsigned int freakuency = settings.frequency();

   if (!settings.isValid()) {
      qWarning("QAudioOutput: open error, invalid format.");
   } else if (settings.frequency() <= 0) {
      qWarning("QAudioOutput: open error, invalid sample rate (%d).",
               settings.frequency());
   } else {
      err = -1;
   }

   if (err == 0) {
      errorState = QAudio::OpenError;
      deviceState = QAudio::StoppedState;
      return false;
   }


   QString dev = QString(QLatin1String(m_device.constData()));
   QList<QByteArray> devices = QAudioDeviceInfoInternal::availableDevices(QAudio::AudioInput);
   if (dev.compare(QLatin1String("default")) == 0) {
#if(SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 14)
      dev = QLatin1String(devices.first());
#else
      dev = QLatin1String("hw:0,0");
#endif
   } else {
#if(SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 14)
      dev = QLatin1String(m_device);
#else
      int idx = 0;
      char *name;

      QString shortName = QLatin1String(m_device.mid(m_device.indexOf('=', 0) + 1).constData());

      while (snd_card_get_name(idx, &name) == 0) {
         if (qstrncmp(shortName.toLocal8Bit().constData(), name, shortName.length()) == 0) {
            break;
         }
         idx++;
      }
      dev = QString(QLatin1String("hw:%1,0")).arg(idx);
#endif
   }

   // Step 1: try and open the device
   while ((count < 5) && (err < 0)) {
      err = snd_pcm_open(&handle, dev.toLocal8Bit().constData(), SND_PCM_STREAM_CAPTURE, 0);
      if (err < 0) {
         count++;
      }
   }
   if (( err < 0) || (handle == 0)) {
      errorState = QAudio::OpenError;
      deviceState = QAudio::StoppedState;
      emit stateChanged(deviceState);
      return false;
   }
   snd_pcm_nonblock( handle, 0 );

   // Step 2: Set the desired HW parameters.
   snd_pcm_hw_params_alloca( &hwparams );

   bool fatal = false;
   QString errMessage;
   unsigned int chunks = 8;

   err = snd_pcm_hw_params_any( handle, hwparams );
   if ( err < 0 ) {
      fatal = true;
      errMessage = QString::fromLatin1("QAudioInput: snd_pcm_hw_params_any: err = %1").arg(err);
   }
   if ( !fatal ) {
      err = snd_pcm_hw_params_set_rate_resample( handle, hwparams, 1 );
      if ( err < 0 ) {
         fatal = true;
         errMessage = QString::fromLatin1("QAudioInput: snd_pcm_hw_params_set_rate_resample: err = %1").arg(err);
      }
   }
   if ( !fatal ) {
      err = snd_pcm_hw_params_set_access( handle, hwparams, access );
      if ( err < 0 ) {
         fatal = true;
         errMessage = QString::fromLatin1("QAudioInput: snd_pcm_hw_params_set_access: err = %1").arg(err);
      }
   }
   if ( !fatal ) {
      err = setFormat();
      if ( err < 0 ) {
         fatal = true;
         errMessage = QString::fromLatin1("QAudioInput: snd_pcm_hw_params_set_format: err = %1").arg(err);
      }
   }
   if ( !fatal ) {
      err = snd_pcm_hw_params_set_channels( handle, hwparams, (unsigned int)settings.channels() );
      if ( err < 0 ) {
         fatal = true;
         errMessage = QString::fromLatin1("QAudioInput: snd_pcm_hw_params_set_channels: err = %1").arg(err);
      }
   }
   if ( !fatal ) {
      err = snd_pcm_hw_params_set_rate_near( handle, hwparams, &freakuency, 0 );
      if ( err < 0 ) {
         fatal = true;
         errMessage = QString::fromLatin1("QAudioInput: snd_pcm_hw_params_set_rate_near: err = %1").arg(err);
      }
   }
   if ( !fatal ) {
      err = snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time, &dir);
      if ( err < 0 ) {
         fatal = true;
         errMessage = QString::fromLatin1("QAudioInput: snd_pcm_hw_params_set_buffer_time_near: err = %1").arg(err);
      }
   }
   if ( !fatal ) {
      err = snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, &dir);
      if ( err < 0 ) {
         fatal = true;
         errMessage = QString::fromLatin1("QAudioInput: snd_pcm_hw_params_set_period_time_near: err = %1").arg(err);
      }
   }
   if ( !fatal ) {
      err = snd_pcm_hw_params_set_periods_near(handle, hwparams, &chunks, &dir);
      if ( err < 0 ) {
         fatal = true;
         errMessage = QString::fromLatin1("QAudioInput: snd_pcm_hw_params_set_periods_near: err = %1").arg(err);
      }
   }
   if ( !fatal ) {
      err = snd_pcm_hw_params(handle, hwparams);
      if ( err < 0 ) {
         fatal = true;
         errMessage = QString::fromLatin1("QAudioInput: snd_pcm_hw_params: err = %1").arg(err);
      }
   }
   if ( err < 0) {
      qWarning() << errMessage;
      errorState = QAudio::OpenError;
      deviceState = QAudio::StoppedState;
      emit stateChanged(deviceState);
      return false;
   }
   snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_frames);
   buffer_size = snd_pcm_frames_to_bytes(handle, buffer_frames);
   snd_pcm_hw_params_get_period_size(hwparams, &period_frames, &dir);
   period_size = snd_pcm_frames_to_bytes(handle, period_frames);
   snd_pcm_hw_params_get_buffer_time(hwparams, &buffer_time, &dir);
   snd_pcm_hw_params_get_period_time(hwparams, &period_time, &dir);

   // Step 3: Set the desired SW parameters.
   snd_pcm_sw_params_t *swparams;
   snd_pcm_sw_params_alloca(&swparams);
   snd_pcm_sw_params_current(handle, swparams);
   snd_pcm_sw_params_set_start_threshold(handle, swparams, period_frames);
   snd_pcm_sw_params_set_stop_threshold(handle, swparams, buffer_frames);
   snd_pcm_sw_params_set_avail_min(handle, swparams, period_frames);
   snd_pcm_sw_params(handle, swparams);

   // Step 4: Prepare audio
   if (audioBuffer == 0) {
      audioBuffer = new char[buffer_size];
   }
   snd_pcm_prepare( handle );
   snd_pcm_start(handle);

   // Step 5: Setup timer
   bytesAvailable = checkBytesReady();

   if (pullMode) {
      connect(audioSource, SIGNAL(readyRead()), this, SLOT(userFeed()));
   }

   // Step 6: Start audio processing
   chunks = buffer_size / period_size;
   timer->start(period_time * chunks / 2000);

   errorState  = QAudio::NoError;

   totalTimeValue = 0;

   return true;
}

void QAudioInputPrivate::close()
{
   timer->stop();

   if ( handle ) {
      snd_pcm_drop( handle );
      snd_pcm_close( handle );
      handle = 0;
      delete [] audioBuffer;
      audioBuffer = 0;
   }
}

int QAudioInputPrivate::checkBytesReady()
{
   if (resuming) {
      bytesAvailable = period_size;
   } else if (deviceState != QAudio::ActiveState
              && deviceState != QAudio::IdleState) {
      bytesAvailable = 0;
   } else {
      int frames = snd_pcm_avail_update(handle);
      if (frames < 0) {
         bytesAvailable = frames;
      } else {
         if ((int)frames > (int)buffer_frames) {
            frames = buffer_frames;
         }
         bytesAvailable = snd_pcm_frames_to_bytes(handle, frames);
      }
   }
   return bytesAvailable;
}

int QAudioInputPrivate::bytesReady() const
{
   return qMax(bytesAvailable, 0);
}

qint64 QAudioInputPrivate::read(char *data, qint64 len)
{
   // Read in some audio data and write it to QIODevice, pull mode
   if ( !handle ) {
      return 0;
   }

   // bytesAvaiable is saved as a side effect of checkBytesReady().
   int bytesToRead = checkBytesReady();

   if (bytesToRead < 0) {
      // bytesAvailable as negative is error code, try to recover from it.
      xrun_recovery(bytesToRead);
      bytesToRead = checkBytesReady();
      if (bytesToRead < 0) {
         // recovery failed must stop and set error.
         close();
         errorState  = QAudio::IOError;
         deviceState = QAudio::StoppedState;
         emit stateChanged(deviceState);
         return 0;
      }
   }

   bytesToRead = qMin(len, bytesToRead);
   bytesToRead -= bytesToRead % period_size;
   int count = 0, err = 0;

   while (count < 5) {
      int chunks = bytesToRead / period_size;
      int frames = chunks * period_frames;
      if (frames > (int)buffer_frames) {
         frames = buffer_frames;
      }
      int readFrames = snd_pcm_readi(handle, audioBuffer, frames);
      if (readFrames >= 0) {
         err = snd_pcm_frames_to_bytes(handle, readFrames);
#ifdef DEBUG_AUDIO
         qDebug() << QString::fromLatin1("read in bytes = %1 (frames=%2)").arg(err).arg(readFrames).toLatin1().constData();
#endif
         break;
      } else if ((readFrames == -EAGAIN) || (readFrames == -EINTR)) {
         errorState = QAudio::IOError;
         err = 0;
         break;
      } else {
         if (readFrames == -EPIPE) {
            errorState = QAudio::UnderrunError;
            err = snd_pcm_prepare(handle);
         } else if (readFrames == -ESTRPIPE) {
            err = snd_pcm_prepare(handle);
         }
         if (err != 0) {
            break;
         }
      }
      count++;
   }
   if (err > 0) {
      // got some send it onward
#ifdef DEBUG_AUDIO
      qDebug() << "frames to write to QIODevice = " <<
               snd_pcm_bytes_to_frames( handle, (int)err ) << " (" << err << ") bytes";
#endif
      if (deviceState != QAudio::ActiveState && deviceState != QAudio::IdleState) {
         return 0;
      }
      if (pullMode) {
         qint64 l = audioSource->write(audioBuffer, err);
         if (l < 0) {
            close();
            errorState = QAudio::IOError;
            deviceState = QAudio::StoppedState;
            emit stateChanged(deviceState);
         } else if (l == 0) {
            if (deviceState != QAudio::IdleState) {
               errorState = QAudio::NoError;
               deviceState = QAudio::IdleState;
               emit stateChanged(deviceState);
            }
         } else {
            bytesAvailable -= err;
            totalTimeValue += err;
            resuming = false;
            if (deviceState != QAudio::ActiveState) {
               errorState = QAudio::NoError;
               deviceState = QAudio::ActiveState;
               emit stateChanged(deviceState);
            }
         }
         return l;

      } else {
         memcpy(data, audioBuffer, err);
         bytesAvailable -= err;
         totalTimeValue += err;
         resuming = false;
         if (deviceState != QAudio::ActiveState) {
            errorState = QAudio::NoError;
            deviceState = QAudio::ActiveState;
            emit stateChanged(deviceState);
         }
         return err;
      }
   }
   return 0;
}

void QAudioInputPrivate::resume()
{
   if (deviceState == QAudio::SuspendedState) {
      int err = 0;

      if (handle) {
         err = snd_pcm_prepare( handle );
         if (err < 0) {
            xrun_recovery(err);
         }

         err = snd_pcm_start(handle);
         if (err < 0) {
            xrun_recovery(err);
         }

         bytesAvailable = buffer_size;
      }
      resuming = true;
      deviceState = QAudio::ActiveState;
      int chunks = buffer_size / period_size;
      timer->start(period_time * chunks / 2000);
      emit stateChanged(deviceState);
   }
}

void QAudioInputPrivate::setBufferSize(int value)
{
   buffer_size = value;
}

int QAudioInputPrivate::bufferSize() const
{
   return buffer_size;
}

int QAudioInputPrivate::periodSize() const
{
   return period_size;
}

void QAudioInputPrivate::setNotifyInterval(int ms)
{
   intervalTime = qMax(0, ms);
}

int QAudioInputPrivate::notifyInterval() const
{
   return intervalTime;
}

qint64 QAudioInputPrivate::processedUSecs() const
{
   qint64 result = qint64(1000000) * totalTimeValue /
                   (settings.channels() * (settings.sampleSize() / 8)) /
                   settings.frequency();

   return result;
}

void QAudioInputPrivate::suspend()
{
   if (deviceState == QAudio::ActiveState || resuming) {
      timer->stop();
      deviceState = QAudio::SuspendedState;
      emit stateChanged(deviceState);
   }
}

void QAudioInputPrivate::userFeed()
{
   if (deviceState == QAudio::StoppedState || deviceState == QAudio::SuspendedState) {
      return;
   }
#ifdef DEBUG_AUDIO
   QTime now(QTime::currentTime());
   qDebug() << now.second() << "s " << now.msec() << "ms :userFeed() IN";
#endif
   deviceReady();
}

bool QAudioInputPrivate::deviceReady()
{
   if (pullMode) {
      // reads some audio data and writes it to QIODevice
      read(0, buffer_size);
   } else {
      // emits readyRead() so user will call read() on QIODevice to get some audio data
      InputPrivate *a = qobject_cast<InputPrivate *>(audioSource);
      a->trigger();
   }
   bytesAvailable = checkBytesReady();

   if (deviceState != QAudio::ActiveState) {
      return true;
   }

   if (bytesAvailable < 0) {
      // bytesAvailable as negative is error code, try to recover from it.
      xrun_recovery(bytesAvailable);
      bytesAvailable = checkBytesReady();
      if (bytesAvailable < 0) {
         // recovery failed must stop and set error.
         close();
         errorState = QAudio::IOError;
         deviceState = QAudio::StoppedState;
         emit stateChanged(deviceState);
         return 0;
      }
   }

   if (intervalTime && (timeStamp.elapsed() + elapsedTimeOffset) > intervalTime) {
      emit notify();
      elapsedTimeOffset = timeStamp.elapsed() + elapsedTimeOffset - intervalTime;
      timeStamp.restart();
   }
   return true;
}

qint64 QAudioInputPrivate::elapsedUSecs() const
{
   if (deviceState == QAudio::StoppedState) {
      return 0;
   }

   return clockStamp.elapsed() * 1000;
}

void QAudioInputPrivate::reset()
{
   if (handle) {
      snd_pcm_reset(handle);
   }
}

void QAudioInputPrivate::drain()
{
   if (handle) {
      snd_pcm_drain(handle);
   }
}

InputPrivate::InputPrivate(QAudioInputPrivate *audio)
{
   audioDevice = qobject_cast<QAudioInputPrivate *>(audio);
}

InputPrivate::~InputPrivate()
{
}

qint64 InputPrivate::readData( char *data, qint64 len)
{
   return audioDevice->read(data, len);
}

qint64 InputPrivate::writeData(const char *data, qint64 len)
{
   Q_UNUSED(data)
   Q_UNUSED(len)
   return 0;
}

void InputPrivate::trigger()
{
   emit readyRead();
}

QT_END_NAMESPACE
