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

#ifndef QNETWORKSESSION_H
#define QNETWORKSESSION_H

#include <qobject.h>
#include <qstring.h>
#include <qnetworkinterface.h>
#include <qvariant.h>
#include <qnetworkconfiguration.h>

#ifndef QT_NO_BEARERMANAGEMENT

#if defined(Q_OS_WIN) && defined(interface)
#undef interface
#endif

#include <qshareddata.h>

class QNetworkSessionPrivate;

class Q_NETWORK_EXPORT QNetworkSession : public QObject
{
   NET_CS_OBJECT(QNetworkSession)

 public:
   enum State {
      Invalid = 0,
      NotAvailable,
      Connecting,
      Connected,
      Closing,
      Disconnected,
      Roaming
   };

   enum SessionError {
      UnknownSessionError = 0,
      SessionAbortedError,
      RoamingError,
      OperationNotSupportedError,
      InvalidConfigurationError
   };

   enum UsagePolicy {
        NoPolicy = 0,
        NoBackgroundTrafficPolicy = 1
   };
   using UsagePolicies = QFlags<UsagePolicy>;

   explicit QNetworkSession(const QNetworkConfiguration &connConfig, QObject *parent = nullptr);
   virtual ~QNetworkSession();

   bool isOpen() const;
   QNetworkConfiguration configuration() const;

#ifndef QT_NO_NETWORKINTERFACE
   QNetworkInterface interface() const;
#endif

   State state() const;
   SessionError error() const;
   QString errorString() const;
   QVariant sessionProperty(const QString &key) const;
   void setSessionProperty(const QString &key, const QVariant &value);

   quint64 bytesWritten() const;
   quint64 bytesReceived() const;
   quint64 activeTime() const;

   QNetworkSession::UsagePolicies usagePolicies() const;
   bool waitForOpened(int msecs = 30000);

   NET_CS_SLOT_1(Public, void open())
   NET_CS_SLOT_2(open)

   NET_CS_SLOT_1(Public, void close())
   NET_CS_SLOT_2(close)

   NET_CS_SLOT_1(Public, void stop())
   NET_CS_SLOT_2(stop)

   // roaming related slots
   NET_CS_SLOT_1(Public, void migrate())
   NET_CS_SLOT_2(migrate)

   NET_CS_SLOT_1(Public, void ignore())
   NET_CS_SLOT_2(ignore)

   NET_CS_SLOT_1(Public, void accept())
   NET_CS_SLOT_2(accept)

   NET_CS_SLOT_1(Public, void reject())
   NET_CS_SLOT_2(reject)

   NET_CS_SIGNAL_1(Public, void stateChanged(QNetworkSession::State un_named_arg1))
   NET_CS_SIGNAL_2(stateChanged, un_named_arg1)

   NET_CS_SIGNAL_1(Public, void opened())
   NET_CS_SIGNAL_2(opened)

   NET_CS_SIGNAL_1(Public, void closed())
   NET_CS_SIGNAL_2(closed)

   NET_CS_SIGNAL_1(Public, void error(QNetworkSession::SessionError un_named_arg1))
   NET_CS_SIGNAL_OVERLOAD(error, (QNetworkSession::SessionError), un_named_arg1)

   NET_CS_SIGNAL_1(Public, void preferredConfigurationChanged(const QNetworkConfiguration &config, bool isSeamless))
   NET_CS_SIGNAL_2(preferredConfigurationChanged, config, isSeamless)

   NET_CS_SIGNAL_1(Public, void newConfigurationActivated())
   NET_CS_SIGNAL_2(newConfigurationActivated)

   NET_CS_SIGNAL_1(Public, void usagePoliciesChanged(QNetworkSession::UsagePolicies usagePolicies))
   NET_CS_SIGNAL_2(usagePoliciesChanged, usagePolicies)

 protected:
   void connectNotify(const QMetaMethod &signal) const override;
   void disconnectNotify(const QMetaMethod &signal) const override;

 private:
   friend class QNetworkSessionPrivate;
   QNetworkSessionPrivate *d;
};

Q_DECLARE_METATYPE(QSharedPointer<QNetworkSession>)
Q_DECLARE_METATYPE(QNetworkSession::State)
Q_DECLARE_METATYPE(QNetworkSession::SessionError)
Q_DECLARE_METATYPE(QNetworkSession::UsagePolicies)

#endif // QT_NO_BEARERMANAGEMENT

#endif // QNETWORKSESSION_H
