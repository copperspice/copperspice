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

#ifndef QHTTPNETWORKCONNECTION_P_H
#define QHTTPNETWORKCONNECTION_P_H

#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <qabstractsocket.h>
#include <qnetworksession.h>
#include <qauthenticator.h>
#include <qnetworkproxy.h>
#include <qbuffer.h>

#include <qscopedpointer.h>
#include <qtimer.h>

#include <qhttpnetworkheader_p.h>
#include <qhttpnetworkrequest_p.h>
#include <qhttpnetworkreply_p.h>
#include <qhttpnetworkconnectionchannel_p.h>

#ifdef QT_SSL

#ifdef QT_OPENSSL
#    include <qsslcontext_openssl_p.h>
#endif

#    include <qsslsocket_p.h>
#    include <qsslsocket.h>
#    include <qsslerror.h>
#else
#   include <qtcpsocket.h>
#endif

class QHttpNetworkRequest;
class QHttpNetworkReply;
class QHttpThreadDelegate;
class QByteArray;
class QHostInfo;
class QHttpNetworkConnectionPrivate;

using HttpMessagePair = QPair<QHttpNetworkRequest, QHttpNetworkReply*>;

class QHttpNetworkConnection : public QObject
{
   NET_CS_OBJECT(QHttpNetworkConnection)

 public:

    enum ConnectionType {
        ConnectionTypeHTTP,
        ConnectionTypeSPDY
    };

#ifndef QT_NO_BEARERMANAGEMENT
   explicit QHttpNetworkConnection(const QString &hostName, quint16 port = 80, bool encrypt = false,
                          ConnectionType connectionType = ConnectionTypeHTTP, QObject *parent = nullptr,
                          QSharedPointer<QNetworkSession> networkSession = QSharedPointer<QNetworkSession>());

   QHttpNetworkConnection(quint16 channelCount, const QString &hostName, quint16 port = 80,
                          bool encrypt = false, QObject *parent = nullptr,
                          QSharedPointer<QNetworkSession> networkSession = QSharedPointer<QNetworkSession>(),
                          ConnectionType connectionType = ConnectionTypeHTTP);

#else
   explicit QHttpNetworkConnection(const QString &hostName, quint16 port = 80, bool encrypt = false,
                          ConnectionType connectionType = ConnectionTypeHTTP,
                          QObject *parent = nullptr);

   QHttpNetworkConnection(quint16 channelCount, const QString &hostName, quint16 port = 80,
                          bool encrypt = false, QObject *parent = nullptr,
                          ConnectionType connectionType = ConnectionTypeHTTP);
#endif

   ~QHttpNetworkConnection();

   //The hostname to which this is connected to.
   QString hostName() const;

   //The HTTP port in use.
   quint16 port() const;

   //add a new HTTP request through this connection
   QHttpNetworkReply *sendRequest(const QHttpNetworkRequest &request);

#ifndef QT_NO_NETWORKPROXY
   //set the proxy for this connection
   void setCacheProxy(const QNetworkProxy &networkProxy);
   QNetworkProxy cacheProxy() const;
   void setTransparentProxy(const QNetworkProxy &networkProxy);
   QNetworkProxy transparentProxy() const;
#endif

   bool isSsl() const;
   QHttpNetworkConnectionChannel *channels() const;

    ConnectionType connectionType();
    void setConnectionType(ConnectionType type);

#ifdef QT_SSL
   void setSslConfiguration(const QSslConfiguration &config);
   void ignoreSslErrors(int channel = -1);
   void ignoreSslErrors(const QList<QSslError> &errors, int channel = -1);
   QSharedPointer<QSslContext> sslContext();
   void setSslContext(QSharedPointer<QSslContext> context);
#endif

   void preConnectFinished();

 protected:
   QScopedPointer<QHttpNetworkConnectionPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QHttpNetworkConnection)
   Q_DISABLE_COPY(QHttpNetworkConnection)
   friend class QHttpThreadDelegate;
   friend class QHttpNetworkReply;
   friend class QHttpNetworkReplyPrivate;
   friend class QHttpNetworkConnectionChannel;
   friend class QHttpProtocolHandler;
   friend class QSpdyProtocolHandler;

   NET_CS_SLOT_1(Private, void _q_startNextRequest())
   NET_CS_SLOT_2(_q_startNextRequest)

   NET_CS_SLOT_1(Private, void _q_hostLookupFinished(const QHostInfo &))
   NET_CS_SLOT_2(_q_hostLookupFinished)

   NET_CS_SLOT_1(Private, void _q_connectDelayedChannel())
   NET_CS_SLOT_2(_q_connectDelayedChannel)
};

class QHttpNetworkConnectionPrivate
{
   Q_DECLARE_PUBLIC(QHttpNetworkConnection)

 public:
   static const int defaultHttpChannelCount;
   static const int defaultPipelineLength;
   static const int defaultRePipelineLength;

   enum ConnectionState {
      RunningState = 0,
      PausedState = 1,
   };

    enum NetworkLayerPreferenceState {
        Unknown,
        HostLookupPending,
        IPv4,
        IPv6,
        IPv4or6
    };
   QHttpNetworkConnectionPrivate(const QString &hostName, quint16 port, bool encrypt,
                  QHttpNetworkConnection::ConnectionType type);

   QHttpNetworkConnectionPrivate(quint16 channelCount, const QString &hostName, quint16 port, bool encrypt,
                  QHttpNetworkConnection::ConnectionType type);

   virtual ~QHttpNetworkConnectionPrivate();
   void init();

   void pauseConnection();
   void resumeConnection();
   ConnectionState state;
   NetworkLayerPreferenceState networkLayerState;

   enum { ChunkSize = 4096 };

   int indexOf(QAbstractSocket *socket) const;

   QHttpNetworkReply *queueRequest(const QHttpNetworkRequest &request);
   void requeueRequest(const HttpMessagePair &pair); // e.g. after pipeline broke
   bool dequeueRequest(QAbstractSocket *socket);
   void prepareRequest(HttpMessagePair &request);
   void updateChannel(int i, const HttpMessagePair &messagePair);
   QHttpNetworkRequest predictNextRequest();

   void fillPipeline(QAbstractSocket *socket);
   bool fillPipeline(QList<HttpMessagePair> &queue, QHttpNetworkConnectionChannel &channel);

   // read more HTTP body after the next event loop spin
   void readMoreLater(QHttpNetworkReply *reply);

   void copyCredentials(int fromChannel, QAuthenticator *auth, bool isProxy);

   void startHostInfoLookup();
   void startNetworkLayerStateLookup();
   void networkLayerDetected(QAbstractSocket::NetworkLayerProtocol protocol);
   void _q_startNextRequest(); // send the next request from the queue

   void _q_hostLookupFinished(const QHostInfo &info);
   void _q_connectDelayedChannel();
   void createAuthorization(QAbstractSocket *socket, QHttpNetworkRequest &request);

   QString errorDetail(QNetworkReply::NetworkError errorCode, QAbstractSocket *socket,
                       const QString &extraDetail = QString());

   void removeReply(QHttpNetworkReply *reply);

   QString hostName;
   quint16 port;
   bool encrypt;
   bool delayIpv4;

   const int channelCount;
   QTimer delayedConnectionTimer;
   QHttpNetworkConnectionChannel *channels;                      // parallel connections to the server
   bool shouldEmitChannelError(QAbstractSocket *socket);

   qint64 uncompressedBytesAvailable(const QHttpNetworkReply &reply) const;
   qint64 uncompressedBytesAvailableNextBlock(const QHttpNetworkReply &reply) const;


   void emitReplyError(QAbstractSocket *socket, QHttpNetworkReply *reply, QNetworkReply::NetworkError errorCode);
   bool handleAuthenticateChallenge(QAbstractSocket *socket, QHttpNetworkReply *reply, bool isProxy, bool &resend);
   QUrl parseRedirectResponse(QAbstractSocket *socket, QHttpNetworkReply *reply);

#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy networkProxy;
   void emitProxyAuthenticationRequired(const QHttpNetworkConnectionChannel *chan, const QNetworkProxy &proxy,
                  QAuthenticator *auth);
#endif

   //The request queues
   QList<HttpMessagePair> highPriorityQueue;
   QList<HttpMessagePair> lowPriorityQueue;

   int preConnectRequests;
   QHttpNetworkConnection::ConnectionType connectionType;
#ifdef QT_SSL
    QSharedPointer<QSslContext> sslContext;
#endif
#ifndef QT_NO_BEARERMANAGEMENT
   QSharedPointer<QNetworkSession> networkSession;
#endif

   friend class QHttpNetworkConnectionChannel;

 protected:
   QHttpNetworkConnection *q_ptr;

};


#endif
