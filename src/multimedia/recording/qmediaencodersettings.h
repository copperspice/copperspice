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

#ifndef QMEDIAENCODERSETTINGS_H
#define QMEDIAENCODERSETTINGS_H

#include <qsharedpointer.h>
#include <qstring.h>
#include <qsize.h>
#include <qvariant.h>
#include <qmultimedia.h>

class QAudioEncoderSettingsPrivate;
class QImageEncoderSettingsPrivate;
class QVideoEncoderSettingsPrivate;

class Q_MULTIMEDIA_EXPORT QAudioEncoderSettings
{
 public:
   QAudioEncoderSettings();
   QAudioEncoderSettings(const QAudioEncoderSettings &other);

   ~QAudioEncoderSettings();

   QAudioEncoderSettings &operator=(const QAudioEncoderSettings &other);
   bool operator==(const QAudioEncoderSettings &other) const;
   bool operator!=(const QAudioEncoderSettings &other) const;

   bool isNull() const;

   QMultimedia::EncodingMode encodingMode() const;
   void setEncodingMode(QMultimedia::EncodingMode mode);

   QString codec() const;
   void setCodec(const QString &codecName);

   int bitRate() const;
   void setBitRate(int bitrate);

   int channelCount() const;
   void setChannelCount(int channels);

   int sampleRate() const;
   void setSampleRate(int rate);

   QMultimedia::EncodingQuality quality() const;
   void setQuality(QMultimedia::EncodingQuality quality);

   QVariant encodingOption(const QString &option) const;
   QVariantMap encodingOptions() const;
   void setEncodingOption(const QString &option, const QVariant &value);
   void setEncodingOptions(const QVariantMap &options);

 private:
   QSharedDataPointer<QAudioEncoderSettingsPrivate> d;
};


class Q_MULTIMEDIA_EXPORT QVideoEncoderSettings
{
 public:
   QVideoEncoderSettings();
   QVideoEncoderSettings(const QVideoEncoderSettings &other);

   ~QVideoEncoderSettings();

   QVideoEncoderSettings &operator=(const QVideoEncoderSettings &other);
   bool operator==(const QVideoEncoderSettings &other) const;
   bool operator!=(const QVideoEncoderSettings &other) const;

   bool isNull() const;

   QMultimedia::EncodingMode encodingMode() const;
   void setEncodingMode(QMultimedia::EncodingMode mode);

   QString codec() const;
   void setCodec(const QString &codecName);

   QSize resolution() const;
   void setResolution(const QSize &resolution);
   void setResolution(int width, int height);

   qreal frameRate() const;
   void setFrameRate(qreal rate);

   int bitRate() const;
   void setBitRate(int bitrate);

   QMultimedia::EncodingQuality quality() const;
   void setQuality(QMultimedia::EncodingQuality quality);

   QVariant encodingOption(const QString &option) const;
   QVariantMap encodingOptions() const;
   void setEncodingOption(const QString &option, const QVariant &value);
   void setEncodingOptions(const QVariantMap &options);

 private:
   QSharedDataPointer<QVideoEncoderSettingsPrivate> d;
};


class Q_MULTIMEDIA_EXPORT QImageEncoderSettings
{
 public:
   QImageEncoderSettings();
   QImageEncoderSettings(const QImageEncoderSettings &other);

   ~QImageEncoderSettings();

   QImageEncoderSettings &operator=(const QImageEncoderSettings &other);
   bool operator==(const QImageEncoderSettings &other) const;
   bool operator!=(const QImageEncoderSettings &other) const;

   bool isNull() const;

   QString codec() const;
   void setCodec(const QString &codecName);

   QSize resolution() const;
   void setResolution(const QSize &resolution);
   void setResolution(int width, int height);

   QMultimedia::EncodingQuality quality() const;
   void setQuality(QMultimedia::EncodingQuality quality);

   QVariant encodingOption(const QString &option) const;
   QVariantMap encodingOptions() const;
   void setEncodingOption(const QString &option, const QVariant &value);
   void setEncodingOptions(const QVariantMap &options);

 private:
   QSharedDataPointer<QImageEncoderSettingsPrivate> d;
};

#endif
