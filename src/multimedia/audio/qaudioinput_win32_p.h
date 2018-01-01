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

#ifndef QAUDIOINPUT_WIN32_P_H
#define QAUDIOINPUT_WIN32_P_H

#include <windows.h>
#include <mmsystem.h>

#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmutex.h>

#include <QtMultimedia/qaudio.h>
#include <QtMultimedia/qaudiodeviceinfo.h>
#include <QtMultimedia/qaudioengine.h>


QT_BEGIN_NAMESPACE

class QAudioInputPrivate : public QAbstractAudioInput
{
   MULTI_CS_OBJECT(QAudioInputPrivate)
 public:
   QAudioInputPrivate(const QByteArray &device, const QAudioFormat &audioFormat);
   ~QAudioInputPrivate();

   qint64 read(char *data, qint64 len);

   QAudioFormat format() const override;
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

   QIODevice *audioSource;
   QAudioFormat settings;
   QAudio::Error errorState;
   QAudio::State deviceState;

 private:
   qint32 buffer_size;
   qint32 period_size;
   qint32 header;
   QByteArray m_device;
   int bytesAvailable;
   int intervalTime;
   QTime timeStamp;
   qint64 elapsedTimeOffset;
   QTime timeStampOpened;
   qint64 totalTimeValue;
   bool pullMode;
   bool resuming;
   WAVEFORMATEX wfx;
   HWAVEIN hWaveIn;
   MMRESULT result;
   WAVEHDR *waveBlocks;
   volatile bool finished;
   volatile int waveFreeBlockCount;
   int waveCurrentBlock;

   QMutex mutex;
   static void QT_WIN_CALLBACK waveInProc( HWAVEIN hWaveIn, UINT uMsg,
                                           DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2 );

   WAVEHDR *allocateBlocks(int size, int count);
   void freeBlocks(WAVEHDR *blockArray);
   bool open();
   void close();

 private :
   MULTI_CS_SLOT_1(Private, void feedback())
   MULTI_CS_SLOT_2(feedback)
   MULTI_CS_SLOT_1(Private, bool deviceReady())
   MULTI_CS_SLOT_2(deviceReady)

 public:
   MULTI_CS_SIGNAL_1(Public, void processMore())
   MULTI_CS_SIGNAL_2(processMore)
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
