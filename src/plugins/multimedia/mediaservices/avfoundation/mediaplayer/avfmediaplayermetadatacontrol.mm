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

#include "avfmediaplayermetadatacontrol.h"
#include "avfmediaplayersession.h"

#include <qmediametadata.h>

#import <AVFoundation/AVFoundation.h>

AVFMediaPlayerMetaDataControl::AVFMediaPlayerMetaDataControl(AVFMediaPlayerSession *session, QObject *parent)
   : QMetaDataReaderControl(parent), m_session(session), m_asset(nullptr)
{
   QObject::connect(m_session, &AVFMediaPlayerSession::mediaStatusChanged, this, &AVFMediaPlayerMetaDataControl::updateTags);
}

AVFMediaPlayerMetaDataControl::~AVFMediaPlayerMetaDataControl()
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO;
#endif
}

bool AVFMediaPlayerMetaDataControl::isMetaDataAvailable() const
{
   return !m_tags.isEmpty();
}

bool AVFMediaPlayerMetaDataControl::isWritable() const
{
   return false;
}

QVariant AVFMediaPlayerMetaDataControl::metaData(const QString &key) const
{
   return m_tags.value(key);
}

QStringList AVFMediaPlayerMetaDataControl::availableMetaData() const
{
   return m_tags.keys();
}

static QString itemKey(AVMetadataItem *item)
{
   NSString *keyString = [item commonKey];

   if (keyString.length != 0) {
      if ([keyString isEqualToString: AVMetadataCommonKeyTitle]) {
         return QMediaMetaData::Title;
      } else if ([keyString isEqualToString: AVMetadataCommonKeySubject]) {
         return QMediaMetaData::SubTitle;
      } else if ([keyString isEqualToString: AVMetadataCommonKeyDescription]) {
         return QMediaMetaData::Description;
      } else if ([keyString isEqualToString: AVMetadataCommonKeyPublisher]) {
         return QMediaMetaData::Publisher;
      } else if ([keyString isEqualToString: AVMetadataCommonKeyCreationDate]) {
         return QMediaMetaData::Date;
      } else if ([keyString isEqualToString: AVMetadataCommonKeyType]) {
         return QMediaMetaData::MediaType;
      } else if ([keyString isEqualToString: AVMetadataCommonKeyLanguage]) {
         return QMediaMetaData::Language;
      } else if ([keyString isEqualToString: AVMetadataCommonKeyCopyrights]) {
         return QMediaMetaData::Copyright;
      } else if ([keyString isEqualToString: AVMetadataCommonKeyAlbumName]) {
         return QMediaMetaData::AlbumTitle;
      } else if ([keyString isEqualToString: AVMetadataCommonKeyAuthor]) {
         return QMediaMetaData::Author;
      } else if ([keyString isEqualToString: AVMetadataCommonKeyArtist]) {
         return QMediaMetaData::ContributingArtist;
      } else if ([keyString isEqualToString: AVMetadataCommonKeyArtwork]) {
         return QMediaMetaData::PosterUrl;
      }
   }

   return QString();
}

void AVFMediaPlayerMetaDataControl::updateTags()
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO;
#endif

   AVAsset *currentAsset = (AVAsset *)m_session->currentAssetHandle();

   // Don't read the tags from the same asset more than once
   if (currentAsset == m_asset) {
      return;
   }

   m_asset = currentAsset;

   QVariantMap oldTags = m_tags;

   // Since we have changed assets, clear old tags
   m_tags.clear();
   bool changed = false;

   // TODO: also process ID3, iTunes and QuickTime metadata

   NSArray *metadataItems = [currentAsset commonMetadata];
   for (AVMetadataItem * item in metadataItems) {
      const QString key = itemKey(item);

      if (! key.isEmpty()) {
         const QString value = QString::fromNSString([item stringValue]);

         if (! value.isEmpty()) {
            m_tags.insert(key, value);

            if (value != oldTags.value(key).toString()) {
               changed = true;
               metaDataChanged(key, value);
            }
         }
      }
   }

   if (oldTags.isEmpty() != m_tags.isEmpty()) {
      metaDataAvailableChanged(! m_tags.isEmpty());
      changed = true;
   }

   if (changed) {
      metaDataChanged();
   }
}
