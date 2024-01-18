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

#ifndef QMEDIARESOURCEPOLICYPLUGIN_P_H
#define QMEDIARESOURCEPOLICYPLUGIN_P_H

#include <qobject.h>

struct Q_MULTIMEDIA_EXPORT QMediaResourceSetFactoryInterface {
   virtual QObject *create(const QString &interfaceId) = 0;
   virtual void destroy(QObject *resourceSet) = 0;
};

#define QMediaResourceSetFactoryInterface_iid "com.copperspice.CS.mediaResourceSetFactory/1.0"
CS_DECLARE_INTERFACE(QMediaResourceSetFactoryInterface, QMediaResourceSetFactoryInterface_iid)

class Q_MULTIMEDIA_EXPORT QMediaResourcePolicyPlugin : public QObject, public QMediaResourceSetFactoryInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QMediaResourcePolicyPlugin, QObject)
   CS_INTERFACES(QMediaResourceSetFactoryInterface)

 public:
   QMediaResourcePolicyPlugin(QObject *parent = nullptr);
   ~QMediaResourcePolicyPlugin();
};


#endif
