/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <QDebug>
#include <qaudioformat.h>

static int qRegisterAudioFormatMetaTypes()
{
   qRegisterMetaType<QAudioFormat>();
   qRegisterMetaType<QAudioFormat::SampleType>();
   qRegisterMetaType<QAudioFormat::Endian>();

   return 0;
}

Q_CONSTRUCTOR_FUNCTION(qRegisterAudioFormatMetaTypes)
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

/*!
    \enum QAudioFormat::SampleType

    \value Unknown       Not Set
    \value SignedInt     samples are signed integers
    \value UnSignedInt   samples are unsigned intergers
    \value Float         samples are floats
*/

/*!
    \enum QAudioFormat::Endian

    \value BigEndian     samples are big endian byte order
    \value LittleEndian  samples are little endian byte order
*/

QT_END_NAMESPACE

