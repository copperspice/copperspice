/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#ifndef QAUDIOINPUT_H
#define QAUDIOINPUT_H

#include <qiodevice.h>
#include <qmultimedia.h>
#include <qaudio.h>
#include <qaudioformat.h>
#include <qaudiodeviceinfo.h>

class QAbstractAudioInput;

class Q_MULTIMEDIA_EXPORT QAudioInput : public QObject
{
   MULTI_CS_OBJECT(QAudioInput)

 public:
   explicit QAudioInput(const QAudioFormat &format = QAudioFormat(), QObject *parent = nullptr);
   explicit QAudioInput(const QAudioDeviceInfo &audioDeviceInfo, const QAudioFormat &format = QAudioFormat(),
      QObject *parent = nullptr);

   QAudioInput(const QAudioInput &) = delete;
   QAudioInput &operator=(const QAudioInput &) = delete;

   ~QAudioInput();

   QAudioFormat format() const;

   void start(QIODevice *device);
   QIODevice *start();

   void stop();
   void reset();
   void suspend();
   void resume();

   void setBufferSize(int bytes);
   int bufferSize() const;

   int bytesReady() const;
   int periodSize() const;

   void setNotifyInterval(int milliSeconds);
   int notifyInterval() const;

   void setVolume(qreal volume);
   qreal volume() const;
   qint64 processedUSecs() const;
   qint64 elapsedUSecs() const;

   QAudio::Error error() const;
   QAudio::State state() const;

   MULTI_CS_SIGNAL_1(Public, void stateChanged(QAudio::State state))
   MULTI_CS_SIGNAL_2(stateChanged, state)

   MULTI_CS_SIGNAL_1(Public, void notify())
   MULTI_CS_SIGNAL_2(notify)

 private:
   QAbstractAudioInput *m_audioInput;
};

#endif
