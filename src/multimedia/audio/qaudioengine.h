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

#ifndef QAUDIOENGINE_H
#define QAUDIOENGINE_H

#include <QtCore/qglobal.h>
#include <QtMultimedia/qaudio.h>
#include <QtMultimedia/qaudioformat.h>
#include <QtMultimedia/qaudiodeviceinfo.h>

QT_BEGIN_NAMESPACE

class Q_MULTIMEDIA_EXPORT QAbstractAudioDeviceInfo : public QObject
{
   MULTI_CS_OBJECT(QAbstractAudioDeviceInfo)

 public:
   virtual QAudioFormat preferredFormat() const = 0;
   virtual bool isFormatSupported(const QAudioFormat &format) const = 0;
   virtual QAudioFormat nearestFormat(const QAudioFormat &format) const = 0;
   virtual QString deviceName() const = 0;
   virtual QStringList codecList() = 0;
   virtual QList<int> frequencyList() = 0;
   virtual QList<int> channelsList() = 0;
   virtual QList<int> sampleSizeList() = 0;
   virtual QList<QAudioFormat::Endian> byteOrderList() = 0;
   virtual QList<QAudioFormat::SampleType> sampleTypeList() = 0;
};

class Q_MULTIMEDIA_EXPORT QAbstractAudioOutput : public QObject
{
   MULTI_CS_OBJECT(QAbstractAudioOutput)

 public:
   virtual QIODevice *start(QIODevice *device) = 0;
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
   virtual QAudioFormat format() const = 0;

 public:
   MULTI_CS_SIGNAL_1(Public, void stateChanged(QAudio::State un_named_arg1))
   MULTI_CS_SIGNAL_2(stateChanged, un_named_arg1)
   MULTI_CS_SIGNAL_1(Public, void notify())
   MULTI_CS_SIGNAL_2(notify)
};

class Q_MULTIMEDIA_EXPORT QAbstractAudioInput : public QObject
{
   MULTI_CS_OBJECT(QAbstractAudioInput)

 public:
   virtual QIODevice *start(QIODevice *device) = 0;
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
   virtual QAudioFormat format() const = 0;

 public:
   MULTI_CS_SIGNAL_1(Public, void stateChanged(QAudio::State un_named_arg1))
   MULTI_CS_SIGNAL_2(stateChanged, un_named_arg1)
   MULTI_CS_SIGNAL_1(Public, void notify())
   MULTI_CS_SIGNAL_2(notify)
};

QT_END_NAMESPACE

#endif // QAUDIOENGINE_H
