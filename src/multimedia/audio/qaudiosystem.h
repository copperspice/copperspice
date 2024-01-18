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

#ifndef QAUDIOSYSTEM_H
#define QAUDIOSYSTEM_H

#include <qmultimedia.h>
#include <qaudio.h>
#include <qaudioformat.h>
#include <qaudiodeviceinfo.h>

class QIODevice;

class Q_MULTIMEDIA_EXPORT QAbstractAudioDeviceInfo : public QObject
{
   MULTI_CS_OBJECT(QAbstractAudioDeviceInfo)

 public:
   virtual QAudioFormat preferredFormat() const = 0;
   virtual bool isFormatSupported(const QAudioFormat &format) const = 0;
   virtual QString deviceName() const = 0;
   virtual QStringList supportedCodecs() = 0;
   virtual QList<int> supportedSampleRates() = 0;
   virtual QList<int> supportedChannelCounts() = 0;
   virtual QList<int> supportedSampleSizes() = 0;
   virtual QList<QAudioFormat::Endian> supportedByteOrders() = 0;
   virtual QList<QAudioFormat::SampleType> supportedSampleTypes() = 0;
};

class Q_MULTIMEDIA_EXPORT QAbstractAudioOutput : public QObject
{
   MULTI_CS_OBJECT(QAbstractAudioOutput)

 public:
   virtual void start(QIODevice *device) = 0;
   virtual QIODevice *start() = 0;
   virtual void stop() = 0;
   virtual void reset() = 0;
   virtual void suspend() = 0;
   virtual void resume() = 0;
   virtual int bytesFree() const = 0;
   virtual int periodSize() const = 0;
   virtual void setBufferSize(int value) = 0;
   virtual int bufferSize() const = 0;
   virtual void setNotifyInterval(int milliSeconds) = 0;
   virtual int notifyInterval() const = 0;
   virtual qint64 processedUSecs() const = 0;
   virtual qint64 elapsedUSecs() const = 0;
   virtual QAudio::Error error() const = 0;
   virtual QAudio::State state() const = 0;
   virtual void setFormat(const QAudioFormat &fmt) = 0;
   virtual QAudioFormat format() const = 0;
   virtual void setVolume(qreal) {}
   virtual qreal volume() const {
      return 1.0;
   }
   virtual QString category() const {
      return QString();
   }
   virtual void setCategory(const QString &) { }

   MULTI_CS_SIGNAL_1(Public, void errorChanged(QAudio::Error error))
   MULTI_CS_SIGNAL_2(errorChanged, error)

   MULTI_CS_SIGNAL_1(Public, void stateChanged(QAudio::State state))
   MULTI_CS_SIGNAL_2(stateChanged, state)

   MULTI_CS_SIGNAL_1(Public, void notify())
   MULTI_CS_SIGNAL_2(notify)
};

class Q_MULTIMEDIA_EXPORT QAbstractAudioInput : public QObject
{
   MULTI_CS_OBJECT(QAbstractAudioInput)

 public:
   virtual void start(QIODevice *device) = 0;
   virtual QIODevice *start() = 0;
   virtual void stop() = 0;
   virtual void reset() = 0;
   virtual void suspend()  = 0;
   virtual void resume() = 0;
   virtual int bytesReady() const = 0;
   virtual int periodSize() const = 0;
   virtual void setBufferSize(int value) = 0;
   virtual int bufferSize() const = 0;
   virtual void setNotifyInterval(int milliSeconds) = 0;
   virtual int notifyInterval() const = 0;
   virtual qint64 processedUSecs() const = 0;
   virtual qint64 elapsedUSecs() const = 0;
   virtual QAudio::Error error() const = 0;
   virtual QAudio::State state() const = 0;
   virtual void setFormat(const QAudioFormat &fmt) = 0;
   virtual QAudioFormat format() const = 0;
   virtual void setVolume(qreal) = 0;
   virtual qreal volume() const = 0;

   MULTI_CS_SIGNAL_1(Public, void errorChanged(QAudio::Error error))
   MULTI_CS_SIGNAL_2(errorChanged, error)

   MULTI_CS_SIGNAL_1(Public, void stateChanged(QAudio::State state))
   MULTI_CS_SIGNAL_2(stateChanged, state)

   MULTI_CS_SIGNAL_1(Public, void notify())
   MULTI_CS_SIGNAL_2(notify)
};

#endif
