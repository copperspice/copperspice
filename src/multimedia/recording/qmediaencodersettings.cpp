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

#include <qmediaencodersettings.h>

class QAudioEncoderSettingsPrivate  : public QSharedData
{
 public:
   QAudioEncoderSettingsPrivate() :
      isNull(true),
      encodingMode(QMultimedia::ConstantQualityEncoding),
      bitrate(-1),
      sampleRate(-1),
      channels(-1),
      quality(QMultimedia::NormalQuality) {
   }

   QAudioEncoderSettingsPrivate(const QAudioEncoderSettingsPrivate &other):
      QSharedData(other),
      isNull(other.isNull),
      encodingMode(other.encodingMode),
      codec(other.codec),
      bitrate(other.bitrate),
      sampleRate(other.sampleRate),
      channels(other.channels),
      quality(other.quality),
      encodingOptions(other.encodingOptions) {
   }

   bool isNull;
   QMultimedia::EncodingMode encodingMode;
   QString codec;
   int bitrate;
   int sampleRate;
   int channels;
   QMultimedia::EncodingQuality quality;
   QVariantMap encodingOptions;

 private:
   QAudioEncoderSettingsPrivate &operator=(const QAudioEncoderSettingsPrivate &other);
};

QAudioEncoderSettings::QAudioEncoderSettings()
   : d(new QAudioEncoderSettingsPrivate)
{
}

QAudioEncoderSettings::QAudioEncoderSettings(const QAudioEncoderSettings &other)
   : d(other.d)
{
}

QAudioEncoderSettings::~QAudioEncoderSettings()
{
}

QAudioEncoderSettings &QAudioEncoderSettings::operator=(const QAudioEncoderSettings &other)
{
   d = other.d;
   return *this;
}

bool QAudioEncoderSettings::operator==(const QAudioEncoderSettings &other) const
{
   return (d == other.d) ||
      (d->isNull == other.d->isNull &&
         d->encodingMode == other.d->encodingMode &&
         d->bitrate == other.d->bitrate &&
         d->sampleRate == other.d->sampleRate &&
         d->channels == other.d->channels &&
         d->quality == other.d->quality &&
         d->codec == other.d->codec &&
         d->encodingOptions == other.d->encodingOptions);
}

bool QAudioEncoderSettings::operator!=(const QAudioEncoderSettings &other) const
{
   return !(*this == other);
}

bool QAudioEncoderSettings::isNull() const
{
   return d->isNull;
}

QMultimedia::EncodingMode QAudioEncoderSettings::encodingMode() const
{
   return d->encodingMode;
}

void QAudioEncoderSettings::setEncodingMode(QMultimedia::EncodingMode mode)
{
   d->encodingMode = mode;
}

QString QAudioEncoderSettings::codec() const
{
   return d->codec;
}

void QAudioEncoderSettings::setCodec(const QString &codec)
{
   d->isNull = false;
   d->codec = codec;
}

int QAudioEncoderSettings::bitRate() const
{
   return d->bitrate;
}

int QAudioEncoderSettings::channelCount() const
{
   return d->channels;
}

void QAudioEncoderSettings::setChannelCount(int channels)
{
   d->isNull = false;
   d->channels = channels;
}

void QAudioEncoderSettings::setBitRate(int rate)
{
   d->isNull = false;
   d->bitrate = rate;
}

int QAudioEncoderSettings::sampleRate() const
{
   return d->sampleRate;
}

void QAudioEncoderSettings::setSampleRate(int rate)
{
   d->isNull = false;
   d->sampleRate = rate;
}

QMultimedia::EncodingQuality QAudioEncoderSettings::quality() const
{
   return d->quality;
}

void QAudioEncoderSettings::setQuality(QMultimedia::EncodingQuality quality)
{
   d->isNull = false;
   d->quality = quality;
}

QVariant QAudioEncoderSettings::encodingOption(const QString &option) const
{
   return d->encodingOptions.value(option);
}

QVariantMap QAudioEncoderSettings::encodingOptions() const
{
   return d->encodingOptions;
}

void QAudioEncoderSettings::setEncodingOption(const QString &option, const QVariant &value)
{
   d->isNull = false;

   if (! value.isValid()) {
      d->encodingOptions.remove(option);
   } else {
      d->encodingOptions.insert(option, value);
   }
}

void QAudioEncoderSettings::setEncodingOptions(const QVariantMap &options)
{
   d->isNull = false;
   d->encodingOptions = options;
}

class QVideoEncoderSettingsPrivate  : public QSharedData
{
 public:
   QVideoEncoderSettingsPrivate() :
      isNull(true),
      encodingMode(QMultimedia::ConstantQualityEncoding),
      bitrate(-1),
      frameRate(0),
      quality(QMultimedia::NormalQuality) {
   }

   QVideoEncoderSettingsPrivate(const QVideoEncoderSettingsPrivate &other):
      QSharedData(other),
      isNull(other.isNull),
      encodingMode(other.encodingMode),
      codec(other.codec),
      bitrate(other.bitrate),
      resolution(other.resolution),
      frameRate(other.frameRate),
      quality(other.quality),
      encodingOptions(other.encodingOptions) {
   }

   bool isNull;
   QMultimedia::EncodingMode encodingMode;
   QString codec;
   int bitrate;
   QSize resolution;
   qreal frameRate;
   QMultimedia::EncodingQuality quality;
   QVariantMap encodingOptions;

 private:
   QVideoEncoderSettingsPrivate &operator=(const QVideoEncoderSettingsPrivate &other);
};

QVideoEncoderSettings::QVideoEncoderSettings()
   : d(new QVideoEncoderSettingsPrivate)
{
}

QVideoEncoderSettings::QVideoEncoderSettings(const QVideoEncoderSettings &other)
   : d(other.d)
{
}

QVideoEncoderSettings::~QVideoEncoderSettings()
{
}

QVideoEncoderSettings &QVideoEncoderSettings::operator=(const QVideoEncoderSettings &other)
{
   d = other.d;
   return *this;
}

bool QVideoEncoderSettings::operator==(const QVideoEncoderSettings &other) const
{
   return (d == other.d) ||
      (d->isNull == other.d->isNull &&
         d->encodingMode == other.d->encodingMode &&
         d->bitrate == other.d->bitrate &&
         d->quality == other.d->quality &&
         d->codec == other.d->codec &&
         d->resolution == other.d->resolution &&
         qFuzzyCompare(d->frameRate, other.d->frameRate) &&
         d->encodingOptions == other.d->encodingOptions);
}

bool QVideoEncoderSettings::operator!=(const QVideoEncoderSettings &other) const
{
   return !(*this == other);
}

bool QVideoEncoderSettings::isNull() const
{
   return d->isNull;
}

QMultimedia::EncodingMode QVideoEncoderSettings::encodingMode() const
{
   return d->encodingMode;
}

void QVideoEncoderSettings::setEncodingMode(QMultimedia::EncodingMode mode)
{
   d->isNull = false;
   d->encodingMode = mode;
}


QString QVideoEncoderSettings::codec() const
{
   return d->codec;
}

void QVideoEncoderSettings::setCodec(const QString &codec)
{
   d->isNull = false;
   d->codec = codec;
}

int QVideoEncoderSettings::bitRate() const
{
   return d->bitrate;
}


void QVideoEncoderSettings::setBitRate(int value)
{
   d->isNull = false;
   d->bitrate = value;
}

qreal QVideoEncoderSettings::frameRate() const
{
   return d->frameRate;
}

void QVideoEncoderSettings::setFrameRate(qreal rate)
{
   d->isNull = false;
   d->frameRate = rate;
}

QSize QVideoEncoderSettings::resolution() const
{
   return d->resolution;
}

void QVideoEncoderSettings::setResolution(const QSize &resolution)
{
   d->isNull = false;
   d->resolution = resolution;
}


void QVideoEncoderSettings::setResolution(int width, int height)
{
   d->isNull = false;
   d->resolution = QSize(width, height);
}

QMultimedia::EncodingQuality QVideoEncoderSettings::quality() const
{
   return d->quality;
}

void QVideoEncoderSettings::setQuality(QMultimedia::EncodingQuality quality)
{
   d->isNull = false;
   d->quality = quality;
}

QVariant QVideoEncoderSettings::encodingOption(const QString &option) const
{
   return d->encodingOptions.value(option);
}

QVariantMap QVideoEncoderSettings::encodingOptions() const
{
   return d->encodingOptions;
}

void QVideoEncoderSettings::setEncodingOption(const QString &option, const QVariant &value)
{
   d->isNull = false;

   if (! value.isValid()) {
      d->encodingOptions.remove(option);
   } else {
      d->encodingOptions.insert(option, value);
   }
}

void QVideoEncoderSettings::setEncodingOptions(const QVariantMap &options)
{
   d->isNull = false;
   d->encodingOptions = options;
}

class QImageEncoderSettingsPrivate  : public QSharedData
{
 public:
   QImageEncoderSettingsPrivate() :
      isNull(true),
      quality(QMultimedia::NormalQuality) {
   }

   QImageEncoderSettingsPrivate(const QImageEncoderSettingsPrivate &other):
      QSharedData(other),
      isNull(other.isNull),
      codec(other.codec),
      resolution(other.resolution),
      quality(other.quality),
      encodingOptions(other.encodingOptions) {
   }

   bool isNull;
   QString codec;
   QSize resolution;
   QMultimedia::EncodingQuality quality;
   QVariantMap encodingOptions;

 private:
   QImageEncoderSettingsPrivate &operator=(const QImageEncoderSettingsPrivate &other);
};

QImageEncoderSettings::QImageEncoderSettings()
   : d(new QImageEncoderSettingsPrivate)
{
}

QImageEncoderSettings::QImageEncoderSettings(const QImageEncoderSettings &other)
   : d(other.d)
{
}


QImageEncoderSettings::~QImageEncoderSettings()
{
}

QImageEncoderSettings &QImageEncoderSettings::operator=(const QImageEncoderSettings &other)
{
   d = other.d;
   return *this;
}

bool QImageEncoderSettings::operator==(const QImageEncoderSettings &other) const
{
   return (d == other.d) ||
      (d->isNull == other.d->isNull &&
         d->quality == other.d->quality &&
         d->codec == other.d->codec &&
         d->resolution == other.d->resolution &&
         d->encodingOptions == other.d->encodingOptions);

}

bool QImageEncoderSettings::operator!=(const QImageEncoderSettings &other) const
{
   return !(*this == other);
}

bool QImageEncoderSettings::isNull() const
{
   return d->isNull;
}

QString QImageEncoderSettings::codec() const
{
   return d->codec;
}

void QImageEncoderSettings::setCodec(const QString &codec)
{
   d->isNull = false;
   d->codec = codec;
}

QSize QImageEncoderSettings::resolution() const
{
   return d->resolution;
}

void QImageEncoderSettings::setResolution(const QSize &resolution)
{
   d->isNull = false;
   d->resolution = resolution;
}

void QImageEncoderSettings::setResolution(int width, int height)
{
   d->isNull = false;
   d->resolution = QSize(width, height);
}

QMultimedia::EncodingQuality QImageEncoderSettings::quality() const
{
   return d->quality;
}

void QImageEncoderSettings::setQuality(QMultimedia::EncodingQuality quality)
{
   d->isNull = false;
   d->quality = quality;
}

QVariant QImageEncoderSettings::encodingOption(const QString &option) const
{
   return d->encodingOptions.value(option);
}

QVariantMap QImageEncoderSettings::encodingOptions() const
{
   return d->encodingOptions;
}

void QImageEncoderSettings::setEncodingOption(const QString &option, const QVariant &value)
{
   d->isNull = false;

   if (! value.isValid()) {
      d->encodingOptions.remove(option);
   } else {
      d->encodingOptions.insert(option, value);
   }
}

void QImageEncoderSettings::setEncodingOptions(const QVariantMap &options)
{
   d->isNull = false;
   d->encodingOptions = options;
}

