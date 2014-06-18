/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QAUDIOINPUT_ALSA_P_H
#define QAUDIOINPUT_ALSA_P_H

#include <alsa/asoundlib.h>

#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qdatetime.h>

#include <QtMultimedia/qaudio.h>
#include <QtMultimedia/qaudiodeviceinfo.h>
#include <QtMultimedia/qaudioengine.h>

QT_BEGIN_NAMESPACE

class InputPrivate;

class QAudioInputPrivate : public QAbstractAudioInput
{
   CS_OBJECT(QAudioInputPrivate)
 public:
   QAudioInputPrivate(const QByteArray &device, const QAudioFormat &audioFormat);
   ~QAudioInputPrivate();

   qint64 read(char *data, qint64 len);

   QIODevice *start(QIODevice *device = 0);
   void stop();
   void reset();
   void suspend();
   void resume();
   int bytesReady() const;
   int periodSize() const;
   void setBufferSize(int value);
   int bufferSize() const;
   void setNotifyInterval(int milliSeconds);
   int notifyInterval() const;
   qint64 processedUSecs() const;
   qint64 elapsedUSecs() const;
   QAudio::Error error() const;
   QAudio::State state() const;
   QAudioFormat format() const;
   bool resuming;
   snd_pcm_t *handle;
   qint64 totalTimeValue;
   QIODevice *audioSource;
   QAudioFormat settings;
   QAudio::Error errorState;
   QAudio::State deviceState;

 private :
   MULTI_CS_SLOT_1(Private, void userFeed())
   MULTI_CS_SLOT_2(userFeed)
   MULTI_CS_SLOT_1(Private, bool deviceReady())
   MULTI_CS_SLOT_2(deviceReady)

 private:
   int checkBytesReady();
   int xrun_recovery(int err);
   int setFormat();
   bool open();
   void close();
   void drain();

   QTimer *timer;
   QElapsedTimer timeStamp;
   QElapsedTimer clockStamp;
   qint64 elapsedTimeOffset;
   int intervalTime;
   char *audioBuffer;
   int bytesAvailable;
   QByteArray m_device;
   bool pullMode;
   int buffer_size;
   int period_size;
   unsigned int buffer_time;
   unsigned int period_time;
   snd_pcm_uframes_t buffer_frames;
   snd_pcm_uframes_t period_frames;
   snd_async_handler_t *ahandler;
   snd_pcm_access_t access;
   snd_pcm_format_t pcmformat;
   snd_timestamp_t *timestamp;
   snd_pcm_hw_params_t *hwparams;
};

class InputPrivate : public QIODevice
{
   CS_OBJECT(InputPrivate)
 public:
   InputPrivate(QAudioInputPrivate *audio);
   ~InputPrivate();

   qint64 readData( char *data, qint64 len);
   qint64 writeData(const char *data, qint64 len);

   void trigger();
 private:
   QAudioInputPrivate *audioDevice;
};

QT_END_NAMESPACE

#endif
