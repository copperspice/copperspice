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

#ifndef QMEDIASERVICEPROVIDER_H
#define QMEDIASERVICEPROVIDER_H

#include <qmediaservice_provider_plugin.h>
#include <qmultimedia.h>
#include <qobject.h>
#include <qshareddata.h>

class QMediaService;

class Q_MULTIMEDIA_EXPORT QMediaServiceProvider : public QObject
{
   MULTI_CS_OBJECT(QMediaServiceProvider)

 public:
   virtual QMediaService *requestService(const QString &type, const QMediaServiceProviderHint &hint = QMediaServiceProviderHint()) = 0;
   virtual void releaseService(QMediaService *service) = 0;

   virtual QMediaServiceProviderHint::Features supportedFeatures(const QMediaService *service) const;

   virtual QMultimedia::SupportEstimate hasSupport(const QString &serviceType,
         const QString &mimeType, const QStringList &codecs, int flags = 0) const;

   virtual QStringList supportedMimeTypes(const QString &serviceType, int flags = 0) const;

   virtual QString defaultDevice(const QString &serviceType) const;
   virtual QList<QString> devices(const QString &serviceType) const;
   virtual QString deviceDescription(const QString &serviceType, const QString &device);

   virtual QCamera::Position cameraPosition(const QString &device) const;
   virtual int cameraOrientation(const QString &device) const;

   static QMediaServiceProvider *defaultServiceProvider();
   static void setDefaultServiceProvider(QMediaServiceProvider *provider);
};

#endif
