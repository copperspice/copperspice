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

#include <qmediaresource.h>

#include <qsize.h>
#include <qurl.h>
#include <qvariant.h>

QMediaResource::QMediaResource()
{
}

QMediaResource::QMediaResource(const QUrl &url, const QString &mimeType)
{
   values.insert(Url, url);
   values.insert(MimeType, mimeType);
}

QMediaResource::QMediaResource(const QNetworkRequest &request, const QString &mimeType)
{
   values.insert(Request, QVariant::fromValue(request));
   values.insert(Url, request.url());
   values.insert(MimeType, mimeType);
}

QMediaResource::QMediaResource(const QMediaResource &other)
   : values(other.values)
{
}

QMediaResource &QMediaResource::operator =(const QMediaResource &other)
{
   values = other.values;

   return *this;
}

QMediaResource::~QMediaResource()
{
}

bool QMediaResource::operator ==(const QMediaResource &other) const
{
   // Compare requests directly as QNetworkRequests are "custom types".
   for (int key : values.keys()) {
      switch (key) {
         case Request:
            if (request() != other.request()) {
               return false;
            }
            break;
         default:
            if (values.value(key) != other.values.value(key)) {
               return false;
            }
      }
   }
   return true;
}

/*!
    Compares a media resource to \a other.

    Returns true if they are different, and false otherwise.
*/
bool QMediaResource::operator !=(const QMediaResource &other) const
{
   return !(*this == other);
}

/*!
    Identifies if a media resource is null.

    Returns true if the resource is null, and false otherwise.
*/
bool QMediaResource::isNull() const
{
   return values.isEmpty();
}

/*!
    Returns the URL of a media resource.
*/
QUrl QMediaResource::url() const
{
   return (values.value(Url)).value<QUrl>();
}

/*!
    Returns the network request associated with this media resource.
*/
QNetworkRequest QMediaResource::request() const
{
   if (values.contains(Request)) {
      return (values.value(Request)).value<QNetworkRequest>();
   }

   return QNetworkRequest(url());
}

/*!
    Returns the MIME type of a media resource.

    This may be null if the MIME type is unknown.
*/
QString QMediaResource::mimeType() const
{
   return (values.value(MimeType)).value<QString>();
}

/*!
    Returns the language of a media resource as an ISO 639-2 code.

    This may be null if the language is unknown.
*/
QString QMediaResource::language() const
{
   return (values.value(Language)).value<QString>();
}

/*!
    Sets the \a language of a media resource.
*/
void QMediaResource::setLanguage(const QString &language)
{
   if (! language.isEmpty()) {
      values.insert(Language, language);
   } else {
      values.remove(Language);
   }
}

/*!
    Returns the audio codec of a media resource.

    This may be null if the media resource does not contain an audio stream, or the codec is
    unknown.
*/
QString QMediaResource::audioCodec() const
{
   return (values.value(AudioCodec)).value<QString>();
}

/*!
    Sets the audio \a codec of a media resource.
*/
void QMediaResource::setAudioCodec(const QString &codec)
{
   if (! codec.isEmpty()) {
      values.insert(AudioCodec, codec);
   } else {
      values.remove(AudioCodec);
   }
}

/*!
    Returns the video codec of a media resource.

    This may be null if the media resource does not contain a video stream, or the codec is
    unknonwn.
*/
QString QMediaResource::videoCodec() const
{
   return (values.value(VideoCodec)).value<QString>();
}

/*!
    Sets the video \a codec of media resource.
*/
void QMediaResource::setVideoCodec(const QString &codec)
{
   if (! codec.isEmpty()) {
      values.insert(VideoCodec, codec);
   } else {
      values.remove(VideoCodec);
   }
}

/*!
    Returns the size in bytes of a media resource.

    This may be zero if the size is unknown.
*/
qint64 QMediaResource::dataSize() const
{
   return (values.value(DataSize)).value<qint64>();
}

/*!
    Sets the \a size in bytes of a media resource.
*/
void QMediaResource::setDataSize(const qint64 size)
{
   if (size != 0) {
      values.insert(DataSize, size);
   } else {
      values.remove(DataSize);
   }
}

/*!
    Returns the bit rate in bits per second of a media resource's audio stream.

    This may be zero if the bit rate is unknown, or the resource contains no audio stream.
*/
int QMediaResource::audioBitRate() const
{
   return values.value(AudioBitRate).toInt();
}

/*!
    Sets the bit \a rate in bits per second of a media resource's video stream.
*/
void QMediaResource::setAudioBitRate(int rate)
{
   if (rate != 0) {
      values.insert(AudioBitRate, rate);
   } else {
      values.remove(AudioBitRate);
   }
}

/*!
    Returns the audio sample rate of a media resource.

    This may be zero if the sample size is unknown, or the resource contains no audio stream.
*/
int QMediaResource::sampleRate() const
{
   return (values.value(SampleRate)).value<int>();
}

/*!
    Sets the audio \a sampleRate of a media resource.
*/
void QMediaResource::setSampleRate(int sampleRate)
{
   if (sampleRate != 0) {
      values.insert(SampleRate, sampleRate);
   } else {
      values.remove(SampleRate);
   }
}

/*!
    Returns the number of audio channels in a media resource.

    This may be zero if the sample size is unknown, or the resource contains no audio stream.
*/
int QMediaResource::channelCount() const
{
   return (values.value(ChannelCount)).value<int>();
}

/*!
    Sets the number of audio \a channels in a media resource.
*/
void QMediaResource::setChannelCount(int channels)
{
   if (channels != 0) {
      values.insert(ChannelCount, channels);
   } else {
      values.remove(ChannelCount);
   }
}

/*!
    Returns the bit rate in bits per second of a media resource's video stream.

    This may be zero if the bit rate is unknown, or the resource contains no video stream.
*/
int QMediaResource::videoBitRate() const
{
   return values.value(VideoBitRate).toInt();
}

/*!
    Sets the bit \a rate in bits per second of a media resource's video stream.
*/
void QMediaResource::setVideoBitRate(int rate)
{
   if (rate != 0) {
      values.insert(VideoBitRate, rate);
   } else {
      values.remove(VideoBitRate);
   }
}

/*!
    Returns the resolution in pixels of a media resource.

    This may be null is the resolution is unknown, or the resource contains no pixel data (i.e. the
    resource is an audio stream.
*/
QSize QMediaResource::resolution() const
{
   return (values.value(Resolution)).value<QSize>();
}

/*!
    Sets the \a resolution in pixels of a media resource.
*/
void QMediaResource::setResolution(const QSize &resolution)
{
   if (resolution.width() != -1 || resolution.height() != -1) {
      values.insert(Resolution, resolution);
   } else {
      values.remove(Resolution);
   }
}

/*!
    Sets the \a width and \a height in pixels of a media resource.
*/
void QMediaResource::setResolution(int width, int height)
{
   if (width != -1 || height != -1) {
      values.insert(Resolution, QSize(width, height));
   } else {
      values.remove(Resolution);
   }
}

