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

#include <qnetworkconfiguration.h>
#include <qnetworkconfiguration_p.h>

#include <qdebug.h>

QNetworkConfiguration::QNetworkConfiguration()
   : d(nullptr)
{
}

QNetworkConfiguration::QNetworkConfiguration(const QNetworkConfiguration &other)
   : d(other.d)
{
}

QNetworkConfiguration::~QNetworkConfiguration()
{
}

QNetworkConfiguration &QNetworkConfiguration::operator=(const QNetworkConfiguration &other)
{
   d = other.d;
   return *this;
}

bool QNetworkConfiguration::operator==(const QNetworkConfiguration &other) const
{
   return (d == other.d);
}

QString QNetworkConfiguration::name() const
{
   if (! d) {
      return QString();
   }

   QRecursiveMutexLocker locker(&d->mutex);

   return d->name;
}

QString QNetworkConfiguration::identifier() const
{
   if (! d) {
      return QString();
   }

   QRecursiveMutexLocker locker(&d->mutex);

   return d->id;
}

QNetworkConfiguration::Type QNetworkConfiguration::type() const
{
   if (! d) {
      return QNetworkConfiguration::Invalid;
   }

   QRecursiveMutexLocker locker(&d->mutex);

   return d->type;
}

bool QNetworkConfiguration::isValid() const
{
   if (! d) {
      return false;
   }

   QRecursiveMutexLocker locker(&d->mutex);

   return d->isValid;
}

QNetworkConfiguration::StateFlags QNetworkConfiguration::state() const
{
   if (! d) {
      return QNetworkConfiguration::Undefined;
   }

   QRecursiveMutexLocker locker(&d->mutex);

   return d->state;
}

QNetworkConfiguration::Purpose QNetworkConfiguration::purpose() const
{
   if (!d) {
      return QNetworkConfiguration::UnknownPurpose;
   }

   QRecursiveMutexLocker locker(&d->mutex);
   return d->purpose;
}

bool QNetworkConfiguration::isRoamingAvailable() const
{
   if (! d) {
      return false;
   }

   QRecursiveMutexLocker locker(&d->mutex);
   return d->roamingSupported;
}

QList<QNetworkConfiguration> QNetworkConfiguration::children() const
{
   QList<QNetworkConfiguration> results;

   if (! d) {
      return results;
   }

   QRecursiveMutexLocker locker(&d->mutex);

   if (d->type != QNetworkConfiguration::ServiceNetwork || ! d->isValid) {
      return results;
   }

   QMutableMapIterator<unsigned int, QNetworkConfigurationPrivatePointer> i(d->serviceNetworkMembers);
   while (i.hasNext()) {
      i.next();

      QNetworkConfigurationPrivatePointer p = i.value();

      //if we have an invalid member get rid of it -> was deleted earlier on
      {
         QRecursiveMutexLocker childLocker(&p->mutex);

         if (! p->isValid) {
            i.remove();
            continue;
         }
      }

      QNetworkConfiguration item;
      item.d = p;
      results << item;
   }

   return results;
}

QNetworkConfiguration::BearerType QNetworkConfiguration::bearerType() const
{
   if (! isValid()) {
      return BearerUnknown;
   }

   QRecursiveMutexLocker locker(&d->mutex);

   return d->bearerType;
}

QNetworkConfiguration::BearerType QNetworkConfiguration::bearerTypeFamily() const
{
   QNetworkConfiguration::BearerType type = bearerType();

   switch (type) {
      case QNetworkConfiguration::BearerUnknown:
      case QNetworkConfiguration::Bearer2G:
      case QNetworkConfiguration::BearerEthernet:
      case QNetworkConfiguration::BearerWLAN:
      case QNetworkConfiguration::BearerBluetooth:
         return type;

      case QNetworkConfiguration::BearerCDMA2000:
      case QNetworkConfiguration::BearerEVDO:
      case QNetworkConfiguration::BearerWCDMA:
      case QNetworkConfiguration::BearerHSPA:
      case QNetworkConfiguration::Bearer3G:
         return QNetworkConfiguration::Bearer3G;

      case QNetworkConfiguration::BearerWiMAX:
      case QNetworkConfiguration::BearerLTE:
      case QNetworkConfiguration::Bearer4G:
         return QNetworkConfiguration::Bearer4G;

      default:
         qWarning() << "QNetworkConfiguration::bearerTypeFamily() Unknown bearer type " << type;
         return QNetworkConfiguration::BearerUnknown;
   }
}

QString QNetworkConfiguration::bearerTypeName() const
{
   if (! isValid()) {
      return QString();
   }

   QRecursiveMutexLocker locker(&d->mutex);

   if (d->type == QNetworkConfiguration::ServiceNetwork || d->type == QNetworkConfiguration::UserChoice) {
      return QString();
   }

   switch (d->bearerType) {

      case BearerEthernet:
         return QString("Ethernet");

      case BearerWLAN:
         return QString("WLAN");

      case Bearer2G:
         return QString("2G");

      case Bearer3G:
         return QString("3G");

      case Bearer4G:
         return QString("4G");

      case BearerCDMA2000:
         return QString("CDMA2000");

      case BearerWCDMA:
         return QString("WCDMA");

      case BearerHSPA:
         return QString("HSPA");

      case BearerBluetooth:
         return QString("Bluetooth");

      case BearerWiMAX:
         return QString("WiMAX");

      case BearerEVDO:
         return QString("EVDO");

      case BearerLTE:
         return QString("LTE");

      case BearerUnknown:
         break;
   }

   return QString("Unknown");
}

