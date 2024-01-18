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

#ifndef QAUDIODECODERCONTROL_H
#define QAUDIODECODERCONTROL_H

#include <qpair.h>
#include <qmediacontrol.h>
#include <qaudiodecoder.h>
#include <qaudiobuffer.h>

class QIODevice;

class Q_MULTIMEDIA_EXPORT QAudioDecoderControl : public QMediaControl
{
   MULTI_CS_OBJECT(QAudioDecoderControl)

 public:
   ~QAudioDecoderControl();

   virtual QAudioDecoder::State state() const = 0;

   virtual QString sourceFilename() const = 0;
   virtual void setSourceFilename(const QString &fileName) = 0;

   virtual QIODevice *sourceDevice() const = 0;
   virtual void setSourceDevice(QIODevice *device) = 0;

   virtual void start() = 0;
   virtual void stop() = 0;

   virtual QAudioFormat audioFormat() const = 0;
   virtual void setAudioFormat(const QAudioFormat &format) = 0;

   virtual QAudioBuffer read() = 0;
   virtual bool bufferAvailable() const = 0;

   virtual qint64 position() const = 0;
   virtual qint64 duration() const = 0;

 public:
   MULTI_CS_SIGNAL_1(Public, void stateChanged(QAudioDecoder::State state))
   MULTI_CS_SIGNAL_2(stateChanged, state)

   MULTI_CS_SIGNAL_1(Public, void formatChanged(const QAudioFormat &format))
   MULTI_CS_SIGNAL_2(formatChanged, format)

   MULTI_CS_SIGNAL_1(Public, void sourceChanged())
   MULTI_CS_SIGNAL_2(sourceChanged)

   MULTI_CS_SIGNAL_1(Public, void error(int error, const QString &errorString))
   MULTI_CS_SIGNAL_2(error, error, errorString)

   MULTI_CS_SIGNAL_1(Public, void bufferReady())
   MULTI_CS_SIGNAL_2(bufferReady)

   MULTI_CS_SIGNAL_1(Public, void bufferAvailableChanged(bool available))
   MULTI_CS_SIGNAL_2(bufferAvailableChanged, available)

   MULTI_CS_SIGNAL_1(Public, void finished())
   MULTI_CS_SIGNAL_2(finished)

   MULTI_CS_SIGNAL_1(Public, void positionChanged(qint64 position))
   MULTI_CS_SIGNAL_2(positionChanged, position)

   MULTI_CS_SIGNAL_1(Public, void durationChanged(qint64 duration))
   MULTI_CS_SIGNAL_2(durationChanged, duration)

 protected:
   explicit QAudioDecoderControl(QObject *parent = nullptr);
};

#define QAudioDecoderControl_Key "com.copperspice.CS.audioDecoderControl/1.0"
CS_DECLARE_INTERFACE(QAudioDecoderControl, QAudioDecoderControl_Key)

#endif
