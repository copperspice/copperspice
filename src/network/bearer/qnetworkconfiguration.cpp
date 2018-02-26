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

#include <qnetworkconfiguration.h>
#include <qnetworkconfiguration_p.h>
#include <qdebug.h>

QNetworkConfiguration::QNetworkConfiguration()
   : d(0)
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

/*!
    \fn bool QNetworkConfiguration::operator!=(const QNetworkConfiguration &other) const

    Returns true if this configuration is not the same as the \a other
    configuration given; otherwise returns false.
*/

/*!
    Returns the user visible name of this configuration.

    The name may either be the name of the underlying access point or the
    name for service network that this configuration represents.
*/
QString QNetworkConfiguration::name() const
{
   if (!d) {
      return QString();
   }

   QMutexLocker locker(&d->mutex);
   return d->name;
}

/*!
    Returns the unique and platform specific identifier for this network configuration;
    otherwise an empty string.
*/
QString QNetworkConfiguration::identifier() const
{
   if (!d) {
      return QString();
   }

   QMutexLocker locker(&d->mutex);
   return d->id;
}

/*!
    Returns the type of the configuration.

    A configuration can represent a single access point configuration or
    a set of access point configurations. Such a set is called service network.
    A configuration that is based on a service network can potentially support
    roaming of network sessions.
*/
QNetworkConfiguration::Type QNetworkConfiguration::type() const
{
   if (!d) {
      return QNetworkConfiguration::Invalid;
   }

   QMutexLocker locker(&d->mutex);
   return d->type;
}

/*!
    Returns true if this QNetworkConfiguration object is valid.
    A configuration may become invalid if the user deletes the configuration or
    the configuration was default-constructed.

    The addition and removal of configurations can be monitored via the
    QNetworkConfigurationManager.

    \sa QNetworkConfigurationManager
*/
bool QNetworkConfiguration::isValid() const
{
   if (!d) {
      return false;
   }

   QMutexLocker locker(&d->mutex);
   return d->isValid;
}

/*!
    Returns the current state of the configuration.
*/
QNetworkConfiguration::StateFlags QNetworkConfiguration::state() const
{
   if (!d) {
      return QNetworkConfiguration::Undefined;
   }

   QMutexLocker locker(&d->mutex);
   return d->state;
}

/*!
    Returns the purpose of this configuration.

    The purpose field may be used to programmatically determine the
    purpose of a configuration. Such information is usually part of the
    access point or service network meta data.
*/
QNetworkConfiguration::Purpose QNetworkConfiguration::purpose() const
{
   if (!d) {
      return QNetworkConfiguration::UnknownPurpose;
   }

   QMutexLocker locker(&d->mutex);
   return d->purpose;
}

/*!
    Returns true if this configuration supports roaming; otherwise false.
*/
bool QNetworkConfiguration::isRoamingAvailable() const
{
   if (!d) {
      return false;
   }

   QMutexLocker locker(&d->mutex);
   return d->roamingSupported;
}

/*!
    Returns all sub configurations of this network configuration in priority order. The first sub
    configuration in the list has the highest priority.

    Only network configurations of type \l ServiceNetwork can have children. Otherwise this
    function returns an empty list.
*/
QList<QNetworkConfiguration> QNetworkConfiguration::children() const
{
   QList<QNetworkConfiguration> results;

   if (!d) {
      return results;
   }

   QMutexLocker locker(&d->mutex);

   if (d->type != QNetworkConfiguration::ServiceNetwork || !d->isValid) {
      return results;
   }

   QMutableMapIterator<unsigned int, QNetworkConfigurationPrivatePointer> i(d->serviceNetworkMembers);
   while (i.hasNext()) {
      i.next();

      QNetworkConfigurationPrivatePointer p = i.value();

      //if we have an invalid member get rid of it -> was deleted earlier on
      {
         QMutexLocker childLocker(&p->mutex);

         if (!p->isValid) {
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
   if (!isValid()) {
      return BearerUnknown;
   }

   QMutexLocker locker(&d->mutex);

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
         qWarning() << "unknown bearer type" << type;
         return QNetworkConfiguration::BearerUnknown;
   }
}
QString QNetworkConfiguration::bearerTypeName() const
{
   if (!isValid()) {
      return QString();
   }

   QMutexLocker locker(&d->mutex);

   if (d->type == QNetworkConfiguration::ServiceNetwork ||
         d->type == QNetworkConfiguration::UserChoice) {
      return QString();
   }

   switch (d->bearerType) {

      case BearerEthernet:
         return "Ethernet";

      case BearerWLAN:
         return "WLAN";

      case Bearer2G:
         return "2G";

      case Bearer3G:
         return "3G";

      case Bearer4G:
         return "4G";

      case BearerCDMA2000:
         return "CDMA2000";

      case BearerWCDMA:
         return "WCDMA";

      case BearerHSPA:
         return "HSPA";

      case BearerBluetooth:
         return "Bluetooth";

      case BearerWiMAX:
         return "WiMAX";

      case BearerEVDO:
         return "EVDO";

      case BearerLTE:
         return "LTE";

      case BearerUnknown:
         break;
   }

   return "Unknown";
}

