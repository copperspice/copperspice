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

#ifndef QAUDIODECODER_H
#define QAUDIODECODER_H

#include <qmediaobject.h>
#include <qaudiobuffer.h>

class QAudioDecoderPrivate;

class Q_MULTIMEDIA_EXPORT QAudioDecoder : public QMediaObject
{
   MULTI_CS_OBJECT(QAudioDecoder)

   MULTI_CS_PROPERTY_READ(sourceFilename, sourceFilename)
   MULTI_CS_PROPERTY_WRITE(sourceFilename, setSourceFilename)
   MULTI_CS_PROPERTY_NOTIFY(sourceFilename, sourceChanged)

   MULTI_CS_PROPERTY_READ(state, state)
   MULTI_CS_PROPERTY_NOTIFY(state, stateChanged)

   MULTI_CS_PROPERTY_READ(error, errorString)

   MULTI_CS_PROPERTY_READ(bufferAvailable, bufferAvailable)
   MULTI_CS_PROPERTY_NOTIFY(bufferAvailable, bufferAvailableChanged)

   MULTI_CS_ENUM(State)
   MULTI_CS_ENUM(Error)

 public:
   enum State {
      StoppedState,
      DecodingState
   };

   enum Error {
      NoError,
      ResourceError,
      FormatError,
      AccessDeniedError,
      ServiceMissingError
   };

   explicit QAudioDecoder(QObject *parent = nullptr);

   QAudioDecoder(const QAudioDecoder &) = delete;
   QAudioDecoder &operator=(const QAudioDecoder &) = delete;

   ~QAudioDecoder();

   static QMultimedia::SupportEstimate hasSupport(const QString &mimeType, const QStringList &codecs = QStringList());

   State state() const;

   QString sourceFilename() const;
   void setSourceFilename(const QString &fileName);

   QIODevice *sourceDevice() const;
   void setSourceDevice(QIODevice *device);

   QAudioFormat audioFormat() const;
   void setAudioFormat(const QAudioFormat &format);

   Error error() const;
   QString errorString() const;

   QAudioBuffer read() const;
   bool bufferAvailable() const;

   qint64 position() const;
   qint64 duration() const;

   bool bind(QObject *object) override;
   void unbind(QObject *object) override;

   MULTI_CS_SLOT_1(Public, void start())
   MULTI_CS_SLOT_2(start)

   MULTI_CS_SLOT_1(Public, void stop())
   MULTI_CS_SLOT_2(stop)

   MULTI_CS_SIGNAL_1(Public, void bufferAvailableChanged(bool available))
   MULTI_CS_SIGNAL_2(bufferAvailableChanged, available)

   MULTI_CS_SIGNAL_1(Public, void bufferReady())
   MULTI_CS_SIGNAL_2(bufferReady)

   MULTI_CS_SIGNAL_1(Public, void finished())
   MULTI_CS_SIGNAL_2(finished)

   MULTI_CS_SIGNAL_1(Public, void stateChanged(QAudioDecoder::State state))
   MULTI_CS_SIGNAL_2(stateChanged, state)

   MULTI_CS_SIGNAL_1(Public, void formatChanged(const QAudioFormat &format))
   MULTI_CS_SIGNAL_2(formatChanged, format)

   MULTI_CS_SIGNAL_1(Public, void error(QAudioDecoder::Error error))
   MULTI_CS_SIGNAL_OVERLOAD(error, (QAudioDecoder::Error), error)

   MULTI_CS_SIGNAL_1(Public, void sourceChanged())
   MULTI_CS_SIGNAL_2(sourceChanged)

   MULTI_CS_SIGNAL_1(Public, void positionChanged(qint64 position))
   MULTI_CS_SIGNAL_2(positionChanged, position)

   MULTI_CS_SIGNAL_1(Public, void durationChanged(qint64 duration))
   MULTI_CS_SIGNAL_2(durationChanged, duration)

 private:
   Q_DECLARE_PRIVATE(QAudioDecoder)

   MULTI_CS_SLOT_1(Private, void _q_stateChanged(QAudioDecoder::State state))
   MULTI_CS_SLOT_2(_q_stateChanged)

   MULTI_CS_SLOT_1(Private, void _q_error(int error, const QString &errorString))
   MULTI_CS_SLOT_2(_q_error)
};

#endif
