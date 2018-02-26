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

#ifndef QHTTPNETWORKREPLY_P_H
#define QHTTPNETWORKREPLY_P_H

#include <qplatformdefs.h>

#ifndef QT_NO_COMPRESS
struct z_stream_s;
#endif

#include <QtNetwork/qtcpsocket.h>

// it's safe to include these even if SSL support is not enabled
#include <QtNetwork/qsslsocket.h>
#include <QtNetwork/qsslerror.h>

#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <qbuffer.h>
#include <QScopedPointer>

#include <qhttpnetworkheader_p.h>
#include <qhttpnetworkrequest_p.h>
#include <qauthenticator_p.h>
#include <qringbuffer_p.h>
#include <qbytedata_p.h>

class QHttpNetworkConnection;
class QHttpNetworkConnectionChannel;
class QHttpNetworkRequest;
class QHttpNetworkConnectionPrivate;
class QHttpNetworkReplyPrivate;

class QHttpNetworkReply : public QObject, public QHttpNetworkHeader
{
   NET_CS_OBJECT_MULTIPLE(QHttpNetworkReply, QObject)

 public:
   explicit QHttpNetworkReply(const QUrl &url = QUrl(), QObject *parent = nullptr);
   virtual ~QHttpNetworkReply();

   QUrl url() const override;
   void setUrl(const QUrl &url) override;

   int majorVersion() const override;
   int minorVersion() const override;

   qint64 contentLength() const override;
   void setContentLength(qint64 length) override;

   QList<QPair<QByteArray, QByteArray> > header() const override;
   QByteArray headerField(const QByteArray &name, const QByteArray &defaultValue = QByteArray()) const override;
   void setHeaderField(const QByteArray &name, const QByteArray &data) override;

   void parseHeader(const QByteArray &header); // mainly for testing

   QHttpNetworkRequest request() const;
   void setRequest(const QHttpNetworkRequest &request);

   int statusCode() const;
   void setStatusCode(int code);

   QString errorString() const;
   void setErrorString(const QString &error);

   QString reasonPhrase() const;

   qint64 bytesAvailable() const;
   qint64 bytesAvailableNextBlock() const;
   bool readAnyAvailable() const;
   QByteArray readAny();
   QByteArray readAll();
   QByteArray read(qint64 amount);
   qint64 sizeNextBlock();
   void setDownstreamLimited(bool t);
   void setReadBufferSize(qint64 size);

   bool supportsUserProvidedDownloadBuffer();
   void setUserProvidedDownloadBuffer(char *);
   char *userProvidedDownloadBuffer();

   void abort();
   bool isAborted() const;
   bool isFinished() const;

   bool isPipeliningUsed() const;
   bool isSpdyUsed() const;
   void setSpdyWasUsed(bool spdy);

   bool isRedirecting() const;
   QHttpNetworkConnection *connection();

   QUrl redirectUrl() const;
   void setRedirectUrl(const QUrl &url);
   static bool isHttpRedirect(int statusCode);

#ifdef QT_SSL
   QSslConfiguration sslConfiguration() const;
   void setSslConfiguration(const QSslConfiguration &config);
   void ignoreSslErrors();
   void ignoreSslErrors(const QList<QSslError> &errors);

   NET_CS_SIGNAL_1(Public, void encrypted())
   NET_CS_SIGNAL_2(encrypted)

   NET_CS_SIGNAL_1(Public, void preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator))
   NET_CS_SIGNAL_2(preSharedKeyAuthenticationRequired, authenticator)

   NET_CS_SIGNAL_1(Public, void sslErrors(const QList <QSslError> &errors))
   NET_CS_SIGNAL_2(sslErrors, errors)
#endif

   NET_CS_SIGNAL_1(Public, void readyRead())
   NET_CS_SIGNAL_2(readyRead)

   NET_CS_SIGNAL_1(Public, void finished())
   NET_CS_SIGNAL_2(finished)

   NET_CS_SIGNAL_1(Public, void finishedWithError(QNetworkReply::NetworkError errorCode, const QString &detail = QString()))
   NET_CS_SIGNAL_2(finishedWithError, errorCode, detail)

   NET_CS_SIGNAL_1(Public, void headerChanged())
   NET_CS_SIGNAL_2(headerChanged)

   NET_CS_SIGNAL_1(Public, void dataReadProgress(qint64 done, qint64 total))
   NET_CS_SIGNAL_2(dataReadProgress, done, total)

   NET_CS_SIGNAL_1(Public, void dataSendProgress(qint64 done, qint64 total))
   NET_CS_SIGNAL_2(dataSendProgress, done, total)

   NET_CS_SIGNAL_1(Public, void cacheCredentials(const QHttpNetworkRequest &request, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(cacheCredentials, request, authenticator)

#ifndef QT_NO_NETWORKPROXY
   NET_CS_SIGNAL_1(Public, void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(proxyAuthenticationRequired, proxy, authenticator)
#endif

   NET_CS_SIGNAL_1(Public, void authenticationRequired(QHttpNetworkRequest request, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(authenticationRequired, request, authenticator)

   NET_CS_SIGNAL_1(Public, void redirected(QUrl url, int httpStatus, int maxRedirectsRemaining))
   NET_CS_SIGNAL_2(redirected, url, httpStatus, maxRedirectsRemaining)

 private:
   Q_DECLARE_PRIVATE(QHttpNetworkReply)

   friend class QHttpSocketEngine;
   friend class QHttpNetworkConnection;
   friend class QHttpNetworkConnectionPrivate;
   friend class QHttpNetworkConnectionChannel;
   friend class QHttpProtocolHandler;
   friend class QSpdyProtocolHandler;

 protected:
   QScopedPointer<QHttpNetworkReplyPrivate> d_ptr;
};

class QHttpNetworkReplyPrivate : public QHttpNetworkHeaderPrivate
{

 public:
   QHttpNetworkReplyPrivate(const QUrl &newUrl = QUrl());
   virtual ~QHttpNetworkReplyPrivate();

   qint64 readStatus(QAbstractSocket *socket);
   bool parseStatus(const QByteArray &status);
   qint64 readHeader(QAbstractSocket *socket);
   void parseHeader(const QByteArray &header);
   qint64 readBody(QAbstractSocket *socket, QByteDataBuffer *out);
   qint64 readBodyVeryFast(QAbstractSocket *socket, char *b);
   qint64 readBodyFast(QAbstractSocket *socket, QByteDataBuffer *rb);
   bool findChallenge(bool forProxy, QByteArray &challenge) const;
   QAuthenticatorPrivate::Method authenticationMethod(bool isProxy) const;
   void clear();
   void clearHttpLayerInformation();

   qint64 readReplyBodyRaw(QAbstractSocket *in, QByteDataBuffer *out, qint64 size);
   qint64 readReplyBodyChunked(QAbstractSocket *in, QByteDataBuffer *out);
   qint64 getChunkSize(QAbstractSocket *in, qint64 *chunkSize);

   bool isRedirecting() const;
   bool shouldEmitSignals();
   bool expectContent();
   void eraseData();

   qint64 bytesAvailable() const;
   bool isChunked();
   bool isConnectionCloseEnabled();
   bool isCompressed();

   void removeAutoDecompressHeader();

   enum ReplyState {
      NothingDoneState,
      ReadingStatusState,
      ReadingHeaderState,
      ReadingDataState,
      AllDoneState,
      SPDYSYNSent,
      SPDYUploading,
      SPDYHalfClosed,
      SPDYClosed,
      Aborted
   } state;

   QHttpNetworkRequest request;
   bool ssl;
   int statusCode;
   int majorVersion;
   int minorVersion;
   QString errorString;
   QString reasonPhrase;
   qint64 bodyLength;
   qint64 contentRead;
   qint64 totalProgress;
   QByteArray fragment; // used for header, status, chunk header etc, not for reply data
   bool chunkedTransferEncoding;
   bool connectionCloseEnabled;
   bool forceConnectionCloseEnabled;
   bool lastChunkRead;

   qint64 currentChunkSize;
   qint64 currentChunkRead;
   qint64 readBufferMaxSize;
   qint32 windowSizeDownload; // only for SPDY
   qint32 windowSizeUpload; // only for SPDY
   qint32 currentlyReceivedDataInWindow; // only for SPDY
   qint32 currentlyUploadedDataInWindow; // only for SPDY
   qint64 totallyUploadedData; // only for SPDY

   QPointer<QHttpNetworkConnection> connection;
   QPointer<QHttpNetworkConnectionChannel> connectionChannel;
   bool autoDecompress;

   QByteDataBuffer responseData; // uncompressed body
   QByteArray compressedData; // compressed body (temporary)
   bool requestIsPrepared;

   bool pipeliningUsed;
    bool spdyUsed;
   bool downstreamLimited;

   char *userProvidedDownloadBuffer;
   QUrl redirectUrl;
#ifndef QT_NO_COMPRESS
    z_stream_s *inflateStrm;
    int initializeInflateStream();
    qint64 uncompressBodyData(QByteDataBuffer *in, QByteDataBuffer *out);
#endif
};



#endif // QHTTPNETWORKREPLY_H
