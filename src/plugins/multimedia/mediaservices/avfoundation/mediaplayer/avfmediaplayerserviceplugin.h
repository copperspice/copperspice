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

#ifndef AVFMEDIAPLAYERSERVICEPLUGIN_H
#define AVFMEDIAPLAYERSERVICEPLUGIN_H

#include <qglobal.h>
#include <qmediaservice_provider_plugin.h>
#include <qmediaplayercontrol.h>

class AVFMediaPlayerServicePlugin
   : public QMediaServiceProviderPlugin, public QMediaServiceSupportedFormatsInterface
   , public QMediaServiceFeaturesInterface
{
   CS_OBJECT_MULTIPLE(AVFMediaPlayerServicePlugin, QMediaServiceProviderPlugin)

   CS_PLUGIN_IID(QMediaServiceProviderInterface_ID)
   CS_PLUGIN_KEY(QMediaPlayerControl_Key)

   CS_INTERFACES(QMediaServiceSupportedFormatsInterface, QMediaServiceFeaturesInterface)

 public:
   explicit AVFMediaPlayerServicePlugin();

   QMediaService *create(const QString &key) override;
   void release(QMediaService *service) override;

   QMediaServiceProviderHint::Features supportedFeatures(const QString &service) const override;
   QMultimedia::SupportEstimate hasSupport(const QString &mimeType, const QStringList &codecs) const override;
   QStringList supportedMimeTypes() const override;

 private:
   void buildSupportedTypes();

   QStringList m_supportedMimeTypes;
};

#endif
