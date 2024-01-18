/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qnetwork_reply.h>
#include <qnetwork_reply_p.h>

#include <qsslconfiguration.h>
#include <qsslerror.h>

const int QNetworkReplyPrivate::progressSignalInterval = 100;
QNetworkReplyPrivate::QNetworkReplyPrivate()
   : readBufferMaxSize(0), emitAllUploadProgressSignals(false),
     operation(QNetworkAccessManager::UnknownOperation),
     m_errorCode(QNetworkReply::NoError), isFinished(false)
{
   // set the default attribute values
   attributes.insert(QNetworkRequest::ConnectionEncryptedAttribute, false);
}

QNetworkReply::QNetworkReply(QObject *parent)
   : QIODevice(*new QNetworkReplyPrivate, parent)
{
}

// internal
QNetworkReply::QNetworkReply(QNetworkReplyPrivate &dd, QObject *parent)
   : QIODevice(dd, parent)
{
}

QNetworkReply::~QNetworkReply()
{
}

void QNetworkReply::close()
{
   QIODevice::close();
}

// internal
bool QNetworkReply::isSequential() const
{
   return true;
}

qint64 QNetworkReply::readBufferSize() const
{
   return d_func()->readBufferMaxSize;
}

void QNetworkReply::setReadBufferSize(qint64 size)
{
   Q_D(QNetworkReply);
   d->readBufferMaxSize = size;
}

QNetworkAccessManager *QNetworkReply::manager() const
{
   return d_func()->manager;
}

QNetworkRequest QNetworkReply::request() const
{
   return d_func()->originalRequest;
}

QNetworkAccessManager::Operation QNetworkReply::operation() const
{
   return d_func()->operation;
}

QNetworkReply::NetworkError QNetworkReply::error() const
{
   return d_func()->m_errorCode;
}

bool QNetworkReply::isFinished() const
{
   return d_func()->isFinished;
}

bool QNetworkReply::isRunning() const
{
   return !isFinished();
}

QUrl QNetworkReply::url() const
{
   return d_func()->url;
}

QVariant QNetworkReply::header(QNetworkRequest::KnownHeaders header) const
{
   return d_func()->cookedHeaders.value(header);
}

bool QNetworkReply::hasRawHeader(const QByteArray &headerName) const
{
   Q_D(const QNetworkReply);
   return d->findRawHeader(headerName) != d->rawHeaders.constEnd();
}

QByteArray QNetworkReply::rawHeader(const QByteArray &headerName) const
{
   Q_D(const QNetworkReply);
   QNetworkHeadersPrivate::RawHeadersList::const_iterator it = d->findRawHeader(headerName);

   if (it != d->rawHeaders.constEnd()) {
      return it->second;
   }
   return QByteArray();
}

const QList<QNetworkReply::RawHeaderPair> &QNetworkReply::rawHeaderPairs() const
{
   Q_D(const QNetworkReply);
   return d->rawHeaders;
}

QList<QByteArray> QNetworkReply::rawHeaderList() const
{
   return d_func()->rawHeadersKeys();
}

QVariant QNetworkReply::attribute(QNetworkRequest::Attribute code) const
{
   return d_func()->attributes.value(code);
}

#ifdef QT_SSL
/*!
    Returns the SSL configuration and state associated with this
    reply, if SSL was used. It will contain the remote server's
    certificate, its certificate chain leading to the Certificate
    Authority as well as the encryption ciphers in use.

    The peer's certificate and its certificate chain will be known by
    the time sslErrors() is emitted, if it's emitted.
*/
QSslConfiguration QNetworkReply::sslConfiguration() const
{
   QSslConfiguration config;
   sslConfigurationImplementation(config);
   return config;
}

void QNetworkReply::setSslConfiguration(const QSslConfiguration &config)
{
   setSslConfigurationImplementation(config);
}

void QNetworkReply::ignoreSslErrors(const QList<QSslError> &errors)
{
   ignoreSslErrorsImplementation(errors);
}
#endif

void QNetworkReply::sslConfigurationImplementation(QSslConfiguration &) const
{

}
void QNetworkReply::setSslConfigurationImplementation(const QSslConfiguration &)
{
}

void QNetworkReply::ignoreSslErrorsImplementation(const QList<QSslError> &errors)
{
   (void) errors;
}

void QNetworkReply::ignoreSslErrors()
{
}

// internal
qint64 QNetworkReply::writeData(const char *, qint64)
{
   return -1;                  // unable to write
}

void QNetworkReply::setOperation(QNetworkAccessManager::Operation operation)
{
   Q_D(QNetworkReply);
   d->operation = operation;
}

void QNetworkReply::setRequest(const QNetworkRequest &request)
{
   Q_D(QNetworkReply);
   d->originalRequest = request;
}

void QNetworkReply::setError(NetworkError errorCode, const QString &errorString)
{
   Q_D(QNetworkReply);
   d->m_errorCode = errorCode;
   setErrorString(errorString); // in QIODevice
}

void QNetworkReply::setFinished(bool finished)
{
   Q_D(QNetworkReply);
   d->isFinished = finished;
}

void QNetworkReply::setUrl(const QUrl &url)
{
   Q_D(QNetworkReply);
   d->url = url;
}

void QNetworkReply::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
   Q_D(QNetworkReply);
   d->setCookedHeader(header, value);
}

void QNetworkReply::setRawHeader(const QByteArray &headerName, const QByteArray &value)
{
   Q_D(QNetworkReply);
   d->setRawHeader(headerName, value);
}

void QNetworkReply::setAttribute(QNetworkRequest::Attribute code, const QVariant &value)
{
   Q_D(QNetworkReply);

   if (value.isValid()) {
      d->attributes.insert(code, value);
   } else {
      d->attributes.remove(code);
   }
}
