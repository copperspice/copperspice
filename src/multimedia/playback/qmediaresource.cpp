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

bool QMediaResource::operator !=(const QMediaResource &other) const
{
   return !(*this == other);
}

bool QMediaResource::isNull() const
{
   return values.isEmpty();
}

QUrl QMediaResource::url() const
{
   return (values.value(Url)).value<QUrl>();
}

QNetworkRequest QMediaResource::request() const
{
   if (values.contains(Request)) {
      return (values.value(Request)).value<QNetworkRequest>();
   }

   return QNetworkRequest(url());
}

QString QMediaResource::mimeType() const
{
   return (values.value(MimeType)).value<QString>();
}

QString QMediaResource::language() const
{
   return (values.value(Language)).value<QString>();
}

void QMediaResource::setLanguage(const QString &language)
{
   if (! language.isEmpty()) {
      values.insert(Language, language);
   } else {
      values.remove(Language);
   }
}

QString QMediaResource::audioCodec() const
{
   return (values.value(AudioCodec)).value<QString>();
}


void QMediaResource::setAudioCodec(const QString &codec)
{
   if (! codec.isEmpty()) {
      values.insert(AudioCodec, codec);
   } else {
      values.remove(AudioCodec);
   }
}

QString QMediaResource::videoCodec() const
{
   return (values.value(VideoCodec)).value<QString>();
}

void QMediaResource::setVideoCodec(const QString &codec)
{
   if (! codec.isEmpty()) {
      values.insert(VideoCodec, codec);
   } else {
      values.remove(VideoCodec);
   }
}

qint64 QMediaResource::dataSize() const
{
   return (values.value(DataSize)).value<qint64>();
}

void QMediaResource::setDataSize(const qint64 size)
{
   if (size != 0) {
      values.insert(DataSize, size);
   } else {
      values.remove(DataSize);
   }
}

int QMediaResource::audioBitRate() const
{
   return values.value(AudioBitRate).toInt();
}

void QMediaResource::setAudioBitRate(int rate)
{
   if (rate != 0) {
      values.insert(AudioBitRate, rate);
   } else {
      values.remove(AudioBitRate);
   }
}

int QMediaResource::sampleRate() const
{
   return (values.value(SampleRate)).value<int>();
}

void QMediaResource::setSampleRate(int sampleRate)
{
   if (sampleRate != 0) {
      values.insert(SampleRate, sampleRate);
   } else {
      values.remove(SampleRate);
   }
}

int QMediaResource::channelCount() const
{
   return (values.value(ChannelCount)).value<int>();
}

void QMediaResource::setChannelCount(int channels)
{
   if (channels != 0) {
      values.insert(ChannelCount, channels);
   } else {
      values.remove(ChannelCount);
   }
}

int QMediaResource::videoBitRate() const
{
   return values.value(VideoBitRate).toInt();
}

void QMediaResource::setVideoBitRate(int rate)
{
   if (rate != 0) {
      values.insert(VideoBitRate, rate);
   } else {
      values.remove(VideoBitRate);
   }
}

QSize QMediaResource::resolution() const
{
   return (values.value(Resolution)).value<QSize>();
}

void QMediaResource::setResolution(const QSize &resolution)
{
   if (resolution.width() != -1 || resolution.height() != -1) {
      values.insert(Resolution, resolution);
   } else {
      values.remove(Resolution);
   }
}

void QMediaResource::setResolution(int width, int height)
{
   if (width != -1 || height != -1) {
      values.insert(Resolution, QSize(width, height));
   } else {
      values.remove(Resolution);
   }
}
