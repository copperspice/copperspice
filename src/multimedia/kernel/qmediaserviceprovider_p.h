/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qobject.h>
#include <qshareddata.h>
#include <qmultimedia.h>
#include <qmediaservice_provider_plugin.h>

class QMediaService;

class Q_MULTIMEDIA_EXPORT QMediaServiceProvider : public QObject
{
   MULTI_CS_OBJECT(QMediaServiceProvider)

 public:
   virtual QMediaService *requestService(const QByteArray &type, const QMediaServiceProviderHint &hint = QMediaServiceProviderHint()) = 0;
   virtual void releaseService(QMediaService *service) = 0;

   virtual QMediaServiceProviderHint::Features supportedFeatures(const QMediaService *service) const;

   virtual QMultimedia::SupportEstimate hasSupport(const QByteArray &serviceType,
      const QString &mimeType, const QStringList &codecs, int flags = 0) const;

   virtual QStringList supportedMimeTypes(const QByteArray &serviceType, int flags = 0) const;

   virtual QByteArray defaultDevice(const QByteArray &serviceType) const;
   virtual QList<QByteArray> devices(const QByteArray &serviceType) const;
   virtual QString deviceDescription(const QByteArray &serviceType, const QByteArray &device);

   // emerald   virtual QCamera::Position cameraPosition(const QByteArray &device) const;
   // emerald    virtual int cameraOrientation(const QByteArray &device) const;

   static QMediaServiceProvider *defaultServiceProvider();
   static void setDefaultServiceProvider(QMediaServiceProvider *provider);
};

#endif
