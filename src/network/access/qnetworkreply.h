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

#ifndef QNETWORKREPLY_H
#define QNETWORKREPLY_H

#include <QIODevice>
#include <QString>
#include <QVariant>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

class QUrl;
class QVariant;
class QAuthenticator;
class QSslConfiguration;
class QSslError;
class QNetworkReplyPrivate;

class Q_NETWORK_EXPORT QNetworkReply : public QIODevice
{
   NET_CS_OBJECT(QNetworkReply)

   NET_CS_ENUM(NetworkError)

 public:
   enum NetworkError {
      NoError = 0,

      // network layer errors [relating to the destination server] (1-99):
      ConnectionRefusedError = 1,
      RemoteHostClosedError,
      HostNotFoundError,
      TimeoutError,
      OperationCanceledError,
      SslHandshakeFailedError,
      TemporaryNetworkFailureError,
      NetworkSessionFailedError,
      BackgroundRequestNotAllowedError,
      TooManyRedirectsError,
      InsecureRedirectError,
      UnknownNetworkError = 99,

      // proxy errors (101-199):
      ProxyConnectionRefusedError = 101,
      ProxyConnectionClosedError,
      ProxyNotFoundError,
      ProxyTimeoutError,
      ProxyAuthenticationRequiredError,
      UnknownProxyError = 199,

      // content errors (201-299):
      ContentAccessDenied = 201,
      ContentOperationNotPermittedError,
      ContentNotFoundError,
      AuthenticationRequiredError,
      ContentReSendError,
      ContentConflictError,
      ContentGoneError,
      UnknownContentError = 299,

      // protocol errors
      ProtocolUnknownError = 301,
      ProtocolInvalidOperationError,
      ProtocolFailure = 399,
      InternalServerError = 401,
      OperationNotImplementedError,
      ServiceUnavailableError,
      UnknownServerError = 499
   };

   ~QNetworkReply();

   // reimplemented from QIODevice
   virtual void close() override;
   virtual bool isSequential() const override;

   // like QAbstractSocket:
   qint64 readBufferSize() const;
   virtual void setReadBufferSize(qint64 size);

   QNetworkAccessManager *manager() const;
   QNetworkAccessManager::Operation operation() const;
   QNetworkRequest request() const;
   NetworkError error() const;
   bool isFinished() const;
   bool isRunning() const;
   QUrl url() const;

   // "cooked" headers
   QVariant header(QNetworkRequest::KnownHeaders header) const;

   // raw headers:
   bool hasRawHeader(const QByteArray &headerName) const;
   QList<QByteArray> rawHeaderList() const;
   QByteArray rawHeader(const QByteArray &headerName) const;

   typedef QPair<QByteArray, QByteArray> RawHeaderPair;
   const QList<RawHeaderPair> &rawHeaderPairs() const;

   // attributes
   QVariant attribute(QNetworkRequest::Attribute code) const;

#ifdef QT_SSL
   QSslConfiguration sslConfiguration() const;
   void setSslConfiguration(const QSslConfiguration &configuration);
   void ignoreSslErrors(const QList<QSslError> &errors);
#endif

   NET_CS_SLOT_1(Public, virtual void abort() = 0)
   NET_CS_SLOT_2(abort)

   NET_CS_SLOT_1(Public, virtual void ignoreSslErrors())
   NET_CS_SLOT_OVERLOAD(ignoreSslErrors, ())

   NET_CS_SIGNAL_1(Public, void metaDataChanged())
   NET_CS_SIGNAL_2(metaDataChanged)

   NET_CS_SIGNAL_1(Public, void finished())
   NET_CS_SIGNAL_2(finished)

   NET_CS_SIGNAL_1(Public, void error(QNetworkReply::NetworkError un_named_arg1))
   NET_CS_SIGNAL_OVERLOAD(error, (QNetworkReply::NetworkError), un_named_arg1)

#ifdef QT_SSL
   NET_CS_SIGNAL_1(Public, void encrypted())
   NET_CS_SIGNAL_2(encrypted)

   NET_CS_SIGNAL_1(Public, void preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator))
   NET_CS_SIGNAL_2(preSharedKeyAuthenticationRequired, authenticator)

   NET_CS_SIGNAL_1(Public, void sslErrors(const QList <QSslError> &errors))
   NET_CS_SIGNAL_2(sslErrors, errors)
#endif

   NET_CS_SIGNAL_1(Public, void redirected(QUrl url))
   NET_CS_SIGNAL_2(redirected, url)

   NET_CS_SIGNAL_1(Public, void uploadProgress(qint64 bytesSent, qint64 bytesTotal))
   NET_CS_SIGNAL_2(uploadProgress, bytesSent, bytesTotal)

   NET_CS_SIGNAL_1(Public, void downloadProgress(qint64 bytesReceived, qint64 bytesTotal))
   NET_CS_SIGNAL_2(downloadProgress, bytesReceived, bytesTotal)

 protected:
   explicit QNetworkReply(QObject *parent = nullptr);
   QNetworkReply(QNetworkReplyPrivate &dd, QObject *parent);
   virtual qint64 writeData(const char *data, qint64 len) override;

   void setOperation(QNetworkAccessManager::Operation operation);
   void setRequest(const QNetworkRequest &request);
   void setError(NetworkError errorCode, const QString &errorString);
   void setFinished(bool);
   void setUrl(const QUrl &url);
   void setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value);
   void setRawHeader(const QByteArray &headerName, const QByteArray &value);
   void setAttribute(QNetworkRequest::Attribute code, const QVariant &value);

   virtual void sslConfigurationImplementation(QSslConfiguration &) const;
   virtual void setSslConfigurationImplementation(const QSslConfiguration &configuration);
   virtual void ignoreSslErrorsImplementation(const QList<QSslError> &errors);

 private:
   Q_DECLARE_PRIVATE(QNetworkReply)
};

#endif
