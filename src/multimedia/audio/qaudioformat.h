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

#ifndef QAUDIOFORMAT_H
#define QAUDIOFORMAT_H

#include <QtCore/qobject.h>
#include <QtCore/qglobal.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QAudioFormatPrivate;

class Q_MULTIMEDIA_EXPORT QAudioFormat
{
 public:
   enum SampleType { Unknown, SignedInt, UnSignedInt, Float };
   enum Endian { BigEndian = QSysInfo::BigEndian, LittleEndian = QSysInfo::LittleEndian };

   QAudioFormat();
   QAudioFormat(const QAudioFormat &other);
   ~QAudioFormat();

   QAudioFormat &operator=(const QAudioFormat &other);
   bool operator==(const QAudioFormat &other) const;
   bool operator!=(const QAudioFormat &other) const;

   bool isValid() const;

   void setFrequency(int frequency);
   int frequency() const;
   void setSampleRate(int sampleRate);
   int sampleRate() const;

   void setChannels(int channels);
   int channels() const;
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

 private:
   QSharedDataPointer<QAudioFormatPrivate> d;
};


QT_END_NAMESPACE

#endif  // QAUDIOFORMAT_H
