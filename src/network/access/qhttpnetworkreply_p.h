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

#ifndef QHTTPNETWORKREPLY_P_H
#define QHTTPNETWORKREPLY_P_H

#include <qplatformdefs.h>

#ifndef QT_NO_HTTP

#ifndef QT_NO_COMPRESS
#include <zlib.h>
static const unsigned char gz_magic[2] = {0x1f, 0x8b}; // gzip magic header

// gzip flag byte
#define HEAD_CRC     0x02 // bit 1 set: header CRC present
#define EXTRA_FIELD  0x04 // bit 2 set: extra field present
#define ORIG_NAME    0x08 // bit 3 set: original file name present
#define COMMENT      0x10 // bit 4 set: file comment present
#define RESERVED     0xE0 // bits 5..7: reserved
#define CHUNK 16384
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

QT_BEGIN_NAMESPACE

class QHttpNetworkConnection;
class QHttpNetworkConnectionChannel;
class QHttpNetworkRequest;
class QHttpNetworkConnectionPrivate;
class QHttpNetworkReplyPrivate;

class QHttpNetworkReply : public QObject, public QHttpNetworkHeader
{
   NET_CS_OBJECT_MULTIPLE(QHttpNetworkReply, QObject)

 public:
   explicit QHttpNetworkReply(const QUrl &url = QUrl(), QObject *parent = 0);
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

   bool isFinished() const;

   bool isPipeliningUsed() const;

   QHttpNetworkConnection *connection();

#ifndef QT_NO_OPENSSL
   QSslConfiguration sslConfiguration() const;
   void setSslConfiguration(const QSslConfiguration &config);
   void ignoreSslErrors();
   void ignoreSslErrors(const QList<QSslError> &errors);

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

   // FIXME we need to change this to qint64!
   NET_CS_SIGNAL_1(Public, void dataReadProgress(int done, int total))
   NET_CS_SIGNAL_2(dataReadProgress, done, total)
   NET_CS_SIGNAL_1(Public, void dataSendProgress(qint64 done, qint64 total))
   NET_CS_SIGNAL_2(dataSendProgress, done, total)
   NET_CS_SIGNAL_1(Public, void cacheCredentials(const QHttpNetworkRequest &request, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(cacheCredentials, request, authenticator)

#ifndef QT_NO_NETWORKPROXY
   NET_CS_SIGNAL_1(Public, void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(proxyAuthenticationRequired, proxy, authenticator)
#endif

   NET_CS_SIGNAL_1(Public, void authenticationRequired(const QHttpNetworkRequest &request, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(authenticationRequired, request, authenticator)

 private:
   Q_DECLARE_PRIVATE(QHttpNetworkReply)

   friend class QHttpNetworkConnection;
   friend class QHttpNetworkConnectionPrivate;
   friend class QHttpNetworkConnectionChannel;

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

   void appendUncompressedReplyData(QByteArray &qba);
   void appendUncompressedReplyData(QByteDataBuffer &data);
   void appendCompressedReplyData(QByteDataBuffer &data);

   bool shouldEmitSignals();
   bool expectContent();
   void eraseData();

   qint64 bytesAvailable() const;
   bool isChunked();
   bool isConnectionCloseEnabled();
   bool isGzipped();

#ifndef QT_NO_COMPRESS
   bool gzipCheckHeader(QByteArray &content, int &pos);
   int gunzipBodyPartially(QByteArray &compressed, QByteArray &inflated);
   void gunzipBodyPartiallyEnd();
#endif

   void removeAutoDecompressHeader();

   enum ReplyState {
      NothingDoneState,
      ReadingStatusState,
      ReadingHeaderState,
      ReadingDataState,
      AllDoneState
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
   QPointer<QHttpNetworkConnection> connection;
   QPointer<QHttpNetworkConnectionChannel> connectionChannel;
   bool initInflate;
   bool streamEnd;

#ifndef QT_NO_COMPRESS
   z_stream inflateStrm;
#endif

   bool autoDecompress;

   QByteDataBuffer responseData; // uncompressed body
   QByteArray compressedData; // compressed body (temporary)
   bool requestIsPrepared;

   bool pipeliningUsed;
   bool downstreamLimited;

   char *userProvidedDownloadBuffer;
};

QT_END_NAMESPACE

//Q_DECLARE_METATYPE(QHttpNetworkReply)

#endif // QT_NO_HTTP

#endif // QHTTPNETWORKREPLY_H
