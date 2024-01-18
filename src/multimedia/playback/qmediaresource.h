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

#ifndef QMEDIARESOURCE_H
#define QMEDIARESOURCE_H

#include <qmap.h>
#include <qstring.h>
#include <qnetwork_request.h>

class Q_MULTIMEDIA_EXPORT QMediaResource
{
 public:
   QMediaResource();
   QMediaResource(const QUrl &url, const QString &mimeType = QString());
   QMediaResource(const QNetworkRequest &request, const QString &mimeType = QString());
   QMediaResource(const QMediaResource &other);
   QMediaResource &operator =(const QMediaResource &other);
   ~QMediaResource();

   bool isNull() const;

   bool operator ==(const QMediaResource &other) const;
   bool operator !=(const QMediaResource &other) const;

   QUrl url() const;
   QNetworkRequest request() const;
   QString mimeType() const;

   QString language() const;
   void setLanguage(const QString &language);

   QString audioCodec() const;
   void setAudioCodec(const QString &codec);

   QString videoCodec() const;
   void setVideoCodec(const QString &codec);

   qint64 dataSize() const;
   void setDataSize(const qint64 size);

   int audioBitRate() const;
   void setAudioBitRate(int rate);

   int sampleRate() const;
   void setSampleRate(int sampleRate);

   int channelCount() const;
   void setChannelCount(int channels);

   int videoBitRate() const;
   void setVideoBitRate(int rate);

   QSize resolution() const;
   void setResolution(const QSize &resolution);
   void setResolution(int width, int height);

 private:
   enum Property {
      Url,
      Request,
      MimeType,
      Language,
      AudioCodec,
      VideoCodec,
      DataSize,
      AudioBitRate,
      VideoBitRate,
      SampleRate,
      ChannelCount,
      Resolution
   };
   QMap<int, QVariant> values;
};

#endif
