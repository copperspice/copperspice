/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QAUDIOOUTPUT_WIN32_P_H
#define QAUDIOOUTPUT_WIN32_P_H

#include <windows.h>
#include <mmsystem.h>

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

class QAudioOutputPrivate : public QAbstractAudioOutput
{
   MULTI_CS_OBJECT(QAudioOutputPrivate)

 public:
   QAudioOutputPrivate(const QString &device, const QAudioFormat &audioFormat);
   ~QAudioOutputPrivate();

   qint64 write( const char *data, qint64 len );

   QAudioFormat format() const override;
   QIODevice *start(QIODevice *device = 0)  override;
   void stop()  override;
   void reset()  override;
   void suspend()  override;
   void resume()  override;
   int bytesFree() const  override;
   int periodSize() const  override;
   void setBufferSize(int value)  override;
   int bufferSize() const  override;
   void setNotifyInterval(int milliSeconds)  override;
   int notifyInterval() const  override;
   qint64 processedUSecs() const  override;
   qint64 elapsedUSecs() const  override;
   QAudio::Error error() const  override;
   QAudio::State state() const  override;

   QIODevice *audioSource;
   QAudioFormat settings;
   QAudio::Error errorState;
   QAudio::State deviceState;

 private :
   MULTI_CS_SLOT_1(Private, void feedback())
   MULTI_CS_SLOT_2(feedback)
   MULTI_CS_SLOT_1(Private, bool deviceReady())
   MULTI_CS_SLOT_2(deviceReady)

   QString  m_device;
   bool resuming;
   int bytesAvailable;
   QTime timeStamp;
   qint64 elapsedTimeOffset;
   QTime timeStampOpened;
   qint32 buffer_size;
   qint32 period_size;
   qint64 totalTimeValue;
   bool pullMode;
   int intervalTime;
   static void QT_WIN_CALLBACK waveOutProc( HWAVEOUT hWaveOut, UINT uMsg,
         DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2 );

   QMutex mutex;

   WAVEHDR *allocateBlocks(int size, int count);
   void freeBlocks(WAVEHDR *blockArray);
   bool open();
   void close();

   WAVEFORMATEX wfx;
   HWAVEOUT hWaveOut;
   MMRESULT result;
   WAVEHDR header;
   WAVEHDR *waveBlocks;
   volatile bool finished;
   volatile int waveFreeBlockCount;
   int waveCurrentBlock;
   char *audioBuffer;
};

class OutputPrivate : public QIODevice
{
   MULTI_CS_OBJECT(OutputPrivate)

 public:
   OutputPrivate(QAudioOutputPrivate *audio);
   ~OutputPrivate();

   qint64 readData( char *data, qint64 len)  override;
   qint64 writeData(const char *data, qint64 len)  override;

 private:
   QAudioOutputPrivate *audioDevice;
};

QT_END_NAMESPACE

#endif
