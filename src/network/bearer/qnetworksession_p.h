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

#ifndef QNETWORKSESSION_P_H
#define QNETWORKSESSION_P_H

#include <qnetworksession.h>
#include <qnetworkconfiguration_p.h>
#include <QtCore/qsharedpointer.h>

#ifndef QT_NO_BEARERMANAGEMENT

QT_BEGIN_NAMESPACE

class Q_NETWORK_EXPORT QNetworkSessionPrivate : public QObject
{
   NET_CS_OBJECT(QNetworkSessionPrivate)

   friend class QNetworkSession;

 public:
   QNetworkSessionPrivate() : QObject(),
      state(QNetworkSession::Invalid), isOpen(false), mutex(QMutex::Recursive) {
   }

   virtual ~QNetworkSessionPrivate() {
   }

   //called by QNetworkSession constructor and ensures
   //that the state is immediately updated (w/o actually opening
   //a session). Also this function should take care of
   //notification hooks to discover future state changes.
   virtual void syncStateWithInterface() = 0;

#ifndef QT_NO_NETWORKINTERFACE
   virtual QNetworkInterface currentInterface() const = 0;
#endif
   virtual QVariant sessionProperty(const QString &key) const = 0;
   virtual void setSessionProperty(const QString &key, const QVariant &value) = 0;

   virtual void open() = 0;
   virtual void close() = 0;
   virtual void stop() = 0;

   virtual void setALREnabled(bool /*enabled*/) {}
   virtual void migrate() = 0;
   virtual void accept() = 0;
   virtual void ignore() = 0;
   virtual void reject() = 0;

   virtual QString errorString() const = 0; //must return translated string
   virtual QNetworkSession::SessionError error() const = 0;

   virtual quint64 bytesWritten() const = 0;
   virtual quint64 bytesReceived() const = 0;
   virtual quint64 activeTime() const = 0;

   virtual QNetworkSession::UsagePolicies usagePolicies() const = 0;
   virtual void setUsagePolicies(QNetworkSession::UsagePolicies) = 0;

   static void setUsagePolicies(QNetworkSession&, QNetworkSession::UsagePolicies); //for unit testing

   //releases any pending waitForOpened() calls
   NET_CS_SIGNAL_1(Public, void quitPendingWaitsForOpened())
   NET_CS_SIGNAL_2(quitPendingWaitsForOpened)

   NET_CS_SIGNAL_1(Public, void error(QNetworkSession::SessionError error))
   NET_CS_SIGNAL_OVERLOAD(error, (QNetworkSession::SessionError), error)

   NET_CS_SIGNAL_1(Public, void stateChanged(QNetworkSession::State state))
   NET_CS_SIGNAL_2(stateChanged, state)

   NET_CS_SIGNAL_1(Public, void closed())
   NET_CS_SIGNAL_2(closed)

   NET_CS_SIGNAL_1(Public, void newConfigurationActivated())
   NET_CS_SIGNAL_2(newConfigurationActivated)

   NET_CS_SIGNAL_1(Public, void preferredConfigurationChanged(const QNetworkConfiguration &config, bool isSeamless))
   NET_CS_SIGNAL_2(preferredConfigurationChanged, config, isSeamless)

   NET_CS_SIGNAL_1(Public, void usagePoliciesChanged(QNetworkSession::UsagePolicies data))
   NET_CS_SIGNAL_2(usagePoliciesChanged, data)

 protected:
   inline QNetworkConfigurationPrivatePointer privateConfiguration(const QNetworkConfiguration &config) const {
      return config.d;
   }

   inline void setPrivateConfiguration(QNetworkConfiguration &config, QNetworkConfigurationPrivatePointer ptr) const {
      config.d = ptr;
   }

   QNetworkSession *q;

   // The config set on QNetworkSession.
   QNetworkConfiguration publicConfig;

   // If publicConfig is a ServiceNetwork this is a copy of publicConfig.
   // If publicConfig is an UserChoice that is resolved to a ServiceNetwork this is the actual
   // ServiceNetwork configuration.
   QNetworkConfiguration serviceConfig;

   // This is the actual active configuration currently in use by the session.
   // Either a copy of publicConfig or one of serviceConfig.children().
   QNetworkConfiguration activeConfig;

   QNetworkSession::State state;
   bool isOpen;

   QMutex mutex;
};



#endif // QT_NO_BEARERMANAGEMENT

#endif // QNETWORKSESSIONPRIVATE_H
