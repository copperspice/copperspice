/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QNETWORKCONFIGURATION_P_H
#define QNETWORKCONFIGURATION_P_H

#include <qnetworkconfiguration.h>
#include <qshareddata.h>
#include <qmutex.h>
#include <qmap.h>

typedef QExplicitlySharedDataPointer<QNetworkConfigurationPrivate> QNetworkConfigurationPrivatePointer;

class QNetworkConfigurationPrivate : public QSharedData
{
 public:
   QNetworkConfigurationPrivate() :
      mutex(QMutex::Recursive),
      type(QNetworkConfiguration::Invalid),
      purpose(QNetworkConfiguration::UnknownPurpose),
      bearerType(QNetworkConfiguration::BearerUnknown),
      isValid(false), roamingSupported(false) {
   }

   virtual ~QNetworkConfigurationPrivate() {
      //release pointers to member configurations
      serviceNetworkMembers.clear();
   }

   QMap<unsigned int, QNetworkConfigurationPrivatePointer> serviceNetworkMembers;

   mutable QMutex mutex;

   QString name;
   QString id;

   QNetworkConfiguration::StateFlags state;
   QNetworkConfiguration::Type type;
   QNetworkConfiguration::Purpose purpose;
   QNetworkConfiguration::BearerType bearerType;

   bool isValid;
   bool roamingSupported;

 private:
   Q_DISABLE_COPY(QNetworkConfigurationPrivate)
};

Q_DECLARE_METATYPE(QNetworkConfigurationPrivatePointer)

#endif
