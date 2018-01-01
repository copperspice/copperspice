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

#ifndef QAUDIOOUTPUT_MAC_P_H
#define QAUDIOOUTPUT_MAC_P_H

#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#include <QtCore/qobject.h>
#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qtimer.h>
#include <QtCore/qatomic.h>

#include <QtMultimedia/qaudio.h>
#include <QtMultimedia/qaudioformat.h>
#include <QtMultimedia/qaudioengine.h>

QT_BEGIN_NAMESPACE

class QIODevice;
class QAbstractAudioDeviceInfo;

namespace QtMultimediaInternal {
class QAudioOutputBuffer;
}

class QAudioOutputPrivate : public QAbstractAudioOutput
{
   MULTI_CS_OBJECT(QAudioOutputPrivate)

 public:
   bool            isOpen;
   int             internalBufferSize;
   int             periodSizeBytes;
   qint64          totalFrames;
   QAudioFormat    audioFormat;
   QIODevice      *audioIO;
   AudioDeviceID   audioDeviceId;
   AudioUnit       audioUnit;
   Float64         clockFrequency;
   UInt64          startTime;
   AudioStreamBasicDescription deviceFormat;
   AudioStreamBasicDescription streamFormat;
   QtMultimediaInternal::QAudioOutputBuffer   *audioBuffer;
   QAtomicInt      audioThreadState;
   QWaitCondition  threadFinished;
   QMutex          mutex;
   QTimer         *intervalTimer;
   QAbstractAudioDeviceInfo *audioDeviceInfo;

   QAudio::Error    errorCode;
   QAudio::State    stateCode;

   QAudioOutputPrivate(const QByteArray &device, const QAudioFormat &format);
   ~QAudioOutputPrivate();

   bool open();
   void close();

   QAudioFormat format() const override;

   QIODevice *start(QIODevice *device) override;
   void stop() override;
   void reset() override;
   void suspend() override;
   void resume() override;

   int bytesFree() const override;
   int periodSize() const override;

   void setBufferSize(int value) override;
   int bufferSize() const override;

   void setNotifyInterval(int milliSeconds) override;
   int notifyInterval() const override;

   qint64 processedUSecs() const override;
   qint64 elapsedUSecs() const override;

   QAudio::Error error() const override;
   QAudio::State state() const override;

   void audioThreadStart();
   void audioThreadStop();
   void audioThreadDrain();

   void audioDeviceStop();
   void audioDeviceIdle();
   void audioDeviceError();

   void startTimers();
   void stopTimers();

 private :
   MULTI_CS_SLOT_1(Private, void deviceStopped())
   MULTI_CS_SLOT_2(deviceStopped)
   MULTI_CS_SLOT_1(Private, void inputReady())
   MULTI_CS_SLOT_2(inputReady)

   enum { Running, Draining, Stopped };

   static OSStatus renderCallback(void *inRefCon,
                                  AudioUnitRenderActionFlags *ioActionFlags,
                                  const AudioTimeStamp *inTimeStamp,
                                  UInt32 inBusNumber,
                                  UInt32 inNumberFrames,
                                  AudioBufferList *ioData);
};

QT_END_NAMESPACE


#endif
