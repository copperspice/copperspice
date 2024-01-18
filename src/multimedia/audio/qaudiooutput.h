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

#ifndef QAUDIOOUTPUT_H
#define QAUDIOOUTPUT_H

#include <qiodevice.h>
#include <qmultimedia.h>
#include <qaudio.h>
#include <qaudioformat.h>
#include <qaudiodeviceinfo.h>

class QAbstractAudioOutput;

class Q_MULTIMEDIA_EXPORT QAudioOutput : public QObject
{
   MULTI_CS_OBJECT(QAudioOutput)

 public:
   explicit QAudioOutput(const QAudioFormat &format = QAudioFormat(), QObject *parent = nullptr);
   explicit QAudioOutput(const QAudioDeviceInfo &audioDeviceInfo, const QAudioFormat &format = QAudioFormat(),
                  QObject *parent = nullptr);

   QAudioOutput(const QAudioOutput &) = delete;
   QAudioOutput &operator=(const QAudioOutput &) = delete;

   ~QAudioOutput();

   QAudioFormat format() const;

   void start(QIODevice *device);
   QIODevice *start();

   void stop();
   void reset();
   void suspend();
   void resume();

   void setBufferSize(int bytes);
   int bufferSize() const;

   int bytesFree() const;
   int periodSize() const;

   void setNotifyInterval(int milliSeconds);
   int notifyInterval() const;

   qint64 processedUSecs() const;
   qint64 elapsedUSecs() const;

   QAudio::Error error() const;
   QAudio::State state() const;

   void setVolume(qreal volume);
   qreal volume() const;

   QString category() const;
   void setCategory(const QString &category);

   MULTI_CS_SIGNAL_1(Public, void stateChanged(QAudio::State state))
   MULTI_CS_SIGNAL_2(stateChanged, state)

   MULTI_CS_SIGNAL_1(Public, void notify())
   MULTI_CS_SIGNAL_2(notify)

 private:
   QAbstractAudioOutput *m_audioOutput;
};

#endif
