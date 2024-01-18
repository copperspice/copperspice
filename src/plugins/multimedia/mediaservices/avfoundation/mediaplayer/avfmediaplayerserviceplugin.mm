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

#include <avfmediaplayerserviceplugin.h>

#include <qdebug.h>
#include <avfmediaplayerservice.h>

#import <AVFoundation/AVFoundation.h>

CS_PLUGIN_REGISTER(AVFMediaPlayerServicePlugin)

AVFMediaPlayerServicePlugin::AVFMediaPlayerServicePlugin()
{
   buildSupportedTypes();
}

QMediaService *AVFMediaPlayerServicePlugin::create(const QString &key)
{
   if (key == QMediaPlayerControl_Key) {
      return new AVFMediaPlayerService;
   }

   qWarning() << "AVFoundation media player service plugin, unsupported key:" << key;

   return nullptr;
}

void AVFMediaPlayerServicePlugin::release(QMediaService *service)
{
   delete service;
}

QMediaServiceProviderHint::Features AVFMediaPlayerServicePlugin::supportedFeatures(const QString &service) const
{
   if (service == Q_MEDIASERVICE_MEDIAPLAYER) {
      return QMediaServiceProviderHint::VideoSurface;
   } else {
      return QMediaServiceProviderHint::Features();
   }
}

QMultimedia::SupportEstimate AVFMediaPlayerServicePlugin::hasSupport(const QString &mimeType, const QStringList &codecs) const
{
   if (m_supportedMimeTypes.contains(mimeType)) {
      return QMultimedia::ProbablySupported;
   }

   return QMultimedia::MaybeSupported;
}

QStringList AVFMediaPlayerServicePlugin::supportedMimeTypes() const
{
   return m_supportedMimeTypes;
}

void AVFMediaPlayerServicePlugin::buildSupportedTypes()
{
   //Populate m_supportedMimeTypes with mimetypes AVAsset supports
   NSArray *mimeTypes = [AVURLAsset audiovisualMIMETypes];
   for (NSString * mimeType in mimeTypes) {
      m_supportedMimeTypes.append(QString::fromUtf8([mimeType UTF8String]));
   }
#ifdef QT_DEBUG_AVF
   qDebug() << "AVFMediaPlayerServicePlugin::buildSupportedTypes";
   qDebug() << "Supported Types: " << m_supportedMimeTypes;
#endif

}
