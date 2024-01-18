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

#include <qgstreamermetadataprovider.h>

#include <qdebug.h>
#include <qmediametadata.h>
#include <qgstreamerplayersession.h>

#include <gst/gstversion.h>

using QGstreamerMetaDataKeyLookup = QMap<QByteArray, QString>;
static QGstreamerMetaDataKeyLookup metadataKeys;

static const QGstreamerMetaDataKeyLookup *qt_gstreamerMetaDataKeys()
{
   if (metadataKeys.isEmpty()) {

      metadataKeys.insert(GST_TAG_TITLE,         QMediaMetaData::Title);
      //metadataKeys.insert(0,                   QMediaMetaData::SubTitle);
      //metadataKeys.insert(0,                   QMediaMetaData::Author);

      metadataKeys.insert(GST_TAG_COMMENT,       QMediaMetaData::Comment);
      metadataKeys.insert(GST_TAG_DESCRIPTION,   QMediaMetaData::Description);
      //metadataKeys.insert(0,                   QMediaMetaData::Category);
      metadataKeys.insert(GST_TAG_GENRE,         QMediaMetaData::Genre);
      metadataKeys.insert("year",                QMediaMetaData::Year);
      //metadataKeys.insert(0,                   QMediaMetaData::UserRating);

      metadataKeys.insert(GST_TAG_LANGUAGE_CODE, QMediaMetaData::Language);
      metadataKeys.insert(GST_TAG_ORGANIZATION,  QMediaMetaData::Publisher);
      metadataKeys.insert(GST_TAG_COPYRIGHT,     QMediaMetaData::Copyright);

      //metadataKeys.insert(0,                   QMediaMetaData::ParentalRating);
      //metadataKeys.insert(0,                   QMediaMetaData::RatingOrganisation);

      // Media
      //metadataKeys.insert(0,                   QMediaMetaData::Size);
      //metadataKeys.insert(0,                   QMediaMetaData::MediaType );
      metadataKeys.insert(GST_TAG_DURATION,      QMediaMetaData::Duration);

      // Audio
      metadataKeys.insert(GST_TAG_BITRATE,       QMediaMetaData::AudioBitRate);
      metadataKeys.insert(GST_TAG_AUDIO_CODEC,   QMediaMetaData::AudioCodec);
      //metadataKeys.insert(0,                   QMediaMetaData::ChannelCount);
      //metadataKeys.insert(0,                   QMediaMetaData::SampleRate);

      // Music
      metadataKeys.insert(GST_TAG_ALBUM,         QMediaMetaData::AlbumTitle);

#if GST_CHECK_VERSION(0, 10, 25)
      metadataKeys.insert(GST_TAG_ALBUM_ARTIST,  QMediaMetaData::AlbumArtist);
#endif

      metadataKeys.insert(GST_TAG_ARTIST,        QMediaMetaData::ContributingArtist);
      //metadataKeys.insert(0,                   QMediaMetaData::Conductor);
      //metadataKeys.insert(0,                   QMediaMetaData::Lyrics);
      //metadataKeys.insert(0,                   QMediaMetaData::Mood);
      metadataKeys.insert(GST_TAG_TRACK_NUMBER,  QMediaMetaData::TrackNumber);

      //metadataKeys.insert(0,                   QMediaMetaData::CoverArtUrlSmall);
      //metadataKeys.insert(0,                   QMediaMetaData::CoverArtUrlLarge);

      // Image/Video
      metadataKeys.insert("resolution",          QMediaMetaData::Resolution);
      metadataKeys.insert("pixel-aspect-ratio",  QMediaMetaData::PixelAspectRatio);

      // Video
      //metadataKeys.insert(0,                   QMediaMetaData::VideoFrameRate);
      //metadataKeys.insert(0,                   QMediaMetaData::VideoBitRate);
      metadataKeys.insert(GST_TAG_VIDEO_CODEC,   QMediaMetaData::VideoCodec);

      //metadataKeys.insert(0,                   QMediaMetaData::PosterUrl);

      // Movie
      //metadataKeys.insert(0,                   QMediaMetaData::ChapterNumber);
      //metadataKeys.insert(0,                   QMediaMetaData::Director);
      metadataKeys.insert(GST_TAG_PERFORMER,     QMediaMetaData::LeadPerformer);
      //metadataKeys.insert(0,                   QMediaMetaData::Writer);

      // Photos
      //metadataKeys.insert(0,                   QMediaMetaData::CameraManufacturer);
      //metadataKeys.insert(0,                   QMediaMetaData::CameraModel);
      //metadataKeys.insert(0,                   QMediaMetaData::Event);
      //metadataKeys.insert(0,                   QMediaMetaData::Subject);
   }

   return &metadataKeys;
}

QGstreamerMetaDataProvider::QGstreamerMetaDataProvider(QGstreamerPlayerSession *session, QObject *parent)
   : QMetaDataReaderControl(parent), m_session(session)
{
   connect(m_session, &QGstreamerPlayerSession::tagsChanged, this, &QGstreamerMetaDataProvider::updateTags);
}

QGstreamerMetaDataProvider::~QGstreamerMetaDataProvider()
{
}

bool QGstreamerMetaDataProvider::isMetaDataAvailable() const
{
   return !m_session->tags().isEmpty();
}

bool QGstreamerMetaDataProvider::isWritable() const
{
   return false;
}

QVariant QGstreamerMetaDataProvider::metaData(const QString &key) const
{
   return m_tags.value(key);
}

QStringList QGstreamerMetaDataProvider::availableMetaData() const
{
   return m_tags.keys();
}

void QGstreamerMetaDataProvider::updateTags()
{
   QVariantMap oldTags = m_tags;
   m_tags.clear();

   bool changed = false;

   QMap<QByteArray, QVariant> tagMap = m_session->tags();

   for (auto iter = tagMap.begin(); iter != tagMap.end(); ++iter)  {
      // use gstreamer native keys for elements not in our key map
      QString keyData = qt_gstreamerMetaDataKeys()->value(iter.key(), iter.key());

      m_tags.insert(keyData, iter.value());

      if (iter.value() != oldTags.value(keyData)) {
         changed = true;
         emit metaDataChanged(keyData, iter.value());
      }
   }

   if (oldTags.isEmpty() != m_tags.isEmpty()) {
      emit metaDataAvailableChanged(isMetaDataAvailable());
      changed = true;
   }

   if (changed) {
      emit metaDataChanged();
   }
}

