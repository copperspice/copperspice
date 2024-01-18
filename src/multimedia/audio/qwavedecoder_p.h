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

#ifndef QWAVEDECODER_H
#define QWAVEDECODER_H

#include <qiodevice.h>
#include <qaudioformat.h>

class QWaveDecoder : public QIODevice
{
   MULTI_CS_OBJECT(QWaveDecoder)

 public:
   explicit QWaveDecoder(QIODevice *source, QObject *parent = nullptr);
   ~QWaveDecoder();

   QAudioFormat audioFormat() const;
   int duration() const;

   qint64 size() const override;
   bool isSequential() const override;
   qint64 bytesAvailable() const override;

   MULTI_CS_SIGNAL_1(Public, void formatKnown())
   MULTI_CS_SIGNAL_2(formatKnown)

   MULTI_CS_SIGNAL_1(Public, void parsingError())
   MULTI_CS_SIGNAL_2(parsingError)

 private:
   qint64 readData(char *data, qint64 maxlen) override;
   qint64 writeData(const char *data, qint64 len) override;

   bool enoughDataAvailable();
   bool findChunk(const char *chunkId);
   void discardBytes(qint64 numBytes);
   void parsingFailed();

   enum State {
      InitialState,
      WaitingForFormatState,
      WaitingForDataState
   };

   struct chunk {
      char        id[4];
      quint32     size;
   };
   bool peekChunk(chunk *pChunk, bool handleEndianness = true);

   struct RIFFHeader {
      chunk       descriptor;
      char        type[4];
   };

   struct WAVEHeader {
      chunk       descriptor;
      quint16     audioFormat;
      quint16     numChannels;
      quint32     sampleRate;
      quint32     byteRate;
      quint16     blockAlign;
      quint16     bitsPerSample;
   };

   bool haveFormat;
   qint64 dataSize;
   QAudioFormat format;
   QIODevice *source;
   State state;
   quint32 junkToSkip;
   bool bigEndian;

   MULTI_CS_SLOT_1(Private, void handleData())
   MULTI_CS_SLOT_2(handleData)
};

#endif
