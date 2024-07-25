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

#ifndef QAUDIOFORMAT_H
#define QAUDIOFORMAT_H

#include <qmultimedia.h>
#include <qobject.h>
#include <qshareddata.h>

class QAudioFormatPrivate;

class Q_MULTIMEDIA_EXPORT QAudioFormat
{
 public:
   enum SampleType {
      Unknown,
      SignedInt,
      UnSignedInt,
      Float
   };

   enum Endian {
      BigEndian    = QSysInfo::BigEndian,
      LittleEndian = QSysInfo::LittleEndian
   };

   QAudioFormat();
   QAudioFormat(const QAudioFormat &other);
   ~QAudioFormat();

   QAudioFormat &operator=(const QAudioFormat &other);
   bool operator==(const QAudioFormat &other) const;
   bool operator!=(const QAudioFormat &other) const;

   bool isValid() const;

   void setSampleRate(int sampleRate);
   int sampleRate() const;

   void setChannelCount(int channelCount);
   int channelCount() const;

   void setSampleSize(int sampleSize);
   int sampleSize() const;

   void setCodec(const QString &codec);
   QString codec() const;

   void setByteOrder(QAudioFormat::Endian byteOrder);
   QAudioFormat::Endian byteOrder() const;

   void setSampleType(QAudioFormat::SampleType sampleType);
   QAudioFormat::SampleType sampleType() const;

   // Helper functions
   qint32 bytesForDuration(qint64 duration) const;
   qint64 durationForBytes(qint32 byteCount) const;

   qint32 bytesForFrames(qint32 frameCount) const;
   qint32 framesForBytes(qint32 byteCount) const;

   qint32 framesForDuration(qint64 duration) const;
   qint64 durationForFrames(qint32 frameCount) const;

   int bytesPerFrame() const;

 private:
   QSharedDataPointer<QAudioFormatPrivate> d;
};

Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, const QAudioFormat &);
Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, QAudioFormat::SampleType);
Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, QAudioFormat::Endian);

#endif
