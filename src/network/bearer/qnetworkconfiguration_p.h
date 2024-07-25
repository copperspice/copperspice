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

#ifndef QNETWORKCONFIGURATION_P_H
#define QNETWORKCONFIGURATION_P_H

#include <qnetworkconfiguration.h>

#include <qmap.h>
#include <qmutex.h>
#include <qshareddata.h>

using QNetworkConfigurationPrivatePointer = QExplicitlySharedDataPointer<QNetworkConfigurationPrivate>;

class QNetworkConfigurationPrivate : public QSharedData
{
 public:
   QNetworkConfigurationPrivate()
      : type(QNetworkConfiguration::Invalid), purpose(QNetworkConfiguration::UnknownPurpose),
      bearerType(QNetworkConfiguration::BearerUnknown), isValid(false), roamingSupported(false)
   {
   }

   QNetworkConfigurationPrivate(const QNetworkConfigurationPrivate &) = delete;
   QNetworkConfigurationPrivate &operator=(const QNetworkConfigurationPrivate &) = delete;

   virtual ~QNetworkConfigurationPrivate() {
      //release pointers to member configurations
      serviceNetworkMembers.clear();
   }

   QMap<unsigned int, QNetworkConfigurationPrivatePointer> serviceNetworkMembers;

   mutable QRecursiveMutex mutex;

   QString name;
   QString id;

   QNetworkConfiguration::StateFlags state;
   QNetworkConfiguration::Type type;
   QNetworkConfiguration::Purpose purpose;
   QNetworkConfiguration::BearerType bearerType;

   bool isValid;
   bool roamingSupported;
};

#endif
