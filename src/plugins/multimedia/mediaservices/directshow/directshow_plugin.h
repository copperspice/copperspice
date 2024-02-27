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

#ifndef DIRECTSHOW_PLUGIN_H
#define DIRECTSHOW_PLUGIN_H

#include <qmediaservice_provider_plugin.h>
#include <qmediaplayercontrol.h>

class DSServicePlugin
   : public QMediaServiceProviderPlugin, public QMediaServiceSupportedDevicesInterface,
     public QMediaServiceDefaultDeviceInterface, public QMediaServiceFeaturesInterface
{
   CS_OBJECT_MULTIPLE(DSServicePlugin, QMediaServiceProviderPlugin)

   CS_PLUGIN_IID(QMediaServiceProviderInterface_ID)
   CS_PLUGIN_KEY(QString(Q_MEDIASERVICE_MEDIAPLAYER) + ", " + Q_MEDIASERVICE_CAMERA)

   CS_INTERFACES(QMediaServiceSupportedDevicesInterface, QMediaServiceDefaultDeviceInterface, QMediaServiceFeaturesInterface)

 public:
   QMediaService *create(const QString  &key) override;
   void release(QMediaService *service) override;

   QMediaServiceProviderHint::Features supportedFeatures(const QString  &service) const override;

   QString  defaultDevice(const QString &service) const override;
   QList<QString > devices(const QString  &service) const override;
   QString deviceDescription(const QString  &service, const QString  &device) override;
};

#endif
