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

#ifndef CAMERA_SERVICEPLUGIN_H
#define CAMERA_SERVICEPLUGIN_H

#include <qmediaservice_provider_plugin.h>
#include <qgstreamervideoinputdevicecontrol_p.h>

#include <gst/gst.h>

class CameraBinServicePlugin : public QMediaServiceProviderPlugin, public QMediaServiceSupportedDevicesInterface,
      public QMediaServiceDefaultDeviceInterface, public QMediaServiceFeaturesInterface, public QMediaServiceCameraInfoInterface
{
   CS_OBJECT_MULTIPLE(CameraBinServicePlugin, QMediaServiceProviderPlugin)

   CS_PLUGIN_IID(QMediaServiceProviderInterface_ID)
   CS_PLUGIN_KEY(Q_MEDIASERVICE_CAMERA)

   CS_INTERFACES(QMediaServiceSupportedDevicesInterface, QMediaServiceDefaultDeviceInterface,
         QMediaServiceFeaturesInterface, QMediaServiceCameraInfoInterface)

 public:
   CameraBinServicePlugin();
   ~CameraBinServicePlugin();

   QMediaService *create(const QString &key) override;
   void release(QMediaService *service) override;

   QMediaServiceProviderHint::Features supportedFeatures(const QString &service) const override;

   QString defaultDevice(const QString &service) const override;
   QList<QString> devices(const QString &service) const override;
   QString deviceDescription(const QString &service, const QString &device) override;
   QVariant deviceProperty(const QByteArray &service, const QByteArray &device, const QByteArray &property);

   QCamera::Position cameraPosition(const QString &device) const override;
   int cameraOrientation(const QString &device) const override;

 private:
   GstElementFactory *sourceFactory() const;

   mutable GstElementFactory *m_sourceFactory;
};

#endif
