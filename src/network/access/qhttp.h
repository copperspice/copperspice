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

#ifndef QHTTP_H
#define QHTTP_H

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <qabstractsocket.h>
#include <QScopedPointer>
#include <qsslerror.h>

#ifndef QT_NO_HTTP

class QTcpSocket;
class QTimerEvent;
class QIODevice;
class QAuthenticator;
class QNetworkProxy;
class QSslError;
class QHttpPrivate;
class QHttpHeaderPrivate;
class QHttpResponseHeaderPrivate;
class QHttpRequestHeaderPrivate;

class Q_NETWORK_EXPORT QHttpHeader
{

 public:
   QHttpHeader();
   QHttpHeader(const QHttpHeader &header);
   QHttpHeader(const QString &str);
   virtual ~QHttpHeader();

   QHttpHeader &operator=(const QHttpHeader &h);

   void setValue(const QString &key, const QString &value);
   void setValues(const QList<QPair<QString, QString> > &values);
   void addValue(const QString &key, const QString &value);
   QList<QPair<QString, QString> > values() const;
   bool hasKey(const QString &key) const;
   QStringList keys() const;
   QString value(const QString &key) const;
   QStringList allValues(const QString &key) const;
   void removeValue(const QString &key);
   void removeAllValues(const QString &key);

   bool hasContentLength() const;
   qint64 contentLength() const;
   void setContentLength(qint64 len);

   bool hasContentType() const;
   QString contentType() const;
   void setContentType(const QString &type);

   virtual QString toString() const;
   bool isValid() const;

   virtual int majorVersion() const = 0;
   virtual int minorVersion() const = 0;

 protected:
   virtual bool parseLine(const QString &line, int number);
   bool parse(const QString &str);
   void setValid(bool);

   QHttpHeader(QHttpHeaderPrivate &dd, const QString &str = QString());
   QHttpHeader(QHttpHeaderPrivate &dd, const QHttpHeader &header);
   QScopedPointer<QHttpHeaderPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QHttpHeader)
};

class Q_NETWORK_EXPORT QHttpResponseHeader : public QHttpHeader
{

 public:
   QHttpResponseHeader();
   QHttpResponseHeader(const QHttpResponseHeader &header);
   QHttpResponseHeader(const QString &str);
   QHttpResponseHeader(int code, const QString &text = QString(), int majorVer = 1, int minorVer = 1);
   QHttpResponseHeader &operator=(const QHttpResponseHeader &header);

   void setStatusLine(int code, const QString &text = QString(), int majorVer = 1, int minorVer = 1);

   int statusCode() const;
   QString reasonPhrase() const;

   int majorVersion() const override;
   int minorVersion() const override;

   QString toString() const override;

 protected:
   bool parseLine(const QString &line, int number) override;

 private:
   Q_DECLARE_PRIVATE(QHttpResponseHeader)
   friend class QHttpPrivate;
};

class Q_NETWORK_EXPORT QHttpRequestHeader : public QHttpHeader
{
 public:
   QHttpRequestHeader();
   QHttpRequestHeader(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);
   QHttpRequestHeader(const QHttpRequestHeader &header);
   QHttpRequestHeader(const QString &str);
   QHttpRequestHeader &operator=(const QHttpRequestHeader &header);

   void setRequest(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);

   QString method() const;
   QString path() const;

   int majorVersion() const override;
   int minorVersion() const override;

   QString toString() const override;

 protected:
   bool parseLine(const QString &line, int number) override;

 private:
   Q_DECLARE_PRIVATE(QHttpRequestHeader)
};

class Q_NETWORK_EXPORT QHttp : public QObject
{
   NET_CS_OBJECT(QHttp)

 public:
   enum ConnectionMode {
      ConnectionModeHttp,
      ConnectionModeHttps
   };

   explicit QHttp(QObject *parent = nullptr);
   QHttp(const QString &hostname, quint16 port = 80, QObject *parent = nullptr);
   QHttp(const QString &hostname, ConnectionMode mode, quint16 port = 0, QObject *parent = nullptr);
   virtual ~QHttp();

   enum State {
      Unconnected,
      HostLookup,
      Connecting,
      Sending,
      Reading,
      Connected,
      Closing
   };
   enum Error {
      NoError,
      UnknownError,
      HostNotFound,
      ConnectionRefused,
      UnexpectedClose,
      InvalidResponseHeader,
      WrongContentLength,
      Aborted,
      AuthenticationRequiredError,
      ProxyAuthenticationRequiredError
   };

   int setHost(const QString &hostname, quint16 port = 80);
   int setHost(const QString &hostname, ConnectionMode mode, quint16 port = 0);

   int setSocket(QTcpSocket *socket);
   int setUser(const QString &username, const QString &password = QString());

#ifndef QT_NO_NETWORKPROXY
   int setProxy(const QString &host, int port,
                const QString &username = QString(),
                const QString &password = QString());
   int setProxy(const QNetworkProxy &proxy);
#endif

   int get(const QString &path, QIODevice *to = 0);
   int post(const QString &path, QIODevice *data, QIODevice *to = 0 );
   int post(const QString &path, const QByteArray &data, QIODevice *to = 0);
   int head(const QString &path);
   int request(const QHttpRequestHeader &header, QIODevice *device = 0, QIODevice *to = 0);
   int request(const QHttpRequestHeader &header, const QByteArray &data, QIODevice *to = 0);

   int closeConnection();
   int close();

   qint64 bytesAvailable() const;
   qint64 read(char *data, qint64 maxlen);
   QByteArray readAll();

   int currentId() const;
   QIODevice *currentSourceDevice() const;
   QIODevice *currentDestinationDevice() const;
   QHttpRequestHeader currentRequest() const;
   QHttpResponseHeader lastResponse() const;
   bool hasPendingRequests() const;
   void clearPendingRequests();

   State state() const;

   Error error() const;
   QString errorString() const;

   NET_CS_SLOT_1(Public, void abort())
   NET_CS_SLOT_2(abort)

#ifndef QT_NO_OPENSSL
   NET_CS_SLOT_1(Public, void ignoreSslErrors())
   NET_CS_SLOT_2(ignoreSslErrors)
#endif

   NET_CS_SIGNAL_1(Public, void stateChanged(int state))
   NET_CS_SIGNAL_2(stateChanged, state)

   NET_CS_SIGNAL_1(Public, void responseHeaderReceived(const QHttpResponseHeader &resp))
   NET_CS_SIGNAL_2(responseHeaderReceived, resp)

   NET_CS_SIGNAL_1(Public, void readyRead(const QHttpResponseHeader &resp))
   NET_CS_SIGNAL_2(readyRead, resp)

   //
   NET_CS_SIGNAL_1(Public, void dataSendProgress(qint64 done, qint64 total))
   NET_CS_SIGNAL_2(dataSendProgress, done, total)

   NET_CS_SIGNAL_1(Public, void dataReadProgress(qint64 done, qint64 total))
   NET_CS_SIGNAL_2(dataReadProgress, done, total)

   //
   NET_CS_SIGNAL_1(Public, void requestStarted(int id))
   NET_CS_SIGNAL_2(requestStarted, id)

   NET_CS_SIGNAL_1(Public, void requestFinished(int id, bool error))
   NET_CS_SIGNAL_2(requestFinished, id, error)

   NET_CS_SIGNAL_1(Public, void done(bool error))
   NET_CS_SIGNAL_2(done, error)

#ifndef QT_NO_NETWORKPROXY
   NET_CS_SIGNAL_1(Public, void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(proxyAuthenticationRequired, proxy, authenticator)
#endif

   NET_CS_SIGNAL_1(Public, void authenticationRequired(const QString &hostname, quint16 port, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(authenticationRequired, hostname, port, authenticator)

#ifndef QT_NO_OPENSSL
   NET_CS_SIGNAL_1(Public, void sslErrors(const QList <QSslError> &errors))
   NET_CS_SIGNAL_2(sslErrors, errors)
#endif

 private:
   Q_DISABLE_COPY(QHttp)
   Q_DECLARE_PRIVATE(QHttp)

   NET_CS_SLOT_1(Private, void _q_startNextRequest())
   NET_CS_SLOT_2(_q_startNextRequest)

   NET_CS_SLOT_1(Private, void _q_slotReadyRead())
   NET_CS_SLOT_2(_q_slotReadyRead)

   NET_CS_SLOT_1(Private, void _q_slotConnected())
   NET_CS_SLOT_2(_q_slotConnected)

   NET_CS_SLOT_1(Private, void _q_slotError(QAbstractSocket::SocketError error))
   NET_CS_SLOT_2(_q_slotError)

   NET_CS_SLOT_1(Private, void _q_slotClosed())
   NET_CS_SLOT_2(_q_slotClosed)

   NET_CS_SLOT_1(Private, void _q_slotBytesWritten(qint64 numBytes))
   NET_CS_SLOT_2(_q_slotBytesWritten)

#ifndef QT_NO_OPENSSL
   NET_CS_SLOT_1(Private, void _q_slotEncryptedBytesWritten(qint64 numBytes))
   NET_CS_SLOT_2(_q_slotEncryptedBytesWritten)
#endif

   NET_CS_SLOT_1(Private, void _q_slotDoFinished())
   NET_CS_SLOT_2(_q_slotDoFinished)

   NET_CS_SLOT_1(Private, void _q_slotSendRequest())
   NET_CS_SLOT_2(_q_slotSendRequest)

   NET_CS_SLOT_1(Private, void _q_continuePost())
   NET_CS_SLOT_2(_q_continuePost)

   friend class QHttpNormalRequest;
   friend class QHttpSetHostRequest;
   friend class QHttpSetSocketRequest;
   friend class QHttpSetUserRequest;
   friend class QHttpSetProxyRequest;
   friend class QHttpCloseRequest;
   friend class QHttpPGHRequest;

 protected:
   QScopedPointer<QHttpPrivate> d_ptr;

};

#endif // QT_NO_HTTP

#endif // QHTTP_H
