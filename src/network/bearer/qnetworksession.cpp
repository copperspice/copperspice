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

#include <qnetworksession.h>
#include <qnetworksession_p.h>
#include <qbearerengine_p.h>
#include <qnetworkconfigmanager_p.h>

#include <QEventLoop>
#include <QTimer>
#include <QThread>


#ifdef interface
#  undef interface
#endif

#ifndef QT_NO_BEARERMANAGEMENT

QNetworkSession::QNetworkSession(const QNetworkConfiguration &connectionConfig, QObject *parent)
   : QObject(parent), d(0)
{
   // invalid configuration
   if (!connectionConfig.identifier().isEmpty()) {
      for (QBearerEngine *engine : qNetworkConfigurationManagerPrivate()->engines()) {
         if (engine->hasIdentifier(connectionConfig.identifier())) {
            d = engine->createSessionBackend();
            d->q = this;
            d->publicConfig = connectionConfig;
            d->syncStateWithInterface();

            connect(d, SIGNAL(quitPendingWaitsForOpened()),          this, SLOT(opened()));
            connect(d, SIGNAL(error(QNetworkSession::SessionError)), this, SLOT(error(QNetworkSession::SessionError)));

            connect(d, SIGNAL(stateChanged(QNetworkSession::State)), this, SLOT(stateChanged(QNetworkSession::State)));
            connect(d, SIGNAL(closed()), this, SLOT(closed()));

            connect(d, SIGNAL(preferredConfigurationChanged(QNetworkConfiguration, bool)),
                    this, SLOT(preferredConfigurationChanged(QNetworkConfiguration, bool)));

            connect(d, SIGNAL(newConfigurationActivated()), this, SLOT(newConfigurationActivated()));

            connect(d, SIGNAL(usagePoliciesChanged(QNetworkSession::UsagePolicies)),
                    this, SLOT(usagePoliciesChanged(QNetworkSession::UsagePolicies)));
            break;
         }
      }
   }
}

/*!
    Frees the resources associated with the QNetworkSession object.
*/
QNetworkSession::~QNetworkSession()
{
   delete d;
}

void QNetworkSession::open()
{
   if (d) {
      d->open();
   } else {
      emit error(InvalidConfigurationError);
   }
}

bool QNetworkSession::waitForOpened(int msecs)
{
   if (!d) {
      return false;
   }

   if (d->isOpen) {
      return true;
   }

   if (!(d->state == Connecting || d->state == Connected)) {
      return false;
   }

   QEventLoop loop;
   QObject::connect(d, SIGNAL(quitPendingWaitsForOpened()), &loop, SLOT(quit()));
   QObject::connect(this, SIGNAL(error(QNetworkSession::SessionError)), &loop, SLOT(quit()));

   //final call
   if (msecs >= 0) {
      QTimer::singleShot(msecs, &loop, SLOT(quit()));
   }

   // enter the event loop and wait for opened/error/timeout
   loop.exec(QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents);

   return d->isOpen;
}

void QNetworkSession::close()
{
   if (d) {
      d->close();
   }
}

void QNetworkSession::stop()
{
   if (d) {
      d->stop();
   }
}

/*!
    Returns the QNetworkConfiguration that this network session object is based on.

    \sa QNetworkConfiguration
*/
QNetworkConfiguration QNetworkSession::configuration() const
{
   return d ? d->publicConfig : QNetworkConfiguration();
}

#ifndef QT_NO_NETWORKINTERFACE

QNetworkInterface QNetworkSession::interface() const
{
   return d ? d->currentInterface() : QNetworkInterface();
}
#endif

bool QNetworkSession::isOpen() const
{
   return d ? d->isOpen : false;
}

QNetworkSession::State QNetworkSession::state() const
{
   return d ? d->state : QNetworkSession::Invalid;
}

/*!
    Returns the type of error that last occurred.

    \sa state(), errorString()
*/
QNetworkSession::SessionError QNetworkSession::error() const
{
   return d ? d->error() : InvalidConfigurationError;
}

/*!
    Returns a human-readable description of the last device error that
    occurred.

    \sa error()
*/
QString QNetworkSession::errorString() const
{
   return d ? d->errorString() : tr("Invalid configuration.");
}

QVariant QNetworkSession::sessionProperty(const QString &key) const
{
   if (!d || !d->publicConfig.isValid()) {
      return QVariant();
   }

   if (key == QLatin1String("ActiveConfiguration")) {
      return d->isOpen ? d->activeConfig.identifier() : QString();
   }

   if (key == QLatin1String("UserChoiceConfiguration")) {
      if (!d->isOpen || d->publicConfig.type() != QNetworkConfiguration::UserChoice) {
         return QString();
      }

      if (d->serviceConfig.isValid()) {
         return d->serviceConfig.identifier();
      } else {
         return d->activeConfig.identifier();
      }
   }

   return d->sessionProperty(key);
}

void QNetworkSession::setSessionProperty(const QString &key, const QVariant &value)
{
   if (!d) {
      return;
   }

   if (key == QLatin1String("ActiveConfiguration") ||
         key == QLatin1String("UserChoiceConfiguration")) {
      return;
   }

   d->setSessionProperty(key, value);
}

/*!
    Instructs the session to roam to the new access point. The old access point remains active
    until the application calls accept().

   The newConfigurationActivated() signal is emitted once roaming has been completed.

    \sa accept()
*/
void QNetworkSession::migrate()
{
   if (d) {
      d->migrate();
   }
}

/*!
    This function indicates that the application does not wish to roam the session.

    \sa migrate()
*/
void QNetworkSession::ignore()
{
   // Needed on at least Symbian platform: the roaming must be explicitly
   // ignore()'d or migrate()'d
   if (d) {
      d->ignore();
   }
}

void QNetworkSession::accept()
{
   if (d) {
      d->accept();
   }
}

/*!
    The new access point is not suitable for the application. By calling this function the
    session returns to the previous access point/configuration. This action may invalidate
    any socket that has been created via the not desired access point.

    \sa accept()
*/
void QNetworkSession::reject()
{
   if (d) {
      d->reject();
   }
}

quint64 QNetworkSession::bytesWritten() const
{
   return d ? d->bytesWritten() : Q_UINT64_C(0);
}

quint64 QNetworkSession::bytesReceived() const
{
   return d ? d->bytesReceived() : Q_UINT64_C(0);
}

/*!
    Returns the number of seconds that the session has been active.
*/
quint64 QNetworkSession::activeTime() const
{
   return d ? d->activeTime() : Q_UINT64_C(0);
}


QNetworkSession::UsagePolicies QNetworkSession::usagePolicies() const
{
   return d ? d->usagePolicies() : QNetworkSession::NoPolicy;
}

void QNetworkSessionPrivate::setUsagePolicies(QNetworkSession &session, QNetworkSession::UsagePolicies policies)
{
   if (!session.d) {
      return;
   }
   session.d->setUsagePolicies(policies);
}


void QNetworkSession::connectNotify(const QMetaMethod &signal) const
{
   QObject::connectNotify(signal);

   if (! d) {
      return;
   }

   // check for preferredConfigurationChanged() signal connect notification
   // This is not required on all platforms

   static const QMetaMethod preferredConfigurationChangedSignal =
      QMetaMethod::fromSignal(&QNetworkSession::preferredConfigurationChanged);

   if (signal == preferredConfigurationChangedSignal) {
      d->setALREnabled(true);
   }
}

/*!
    \internal

    This function is called when the client disconnects from the
    preferredConfigurationChanged() signal.

    \sa connectNotify()
*/
void QNetworkSession::disconnectNotify(const QMetaMethod &signal) const
{
   QObject::disconnectNotify(signal);

   if (! d) {
      return;
   }

   //check for preferredConfigurationChanged() signal disconnect notification
   //This is not required on all platforms
   static const QMetaMethod preferredConfigurationChangedSignal =
      QMetaMethod::fromSignal(&QNetworkSession::preferredConfigurationChanged);

   if (signal == preferredConfigurationChangedSignal) {
      d->setALREnabled(false);
   }
}


#endif // QT_NO_BEARERMANAGEMENT
