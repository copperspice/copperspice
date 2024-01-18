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

#include <qgstreameraudiodecoderserviceplugin.h>

#include <qdebug.h>
#include <qdir.h>
#include <qdebug.h>
#include <qstring.h>
#include <qgstreameraudiodecoderservice.h>

#include <qgstutils_p.h>

CS_PLUGIN_REGISTER(QGstreamerAudioDecoderServicePlugin)

QMediaService *QGstreamerAudioDecoderServicePlugin::create(const QString &key)
{
   QGstUtils::initializeGst();

   if (key == Q_MEDIASERVICE_AUDIODECODER) {
      return new QGstreamerAudioDecoderService;
   }

   qWarning() << "GStreamer audio decoder service plugin, unsupported key:" << key;

   return nullptr;
}

void QGstreamerAudioDecoderServicePlugin::release(QMediaService *service)
{
   delete service;
}

QMultimedia::SupportEstimate QGstreamerAudioDecoderServicePlugin::hasSupport(const QString &mimeType, const QStringList &codecs) const
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
      || gst_element_factory_list_is_type(factory, GST_ELEMENT_FACTORY_TYPE_DECODER
         | GST_ELEMENT_FACTORY_TYPE_MEDIA_AUDIO);
#else
   return (factory && (qstrcmp(factory->details.klass, "Codec/Decoder/Audio") == 0
            || qstrcmp(factory->details.klass, "Codec/Demux") == 0));
#endif
}

void QGstreamerAudioDecoderServicePlugin::updateSupportedMimeTypes() const
{
   m_supportedMimeTypeSet = QGstUtils::supportedMimeTypes(isDecoderOrDemuxer);
}

QStringList QGstreamerAudioDecoderServicePlugin::supportedMimeTypes() const
{
   return QStringList();
}

