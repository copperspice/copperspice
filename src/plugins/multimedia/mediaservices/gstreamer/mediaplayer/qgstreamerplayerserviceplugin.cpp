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

#include <qdebug.h>
#include <qdir.h>
#include <qdebug.h>
#include <qstring.h>
#include <qgstreamerplayerserviceplugin.h>
#include <qgstreamerplayerservice.h>

#include <qgstutils_p.h>

CS_PLUGIN_REGISTER(QGstreamerPlayerServicePlugin)

QMediaService *QGstreamerPlayerServicePlugin::create(const QString &key)
{
   QGstUtils::initializeGst();

   if (key == Q_MEDIASERVICE_MEDIAPLAYER) {
      return new QGstreamerPlayerService;
   }

   qWarning() << "GStreamer media player service plugin, unsupported key:" << key;

   return nullptr;
}

void QGstreamerPlayerServicePlugin::release(QMediaService *service)
{
   delete service;
}

QMediaServiceProviderHint::Features QGstreamerPlayerServicePlugin::supportedFeatures(const QString &service) const
{
   if (service == Q_MEDIASERVICE_MEDIAPLAYER)
      return

#ifdef HAVE_GST_APPSRC
         QMediaServiceProviderHint::StreamPlayback |
#endif
         QMediaServiceProviderHint::VideoSurface;

   else {
      return QMediaServiceProviderHint::Features();
   }
}

QMultimedia::SupportEstimate QGstreamerPlayerServicePlugin::hasSupport(const QString &mimeType, const QStringList &codecs) const
{
   if (m_supportedMimeTypeSet.isEmpty()) {
      updateSupportedMimeTypes();
   }

   return QGstUtils::hasSupport(mimeType, codecs, m_supportedMimeTypeSet);
}

static bool isDecoderOrDemuxer(GstElementFactory *factory)
{
#if GST_CHECK_VERSION(0, 10, 31)
   return gst_element_factory_list_is_type(factory, GST_ELEMENT_FACTORY_TYPE_DEMUXER)
      || gst_element_factory_list_is_type(factory, GST_ELEMENT_FACTORY_TYPE_DECODER);

#else
   return (factory
         && (qstrcmp(factory->details.klass,   "Codec/Decoder/Audio") == 0
            || qstrcmp(factory->details.klass, "Codec/Decoder/Video") == 0
            || qstrcmp(factory->details.klass, "Codec/Demux") == 0 ));
#endif
}

void QGstreamerPlayerServicePlugin::updateSupportedMimeTypes() const
{
   m_supportedMimeTypeSet = QGstUtils::supportedMimeTypes(isDecoderOrDemuxer);
}

QStringList QGstreamerPlayerServicePlugin::supportedMimeTypes() const
{
   return QStringList();
}

