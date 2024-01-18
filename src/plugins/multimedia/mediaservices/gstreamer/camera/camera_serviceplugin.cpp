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

#include <camera_serviceplugin.h>
#include <camera_service.h>
#include <qdebug.h>
#include <qdir.h>
#include <qstring.h>

#include <qgstutils_p.h>

CS_PLUGIN_REGISTER(CameraBinServicePlugin)

template <typename T, int N>
static int lengthOf(const T(&)[N])
{
   return N;
}

CameraBinServicePlugin::CameraBinServicePlugin()
   : m_sourceFactory(nullptr)
{
}

CameraBinServicePlugin::~CameraBinServicePlugin()
{
   if (m_sourceFactory) {
      gst_object_unref(GST_OBJECT(m_sourceFactory));
   }
}

QMediaService *CameraBinServicePlugin::create(const QString &key)
{
   QGstUtils::initializeGst();

   if (key == Q_MEDIASERVICE_CAMERA) {
      if (! CameraBinService::isCameraBinAvailable()) {
         guint major, minor, micro, nano;
         gst_version(&major, &minor, &micro, &nano);

         qWarning("Error: Unable to create camera service, the 'camerabin' plugin is missing for "
                  "GStreamer %u.%u.\n Please install the 'bad' GStreamer plugin package.", major, minor);

         return nullptr;
      }

      return new CameraBinService(sourceFactory());
   }

   qWarning() << "GStreamer camerabin service plugin, unsupported key:" << key;

   return nullptr;
}

void CameraBinServicePlugin::release(QMediaService *service)
{
   delete service;
}

QMediaServiceProviderHint::Features CameraBinServicePlugin::supportedFeatures(const QString &service) const
{
   if (service == Q_MEDIASERVICE_CAMERA) {
      return QMediaServiceProviderHint::VideoSurface;
   }

   return QMediaServiceProviderHint::Features();
}

QString CameraBinServicePlugin::defaultDevice(const QString &service) const
{
   return service == Q_MEDIASERVICE_CAMERA
          ? QGstUtils::enumerateCameras(sourceFactory()).value(0).name : QString();
}

QList<QString> CameraBinServicePlugin::devices(const QString &service) const
{
   return service == Q_MEDIASERVICE_CAMERA ? QGstUtils::cameraDevices(m_sourceFactory) : QList<QString>();
}

QString CameraBinServicePlugin::deviceDescription(const QString &service, const QString &deviceName)
{
   return service == Q_MEDIASERVICE_CAMERA ? QGstUtils::cameraDescription(deviceName, m_sourceFactory) : QString();
}

QVariant CameraBinServicePlugin::deviceProperty(const QByteArray &service, const QByteArray &device, const QByteArray &property)
{
   (void) service;
   (void) device;
   (void) property;

   return QVariant();
}

QCamera::Position CameraBinServicePlugin::cameraPosition(const QString &deviceName) const
{
   return QGstUtils::cameraPosition(deviceName, m_sourceFactory);
}

int CameraBinServicePlugin::cameraOrientation(const QString &deviceName) const
{
   return QGstUtils::cameraOrientation(deviceName, m_sourceFactory);
}

GstElementFactory *CameraBinServicePlugin::sourceFactory() const
{
   if (!m_sourceFactory) {
      GstElementFactory *factory = nullptr;

      const QByteArray envCandidate = qgetenv("QT_GSTREAMER_CAMERABIN_SRC");
      if (!envCandidate.isEmpty()) {
         factory = gst_element_factory_find(envCandidate.constData());
      }

      static const char *candidates[] = { "subdevsrc", "wrappercamerabinsrc" };
      for (int i = 0; !factory && i < lengthOf(candidates); ++i) {
         factory = gst_element_factory_find(candidates[i]);
      }

      if (factory) {
         m_sourceFactory = GST_ELEMENT_FACTORY(gst_plugin_feature_load(GST_PLUGIN_FEATURE(factory)));
         gst_object_unref((GST_OBJECT(factory)));
      }
   }

   return m_sourceFactory;
}

