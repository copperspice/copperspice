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

#ifndef QAUDIOINPUT_MAC_P_H
#define QAUDIOINPUT_MAC_P_H

#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#include <QtCore/qobject.h>
#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qatomic.h>

#include <QtMultimedia/qaudio.h>
#include <QtMultimedia/qaudioformat.h>
#include <QtMultimedia/qaudioengine.h>

QT_BEGIN_NAMESPACE

class QTimer;
class QIODevice;
class QAbstractAudioDeviceInfo;

namespace QtMultimediaInternal {
class QAudioInputBuffer;
}

class QAudioInputPrivate : public QAbstractAudioInput
{
   CS_OBJECT(QAudioInputPrivate)

 public:
   bool            isOpen;
   int             periodSizeBytes;
   int             internalBufferSize;
   qint64          totalFrames;
   QAudioFormat    audioFormat;
   QIODevice      *audioIO;
   AudioUnit       audioUnit;
   AudioDeviceID   audioDeviceId;
   Float64         clockFrequency;
   UInt64          startTime;
   QAudio::Error   errorCode;
   QAudio::State   stateCode;
   QtMultimediaInternal::QAudioInputBuffer   *audioBuffer;
   QMutex          mutex;
   QWaitCondition  threadFinished;
   QAtomicInt      audioThreadState;
   QTimer         *intervalTimer;
   AudioStreamBasicDescription streamFormat;
   AudioStreamBasicDescription deviceFormat;
   QAbstractAudioDeviceInfo *audioDeviceInfo;

   QAudioInputPrivate(const QByteArray &device, QAudioFormat const &format);
   ~QAudioInputPrivate();

   bool open();
   void close();

   QAudioFormat format() const;

   QIODevice *start(QIODevice *device);
   void stop();
   void reset();
   void suspend();
   void resume();
   void idle();

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

   void audioThreadStart();
   void audioThreadStop();

   void audioDeviceStop();
   void audioDeviceFull();
   void audioDeviceError();

   void startTimers();
   void stopTimers();

 private :
   MULTI_CS_SLOT_1(Private, void deviceStopped())
   MULTI_CS_SLOT_2(deviceStopped)

   enum { Running, Stopped };

   // Input callback
   static OSStatus inputCallback(void *inRefCon,
                                 AudioUnitRenderActionFlags *ioActionFlags,
                                 const AudioTimeStamp *inTimeStamp,
                                 UInt32 inBusNumber,
                                 UInt32 inNumberFrames,
                                 AudioBufferList *ioData);
};

QT_END_NAMESPACE

#endif // QAUDIOINPUT_MAC_P_H
