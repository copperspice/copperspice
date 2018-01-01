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

#include <QDebug>
#include <QtMultimedia/qaudioformat.h>


QT_BEGIN_NAMESPACE


class QAudioFormatPrivate : public QSharedData
{
 public:
   QAudioFormatPrivate() {
      frequency = -1;
      channels = -1;
      sampleSize = -1;
      byteOrder = QAudioFormat::Endian(QSysInfo::ByteOrder);
      sampleType = QAudioFormat::Unknown;
   }

   QAudioFormatPrivate(const QAudioFormatPrivate &other):
      QSharedData(other),
      codec(other.codec),
      byteOrder(other.byteOrder),
      sampleType(other.sampleType),
      frequency(other.frequency),
      channels(other.channels),
      sampleSize(other.sampleSize) {
   }

   QAudioFormatPrivate &operator=(const QAudioFormatPrivate &other) {
      codec = other.codec;
      byteOrder = other.byteOrder;
      sampleType = other.sampleType;
      frequency = other.frequency;
      channels = other.channels;
      sampleSize = other.sampleSize;

      return *this;
   }

   QString codec;
   QAudioFormat::Endian byteOrder;
   QAudioFormat::SampleType sampleType;
   int frequency;
   int channels;
   int sampleSize;
};

/*!
    \class QAudioFormat
    \brief The QAudioFormat class stores audio parameter information.

    \inmodule QtMultimedia
    \ingroup  multimedia
    \since 4.6

    An audio format specifies how data in an audio stream is arranged,
    i.e, how the stream is to be interpreted. The encoding itself is
    specified by the codec() used for the stream.

    In addition to the encoding, QAudioFormat contains other
    parameters that further specify how the audio data is arranged.
    These are the frequency, the number of channels, the sample size,
    the sample type, and the byte order. The following table describes
    these in more detail.

    \table
        \header
            \o Parameter
            \o Description
        \row
            \o Sample Rate
            \o Samples per second of audio data in Hertz.
        \row
            \o Number of channels
            \o The number of audio channels (typically one for mono
               or two for stereo)
        \row
            \o Sample size
            \o How much data is stored in each sample (typically 8
               or 16 bits)
        \row
            \o Sample type
            \o Numerical representation of sample (typically signed integer,
               unsigned integer or float)
        \row
            \o Byte order
            \o Byte ordering of sample (typically little endian, big endian)
    \endtable

    You can obtain audio formats compatible with the audio device used
    through functions in QAudioDeviceInfo. This class also lets you
    query available parameter values for a device, so that you can set
    the parameters yourself. See the QAudioDeviceInfo class
    description for details. You need to know the format of the audio
    streams you wish to play. Qt does not set up formats for you.
*/

/*!
    Construct a new audio format.

    Values are initialized as follows:
    \list
    \o sampleRate()  = -1
    \o channelCount() = -1
    \o sampleSize() = -1
    \o byteOrder()  = QAudioFormat::Endian(QSysInfo::ByteOrder)
    \o sampleType() = QAudioFormat::Unknown
    \c codec()      = ""
    \endlist
*/

QAudioFormat::QAudioFormat():
   d(new QAudioFormatPrivate)
{
}

/*!
    Construct a new audio format using \a other.
*/

QAudioFormat::QAudioFormat(const QAudioFormat &other):
   d(other.d)
{
}

/*!
    Destroy this audio format.
*/

QAudioFormat::~QAudioFormat()
{
}

/*!
    Assigns \a other to this QAudioFormat implementation.
*/

QAudioFormat &QAudioFormat::operator=(const QAudioFormat &other)
{
   d = other.d;
   return *this;
}

/*!
  Returns true if this QAudioFormat is equal to the \a other
  QAudioFormat; otherwise returns false.

  All elements of QAudioFormat are used for the comparison.
*/

bool QAudioFormat::operator==(const QAudioFormat &other) const
{
   return d->frequency == other.d->frequency &&
          d->channels == other.d->channels &&
          d->sampleSize == other.d->sampleSize &&
          d->byteOrder == other.d->byteOrder &&
          d->codec == other.d->codec &&
          d->sampleType == other.d->sampleType;
}

/*!
  Returns true if this QAudioFormat is not equal to the \a other
  QAudioFormat; otherwise returns false.

  All elements of QAudioFormat are used for the comparison.
*/

bool QAudioFormat::operator!=(const QAudioFormat &other) const
{
   return !(*this == other);
}

/*!
    Returns true if all of the parameters are valid.
*/

bool QAudioFormat::isValid() const
{
   return d->frequency != -1 && d->channels != -1 && d->sampleSize != -1 &&
          d->sampleType != QAudioFormat::Unknown && !d->codec.isEmpty();
}

/*!
   Sets the sample rate to \a samplerate Hertz.

   \since 4.7
*/

void QAudioFormat::setSampleRate(int samplerate)
{
   d->frequency = samplerate;
}

/*!
   \obsolete

   Use setSampleRate() instead.
*/

void QAudioFormat::setFrequency(int frequency)
{
   d->frequency = frequency;
}

/*!
    Returns the current sample rate in Hertz.

    \since 4.7
*/

int QAudioFormat::sampleRate() const
{
   return d->frequency;
}

/*!
   \obsolete

   Use sampleRate() instead.
*/

int QAudioFormat::frequency() const
{
   return d->frequency;
}

/*!
   Sets the channel count to \a channels.

   \since 4.7
*/

void QAudioFormat::setChannelCount(int channels)
{
   d->channels = channels;
}

/*!
   \obsolete

   Use setChannelCount() instead.
*/

void QAudioFormat::setChannels(int channels)
{
   d->channels = channels;
}

/*!
    Returns the current channel count value.

    \since 4.7
*/

int QAudioFormat::channelCount() const
{
   return d->channels;
}

/*!
    \obsolete

    Use channelCount() instead.
*/

int QAudioFormat::channels() const
{
   return d->channels;
}

/*!
   Sets the sample size to the \a sampleSize specified.
*/

void QAudioFormat::setSampleSize(int sampleSize)
{
   d->sampleSize = sampleSize;
}

/*!
    Returns the current sample size value.
*/

int QAudioFormat::sampleSize() const
{
   return d->sampleSize;
}

/*!
   Sets the codec to \a codec.

   \sa QAudioDeviceInfo::supportedCodecs()
*/

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

