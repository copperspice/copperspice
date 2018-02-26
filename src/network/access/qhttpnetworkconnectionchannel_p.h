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

#ifndef QHTTPNETWORKCONNECTIONCHANNEL_P_H
#define QHTTPNETWORKCONNECTIONCHANNEL_P_H

#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <qabstractsocket.h>

#include <qauthenticator.h>
#include <qnetworkproxy.h>
#include <qbuffer.h>

#include <qhttpnetworkheader_p.h>
#include <qhttpnetworkrequest_p.h>
#include <qhttpnetworkreply_p.h>
#include <qhttpnetworkconnection_p.h>
#include <qabstractprotocolhandler_p.h>

#ifdef QT_SSL
#   include <qsslsocket.h>
#   include <qsslerror.h>
#   include <qsslconfiguration.h>
#else
#   include <qtcpsocket.h>
#endif

class QHttpNetworkRequest;
class QHttpNetworkReply;
class QByteArray;

using HttpMessagePair = QPair<QHttpNetworkRequest, QHttpNetworkReply*>;

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
   bool isInitialized;
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
   QScopedPointer<QAbstractProtocolHandler> protocolHandler;

#ifdef QT_SSL
   bool ignoreAllSslErrors;
   QList<QSslError> ignoreSslErrorsList;
   QSslConfiguration sslConfiguration;
   QMultiMap<int, HttpMessagePair> spdyRequestsToSend; // sorted by priority

   void ignoreSslErrors();
   void ignoreSslErrors(const QList<QSslError> &errors);
   void setSslConfiguration(const QSslConfiguration &config);
   void requeueSpdyRequests(); // when we wanted SPDY but got HTTP
   void emitFinishedWithError(QNetworkReply::NetworkError error, const char *message);
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

   QAbstractSocket::NetworkLayerProtocol networkLayerPreference;
   void setConnection(QHttpNetworkConnection *c);
   QPointer<QHttpNetworkConnection> connection;

#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy proxy;
    void setProxy(const QNetworkProxy &networkProxy);
#endif
   void init();
   void close();
   void abort();

   bool sendRequest();
   bool ensureConnection();

   void allDone(); // reply header + body have been read
   void handleStatus(); // called from allDone()

   bool resetUploadData(); // return true if resetting worked or there is no upload data

   void handleUnexpectedEOF();
   void closeAndResendCurrentRequest();
   void resendCurrentRequest();

   bool isSocketBusy() const;
   bool isSocketWriting() const;
   bool isSocketWaiting() const;
   bool isSocketReading() const;


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

#ifdef QT_SSL
   NET_CS_SLOT_1(Protected, void _q_encrypted())
   NET_CS_SLOT_2(_q_encrypted)  // start sending request (https)

   NET_CS_SLOT_1(Protected, void _q_sslErrors(const QList <QSslError> &errors))
   NET_CS_SLOT_2(_q_sslErrors)  // ssl errors from the socket

   NET_CS_SLOT_1(Protected, void _q_preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *))
   NET_CS_SLOT_2(_q_preSharedKeyAuthenticationRequired)

   NET_CS_SLOT_1(Protected, void _q_encryptedBytesWritten(qint64 bytes))
   NET_CS_SLOT_2(_q_encryptedBytesWritten)  // proceed sending
#endif
   friend class QHttpProtocolHandler;
};




#endif
