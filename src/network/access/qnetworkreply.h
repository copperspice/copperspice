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

#ifndef QNETWORKREPLY_H
#define QNETWORKREPLY_H

#include <QtCore/QIODevice>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>

QT_BEGIN_NAMESPACE

class QUrl;
class QVariant;
class QAuthenticator;
class QSslConfiguration;
class QSslError;
class QNetworkReplyPrivate;

class Q_NETWORK_EXPORT QNetworkReply: public QIODevice
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
      UnknownContentError = 299,

      // protocol errors
      ProtocolUnknownError = 301,
      ProtocolInvalidOperationError,
      ProtocolFailure = 399
   };

   ~QNetworkReply();
   virtual void abort() = 0;

   // reimplemented from QIODevice
   void close() override;
   bool isSequential() const override;

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

#ifndef QT_NO_OPENSSL
   QSslConfiguration sslConfiguration() const;
   void setSslConfiguration(const QSslConfiguration &configuration);
   void ignoreSslErrors(const QList<QSslError> &errors);
#endif

   virtual QSslConfiguration sslConfigurationImplementation() const;
   virtual void setSslConfigurationImplementation(const QSslConfiguration &configuration);
   virtual void ignoreSslErrorsImplementation(const QList<QSslError> &errors);

   NET_CS_SLOT_1(Public, virtual void ignoreSslErrors())
   NET_CS_SLOT_OVERLOAD(ignoreSslErrors, ())

   NET_CS_SIGNAL_1(Public, void metaDataChanged())
   NET_CS_SIGNAL_2(metaDataChanged)

   NET_CS_SIGNAL_1(Public, void finished())
   NET_CS_SIGNAL_2(finished)

   NET_CS_SIGNAL_1(Public, void error(QNetworkReply::NetworkError un_named_arg1))
   NET_CS_SIGNAL_OVERLOAD(error, (QNetworkReply::NetworkError), un_named_arg1)

#ifndef QT_NO_OPENSSL
   NET_CS_SIGNAL_1(Public, void sslErrors(const QList <QSslError> &errors))
   NET_CS_SIGNAL_2(sslErrors, errors)
#endif

   NET_CS_SIGNAL_1(Public, void uploadProgress(qint64 bytesSent, qint64 bytesTotal))
   NET_CS_SIGNAL_2(uploadProgress, bytesSent, bytesTotal)

   NET_CS_SIGNAL_1(Public, void downloadProgress(qint64 bytesReceived, qint64 bytesTotal))
   NET_CS_SIGNAL_2(downloadProgress, bytesReceived, bytesTotal)

 protected:
   QNetworkReply(QObject *parent = nullptr);
   QNetworkReply(QNetworkReplyPrivate &dd, QObject *parent);
   qint64 writeData(const char *data, qint64 len) override;

   void setOperation(QNetworkAccessManager::Operation operation);
   void setRequest(const QNetworkRequest &request);
   void setError(NetworkError errorCode, const QString &errorString);
   void setFinished(bool);
   void setUrl(const QUrl &url);
   void setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value);
   void setRawHeader(const QByteArray &headerName, const QByteArray &value);
   void setAttribute(QNetworkRequest::Attribute code, const QVariant &value);

 private:
   Q_DECLARE_PRIVATE(QNetworkReply)
};

QT_END_NAMESPACE

#endif
