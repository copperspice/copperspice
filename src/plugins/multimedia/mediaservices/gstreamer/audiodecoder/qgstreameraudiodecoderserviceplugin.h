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

#ifndef QGSTREAMERAUDIODECODERSERVICEPLUGIN_H
#define QGSTREAMERAUDIODECODERSERVICEPLUGIN_H

#include <qaudiodecodercontrol.h>
#include <qmediaservice_provider_plugin.h>
#include <qobject.h>
#include <qset.h>

class QGstreamerAudioDecoderServicePlugin : public QMediaServiceProviderPlugin, public QMediaServiceSupportedFormatsInterface
{
   CS_OBJECT_MULTIPLE(QGstreamerAudioDecoderServicePlugin, QMediaServiceProviderPlugin)

   CS_PLUGIN_IID(QMediaServiceProviderInterface_ID)
   CS_PLUGIN_KEY(Q_MEDIASERVICE_AUDIODECODER)

   CS_INTERFACES(QMediaServiceSupportedFormatsInterface)

 public:
   QMediaService *create(QString const &key) override;
   void release(QMediaService *service) override;

   QMultimedia::SupportEstimate hasSupport(const QString &mimeType, const QStringList &codecs) const override;
   QStringList supportedMimeTypes() const override;

 private:
   void updateSupportedMimeTypes() const;

   mutable QSet<QString> m_supportedMimeTypeSet;
};

#endif
