/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QHTTPNETWORKCONNECTIONCHANNEL_P_H
#define QHTTPNETWORKCONNECTIONCHANNEL_P_H

#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qabstractsocket.h>

#include <qauthenticator.h>
#include <qnetworkproxy.h>
#include <qbuffer.h>

#include <qhttpnetworkheader_p.h>
#include <qhttpnetworkrequest_p.h>
#include <qhttpnetworkreply_p.h>
#include <qhttpnetworkconnection_p.h>

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

#ifndef HttpMessagePair
typedef QPair<QHttpNetworkRequest, QHttpNetworkReply *> HttpMessagePair;
#endif

class QHttpNetworkConnectionChannel : public QObject
{
   NET_CS_OBJECT(QHttpNetworkConnectionChannel)

 public:
   enum ChannelState {
      IdleState = 0,          // ready to send request
      ConnectingState = 1,    // connecting to host
      WritingState = 2,       // writing the data
      WaitingState = 4,       // waiting for reply
      ReadingState = 8,       // reading the reply
      ClosingState = 16,
      BusyState = (ConnectingState | WritingState | WaitingState | ReadingState | ClosingState)
   };

   QAbstractSocket *socket;
   bool ssl;
   ChannelState state;
   QHttpNetworkRequest request; // current request
   QHttpNetworkReply *reply; // current reply for this request
   qint64 written;
   qint64 bytesTotal;
   bool resendCurrent;
   int lastStatus; // last status received on this channel
   bool pendingEncrypt; // for https (send after encrypted)
   int reconnectAttempts; // maximum 2 reconnection attempts
   QAuthenticatorPrivate::Method authMethod;
   QAuthenticatorPrivate::Method proxyAuthMethod;
   QAuthenticator authenticator;
   QAuthenticator proxyAuthenticator;
   bool authenticationCredentialsSent;
   bool proxyCredentialsSent;

#ifndef QT_NO_OPENSSL
   bool ignoreAllSslErrors;
   QList<QSslError> ignoreSslErrorsList;
#endif

#ifndef QT_NO_BEARERMANAGEMENT
   QSharedPointer<QNetworkSession> networkSession;
#endif

   // HTTP pipelining -> http://en.wikipedia.org/wiki/Http_pipelining
   enum PipeliningSupport {
      PipeliningSupportUnknown, // default for a new connection
      PipeliningProbablySupported, // after having received a server response that indicates support
      PipeliningNotSupported // currently not used
   };
   PipeliningSupport pipeliningSupported;
   QList<HttpMessagePair> alreadyPipelinedRequests;
   QByteArray pipeline; // temporary buffer that gets sent to socket in pipelineFlush
   void pipelineInto(HttpMessagePair &pair);
   void pipelineFlush();
   void requeueCurrentlyPipelinedRequests();
   void detectPipeliningSupport();

   QHttpNetworkConnectionChannel();

   void setConnection(QHttpNetworkConnection *c);
   QPointer<QHttpNetworkConnection> connection;

   void init();
   void close();

   bool sendRequest();

   bool ensureConnection();

   bool expand(bool dataComplete);
   void allDone(); // reply header + body have been read
   void handleStatus(); // called from allDone()

   bool resetUploadData(); // return true if resetting worked or there is no upload data

   void handleUnexpectedEOF();
   void closeAndResendCurrentRequest();

   bool isSocketBusy() const;
   bool isSocketWriting() const;
   bool isSocketWaiting() const;
   bool isSocketReading() const;

   friend class QNetworkAccessHttpBackend;

 protected :
   NET_CS_SLOT_1(Protected, void _q_receiveReply())
   NET_CS_SLOT_2(_q_receiveReply)
   NET_CS_SLOT_1(Protected, void _q_bytesWritten(qint64 bytes))
   NET_CS_SLOT_2(_q_bytesWritten)  // proceed sending
   NET_CS_SLOT_1(Protected, void _q_readyRead())
   NET_CS_SLOT_2(_q_readyRead)  // pending data to read
   NET_CS_SLOT_1(Protected, void _q_disconnected())
   NET_CS_SLOT_2(_q_disconnected)  // disconnected from host
   NET_CS_SLOT_1(Protected, void _q_connected())
   NET_CS_SLOT_2(_q_connected)  // start sending request
   NET_CS_SLOT_1(Protected, void _q_error(QAbstractSocket::SocketError un_named_arg1))
   NET_CS_SLOT_2(_q_error)  // error from socket

#ifndef QT_NO_NETWORKPROXY
   NET_CS_SLOT_1(Protected, void _q_proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth))
   NET_CS_SLOT_2(_q_proxyAuthenticationRequired)  // from transparent proxy
#endif

   NET_CS_SLOT_1(Protected, void _q_uploadDataReadyRead())
   NET_CS_SLOT_2(_q_uploadDataReadyRead)

#ifndef QT_NO_OPENSSL
   NET_CS_SLOT_1(Protected, void _q_encrypted())
   NET_CS_SLOT_2(_q_encrypted)  // start sending request (https)
   NET_CS_SLOT_1(Protected, void _q_sslErrors(const QList <QSslError> &errors))
   NET_CS_SLOT_2(_q_sslErrors)  // ssl errors from the socket
   NET_CS_SLOT_1(Protected, void _q_encryptedBytesWritten(qint64 bytes))
   NET_CS_SLOT_2(_q_encryptedBytesWritten)  // proceed sending
#endif
};

QT_END_NAMESPACE

#endif // QT_NO_HTTP

#endif
