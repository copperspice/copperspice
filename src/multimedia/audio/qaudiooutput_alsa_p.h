/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QAUDIOOUTPUT_ALSA_P_H
#define QAUDIOOUTPUT_ALSA_P_H

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

class OutputPrivate;

class QAudioOutputPrivate : public QAbstractAudioOutput
{
   friend class OutputPrivate;
   CS_OBJECT(QAudioOutputPrivate)
 public:
   QAudioOutputPrivate(const QByteArray &device, const QAudioFormat &audioFormat);
   ~QAudioOutputPrivate();

   qint64 write( const char *data, qint64 len );

   QIODevice *start(QIODevice *device = 0);
   void stop();
   void reset();
   void suspend();
   void resume();
   int bytesFree() const;
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

   QIODevice *audioSource;
   QAudioFormat settings;
   QAudio::Error errorState;
   QAudio::State deviceState;

 private :
   MULTI_CS_SLOT_1(Private, void userFeed())
   MULTI_CS_SLOT_2(userFeed)
   MULTI_CS_SLOT_1(Private, void feedback())
   MULTI_CS_SLOT_2(feedback)
   MULTI_CS_SLOT_1(Private, void updateAvailable())
   MULTI_CS_SLOT_2(updateAvailable)
   MULTI_CS_SLOT_1(Private, bool deviceReady())
   MULTI_CS_SLOT_2(deviceReady)

 public:
   MULTI_CS_SIGNAL_1(Public, void processMore())
   MULTI_CS_SIGNAL_2(processMore)

 private:
   bool opened;
   bool pullMode;
   bool resuming;
   int buffer_size;
   int period_size;
   int intervalTime;
   qint64 totalTimeValue;
   unsigned int buffer_time;
   unsigned int period_time;
   snd_pcm_uframes_t buffer_frames;
   snd_pcm_uframes_t period_frames;
   static void async_callback(snd_async_handler_t *ahandler);
   int xrun_recovery(int err);

   int setFormat();
   bool open();
   void close();

   QTimer *timer;
   QByteArray m_device;
   int bytesAvailable;
   QElapsedTimer timeStamp;
   QElapsedTimer clockStamp;
   qint64 elapsedTimeOffset;
   char *audioBuffer;
   snd_pcm_t *handle;
   snd_async_handler_t *ahandler;
   snd_pcm_access_t access;
   snd_pcm_format_t pcmformat;
   snd_timestamp_t *timestamp;
   snd_pcm_hw_params_t *hwparams;
};

class OutputPrivate : public QIODevice
{
   friend class QAudioOutputPrivate;
   CS_OBJECT(OutputPrivate)

 public:
   OutputPrivate(QAudioOutputPrivate *audio);
   ~OutputPrivate();

   qint64 readData( char *data, qint64 len);
   qint64 writeData(const char *data, qint64 len);

 private:
   QAudioOutputPrivate *audioDevice;
};

QT_END_NAMESPACE

#endif
