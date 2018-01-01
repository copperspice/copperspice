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
   MULTI_CS_OBJECT(QAudioInputPrivate)

 public:
   QAudioInputPrivate(const QByteArray &device, const QAudioFormat &audioFormat);
   ~QAudioInputPrivate();

   qint64 read(char *data, qint64 len);

   QIODevice *start(QIODevice *device = 0) override; 
   void stop() override;
   void reset() override;
   void suspend() override;
   void resume() override;
   int bytesReady() const override;
   int periodSize() const override;
   void setBufferSize(int value) override;
   int bufferSize() const override;
   void setNotifyInterval(int milliSeconds) override;
   int notifyInterval() const override;
   qint64 processedUSecs() const override;
   qint64 elapsedUSecs() const override;
   QAudio::Error error() const override;
   QAudio::State state() const override;
   QAudioFormat format() const override;

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
   MULTI_CS_OBJECT(InputPrivate)

 public:
   InputPrivate(QAudioInputPrivate *audio);
   ~InputPrivate();

   qint64 readData( char *data, qint64 len) override;
   qint64 writeData(const char *data, qint64 len) override;

   void trigger();

 private:
   QAudioInputPrivate *audioDevice;
};

QT_END_NAMESPACE

#endif
