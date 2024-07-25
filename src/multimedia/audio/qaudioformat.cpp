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

#include <qaudioformat.h>

#include <qdebug.h>

class QAudioFormatPrivate : public QSharedData
{
 public:
   QAudioFormatPrivate() {
      sampleRate = -1;
      channels   = -1;
      sampleSize = -1;
      byteOrder  = QAudioFormat::Endian(QSysInfo::ByteOrder);
      sampleType = QAudioFormat::Unknown;
   }

   QAudioFormatPrivate(const QAudioFormatPrivate &other):
      QSharedData(other),
      codec(other.codec),
      byteOrder(other.byteOrder),
      sampleType(other.sampleType),
      sampleRate(other.sampleRate),
      channels(other.channels),
      sampleSize(other.sampleSize) {
   }

   QAudioFormatPrivate &operator=(const QAudioFormatPrivate &other) {
      codec      = other.codec;
      byteOrder  = other.byteOrder;
      sampleType = other.sampleType;
      sampleRate = other.sampleRate;
      channels   = other.channels;
      sampleSize = other.sampleSize;

      return *this;
   }

   QString codec;
   QAudioFormat::Endian byteOrder;
   QAudioFormat::SampleType sampleType;
   int sampleRate;
   int channels;
   int sampleSize;
};

QAudioFormat::QAudioFormat():
   d(new QAudioFormatPrivate)
{
}

QAudioFormat::QAudioFormat(const QAudioFormat &other):
   d(other.d)
{
}

QAudioFormat::~QAudioFormat()
{
}

QAudioFormat &QAudioFormat::operator=(const QAudioFormat &other)
{
   d = other.d;
   return *this;
}


bool QAudioFormat::operator==(const QAudioFormat &other) const
{
   return d->sampleRate == other.d->sampleRate &&
      d->channels == other.d->channels &&
      d->sampleSize == other.d->sampleSize &&
      d->byteOrder == other.d->byteOrder &&
      d->codec == other.d->codec &&
      d->sampleType == other.d->sampleType;
}

bool QAudioFormat::operator!=(const QAudioFormat &other) const
{
   return !(*this == other);
}

bool QAudioFormat::isValid() const
{
   return d->sampleRate != -1 && d->channels != -1 && d->sampleSize != -1 &&
      d->sampleType != QAudioFormat::Unknown && !d->codec.isEmpty();
}

void QAudioFormat::setSampleRate(int samplerate)
{
   d->sampleRate = samplerate;
}

int QAudioFormat::sampleRate() const
{
   return d->sampleRate;
}

void QAudioFormat::setChannelCount(int channels)
{
   d->channels = channels;
}

int QAudioFormat::channelCount() const
{
   return d->channels;
}


void QAudioFormat::setSampleSize(int sampleSize)
{
   d->sampleSize = sampleSize;
}

int QAudioFormat::sampleSize() const
{
   return d->sampleSize;
}

void QAudioFormat::setCodec(const QString &codec)
{
   d->codec = codec;
}

/*!
    Returns the current codec value.

   \sa QAudioDeviceInfo::supportedCodecs()
*/

QString QAudioFormat::codec() const
{
   return d->codec;
}

/*!
   Sets the byteOrder to \a byteOrder.
*/

void QAudioFormat::setByteOrder(QAudioFormat::Endian byteOrder)
{
   d->byteOrder = byteOrder;
}

/*!
    Returns the current byteOrder value.
*/

QAudioFormat::Endian QAudioFormat::byteOrder() const
{
   return d->byteOrder;
}

/*!
   Sets the sampleType to \a sampleType.
*/

void QAudioFormat::setSampleType(QAudioFormat::SampleType sampleType)
{
   d->sampleType = sampleType;
}

/*!
    Returns the current SampleType value.
*/

QAudioFormat::SampleType QAudioFormat::sampleType() const
{
   return d->sampleType;
}


qint32 QAudioFormat::bytesForDuration(qint64 duration) const
{
   return bytesPerFrame() * framesForDuration(duration);
}
qint64 QAudioFormat::durationForBytes(qint32 bytes) const
{
   if (!isValid() || bytes <= 0) {
      return 0;
   }

   // We round the byte count to ensure whole frames
   return qint64(1000000LL * (bytes / bytesPerFrame())) / sampleRate();
}
qint32 QAudioFormat::bytesForFrames(qint32 frameCount) const
{
   return frameCount * bytesPerFrame();
}
qint32 QAudioFormat::framesForBytes(qint32 byteCount) const
{
   int size = bytesPerFrame();
   if (size > 0) {
      return byteCount / size;
   }
   return 0;
}
qint32 QAudioFormat::framesForDuration(qint64 duration) const
{
   if (!isValid()) {
      return 0;
   }

   return qint32((duration * sampleRate()) / 1000000LL);
}
qint64 QAudioFormat::durationForFrames(qint32 frameCount) const
{
   if (!isValid() || frameCount <= 0) {
      return 0;
   }

   return (frameCount * 1000000LL) / sampleRate();
}
int QAudioFormat::bytesPerFrame() const
{
   if (!isValid()) {
      return 0;
   }

   return (sampleSize() * channelCount()) / 8;
}
QDebug operator<<(QDebug dbg, QAudioFormat::Endian endian)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   switch (endian) {
      case QAudioFormat::BigEndian:
         dbg << "BigEndian";
         break;
      case QAudioFormat::LittleEndian:
         dbg << "LittleEndian";
         break;
   }
   return dbg;
}

QDebug operator<<(QDebug dbg, QAudioFormat::SampleType type)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   switch (type) {
      case QAudioFormat::SignedInt:
         dbg << "SignedInt";
         break;
      case QAudioFormat::UnSignedInt:
         dbg << "UnSignedInt";
         break;
      case QAudioFormat::Float:
         dbg << "Float";
         break;
      default:
         dbg << "Unknown";
         break;
   }
   return dbg;
}

QDebug operator<<(QDebug dbg, const QAudioFormat &f)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   dbg << "QAudioFormat(" << f.sampleRate() << "Hz, "
      << f.sampleSize() << "bit, channelCount=" << f.channelCount()
      << ", sampleType=" << f.sampleType() << ", byteOrder=" << f.byteOrder()
      << ", codec=" << f.codec() << ')';

   return dbg;
}

