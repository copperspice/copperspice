/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtNetwork/qnetworkinterface.h>
#include <QtCore/qvariant.h>
#include <QtNetwork/qnetworkconfiguration.h>

#ifndef QT_NO_BEARERMANAGEMENT

#if defined(Q_OS_WIN) && defined(interface)
#undef interface
#endif

#ifndef QT_MOBILITY_BEARER
#include <QtCore/qshareddata.h>
QT_BEGIN_NAMESPACE
#define QNetworkSessionExport Q_NETWORK_EXPORT

#else
#include <qmobilityglobal.h>
QTM_BEGIN_NAMESPACE
#define QNetworkSessionExport Q_BEARER_EXPORT

#endif

class QNetworkSessionPrivate;

class QNetworkSessionExport QNetworkSession : public QObject
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

   bool waitForOpened(int msecs = 30000);

   NET_CS_SLOT_1(Public, void open())
   NET_CS_SLOT_2(open)

   NET_CS_SLOT_1(Public, void close())
   NET_CS_SLOT_2(close)

   NET_CS_SLOT_1(Public, void stop())
   NET_CS_SLOT_2(stop)

   //roaming related slots
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

 protected:
   virtual void connectNotify(const char *signal);
   virtual void disconnectNotify(const char *signal);

 private:
   friend class QNetworkSessionPrivate;
   QNetworkSessionPrivate *d;
};

#ifndef QT_MOBILITY_BEARER
QT_END_NAMESPACE
Q_DECLARE_METATYPE(QNetworkSession::State)
Q_DECLARE_METATYPE(QNetworkSession::SessionError)

#else
QTM_END_NAMESPACE

#endif

#endif // QT_NO_BEARERMANAGEMENT

#endif // QNETWORKSESSION_H
