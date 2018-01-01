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

#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qabstractsocket.h>
#include <QtNetwork/qnetworksession.h>
#include <qauthenticator.h>
#include <qnetworkproxy.h>
#include <qbuffer.h>
#include <QScopedPointer>

#include <qhttpnetworkheader_p.h>
#include <qhttpnetworkrequest_p.h>
#include <qhttpnetworkreply_p.h>
#include <qhttpnetworkconnectionchannel_p.h>

#ifndef QT_NO_HTTP

#ifndef QT_NO_OPENSSL
#    include <QtNetwork/qsslsocket.h>
#    include <QtNetwork/qsslerror.h>
#else
#   include <QtNetwork/qtcpsocket.h>
#endif

QT_BEGIN_NAMESPACE

class QHttpNetworkRequest;
class QHttpNetworkReply;
class QByteArray;
class QHttpNetworkConnectionPrivate;

class QHttpNetworkConnection : public QObject
{
   NET_CS_OBJECT(QHttpNetworkConnection)

 public:

#ifndef QT_NO_BEARERMANAGEMENT
   QHttpNetworkConnection(const QString &hostName, quint16 port = 80, bool encrypt = false,
                          QObject *parent = nullptr, QSharedPointer<QNetworkSession> networkSession = QSharedPointer<QNetworkSession>());

   QHttpNetworkConnection(quint16 channelCount, const QString &hostName, quint16 port = 80,
                          bool encrypt = false, QObject *parent = nullptr,
                          QSharedPointer<QNetworkSession> networkSession = QSharedPointer<QNetworkSession>());

#else
   QHttpNetworkConnection(const QString &hostName, quint16 port = 80, bool encrypt = false,
                          QObject *parent = nullptr);

   QHttpNetworkConnection(quint16 channelCount, const QString &hostName,
                          quint16 port = 80, bool encrypt = false, QObject *parent = nullptr);
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

#ifndef QT_NO_OPENSSL
   void setSslConfiguration(const QSslConfiguration &config);
   void ignoreSslErrors(int channel = -1);
   void ignoreSslErrors(const QList<QSslError> &errors, int channel = -1);
#endif

 protected:
   QScopedPointer<QHttpNetworkConnectionPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QHttpNetworkConnection)
   Q_DISABLE_COPY(QHttpNetworkConnection)

   friend class QHttpNetworkReply;
   friend class QHttpNetworkReplyPrivate;
   friend class QHttpNetworkConnectionChannel;

   NET_CS_SLOT_1(Private, void _q_startNextRequest())
   NET_CS_SLOT_2(_q_startNextRequest) 
};


// private classes
typedef QPair<QHttpNetworkRequest, QHttpNetworkReply *> HttpMessagePair;


class QHttpNetworkConnectionPrivate
{
   Q_DECLARE_PUBLIC(QHttpNetworkConnection)

 public:
   static const int defaultChannelCount;
   static const int defaultPipelineLength;
   static const int defaultRePipelineLength;

   enum ConnectionState {
      RunningState = 0,
      PausedState = 1,
   };

   QHttpNetworkConnectionPrivate(const QString &hostName, quint16 port, bool encrypt);
   QHttpNetworkConnectionPrivate(quint16 channelCount, const QString &hostName, quint16 port, bool encrypt);

   virtual ~QHttpNetworkConnectionPrivate();
   void init();

   void pauseConnection();
   void resumeConnection();
   ConnectionState state;

   enum { ChunkSize = 4096 };

   int indexOf(QAbstractSocket *socket) const;

   QHttpNetworkReply *queueRequest(const QHttpNetworkRequest &request);
   void requeueRequest(const HttpMessagePair &pair); // e.g. after pipeline broke
   bool dequeueRequest(QAbstractSocket *socket);
   void prepareRequest(HttpMessagePair &request);
   QHttpNetworkRequest predictNextRequest();

   void fillPipeline(QAbstractSocket *socket);
   bool fillPipeline(QList<HttpMessagePair> &queue, QHttpNetworkConnectionChannel &channel);

   // read more HTTP body after the next event loop spin
   void readMoreLater(QHttpNetworkReply *reply);

   void copyCredentials(int fromChannel, QAuthenticator *auth, bool isProxy);

   void _q_startNextRequest(); // send the next request from the queue

   void createAuthorization(QAbstractSocket *socket, QHttpNetworkRequest &request);

   QString errorDetail(QNetworkReply::NetworkError errorCode, QAbstractSocket *socket,
                       const QString &extraDetail = QString());

#ifndef QT_NO_COMPRESS
   bool expand(QAbstractSocket *socket, QHttpNetworkReply *reply, bool dataComplete);
#endif
   void removeReply(QHttpNetworkReply *reply);

   QString hostName;
   quint16 port;
   bool encrypt;

   const int channelCount;
   QHttpNetworkConnectionChannel *channels; // parallel connections to the server

   qint64 uncompressedBytesAvailable(const QHttpNetworkReply &reply) const;
   qint64 uncompressedBytesAvailableNextBlock(const QHttpNetworkReply &reply) const;


   void emitReplyError(QAbstractSocket *socket, QHttpNetworkReply *reply, QNetworkReply::NetworkError errorCode);
   bool handleAuthenticateChallenge(QAbstractSocket *socket, QHttpNetworkReply *reply, bool isProxy, bool &resend);

#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy networkProxy;
   void emitProxyAuthenticationRequired(const QHttpNetworkConnectionChannel *chan, const QNetworkProxy &proxy,
                                        QAuthenticator *auth);
#endif

   //The request queues
   QList<HttpMessagePair> highPriorityQueue;
   QList<HttpMessagePair> lowPriorityQueue;

#ifndef QT_NO_BEARERMANAGEMENT
   QSharedPointer<QNetworkSession> networkSession;
#endif

   friend class QHttpNetworkConnectionChannel;

 protected:
   QHttpNetworkConnection *q_ptr;

};

QT_END_NAMESPACE

#endif // QT_NO_HTTP

#endif
