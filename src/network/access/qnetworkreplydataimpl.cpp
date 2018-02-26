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

#include <qnetworkreplydataimpl_p.h>
#include <qdataurl_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>

QT_BEGIN_NAMESPACE

QNetworkReplyDataImplPrivate::QNetworkReplyDataImplPrivate()
   : QNetworkReplyPrivate()
{
}

QNetworkReplyDataImplPrivate::~QNetworkReplyDataImplPrivate()
{
}

QNetworkReplyDataImpl::~QNetworkReplyDataImpl()
{
}

QNetworkReplyDataImpl::QNetworkReplyDataImpl(QObject *parent, const QNetworkRequest &req, const QNetworkAccessManager::Operation op)
   : QNetworkReply(*new QNetworkReplyDataImplPrivate(), parent)
{
   Q_D(QNetworkReplyDataImpl);

   setRequest(req);
   setUrl(req.url());
   setOperation(op);
   setFinished(true);

   QNetworkReply::open(QIODevice::ReadOnly);

   QUrl url = req.url();

   QString mimeType;
   QByteArray payload;
   bool isValid = false;

   if (url.scheme().compare("data", Qt::CaseInsensitive) == 0 && url.host().isEmpty() ) {
      QPair<QString, QByteArray> retval = qDecodeDataUrl(url);

      mimeType = retval.first;
      payload  = retval.second;
      isValid  = true;
   }

   if (isValid) {
      qint64 size = payload.size();

      setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
      setHeader(QNetworkRequest::ContentLengthHeader, size);

      QMetaObject::invokeMethod(this, "metaDataChanged", Qt::QueuedConnection);

      d->decodedData.setData(payload);
      d->decodedData.open(QIODevice::ReadOnly);

      QMetaObject::invokeMethod(this, "downloadProgress", Qt::QueuedConnection, Q_ARG(qint64, size), Q_ARG(qint64, size));
      QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);
      QMetaObject::invokeMethod(this, "finished",  Qt::QueuedConnection);

   } else {
      // something is wrong with this URL
      const QString msg = QCoreApplication::translate("QNetworkAccessDataBackend", "Invalid URI: %1").arg(url.toString());

      setError(QNetworkReply::ProtocolFailure, msg);
      QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                Q_ARG(QNetworkReply::NetworkError, QNetworkReply::ProtocolFailure));

      QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
   }
}

void QNetworkReplyDataImpl::close()
{
   QNetworkReply::close();
}

void QNetworkReplyDataImpl::abort()
{
   QNetworkReply::close();
}

qint64 QNetworkReplyDataImpl::bytesAvailable() const
{
   Q_D(const QNetworkReplyDataImpl);
   return QNetworkReply::bytesAvailable() + d->decodedData.bytesAvailable();
}

bool QNetworkReplyDataImpl::isSequential () const
{
   return true;
}

qint64 QNetworkReplyDataImpl::size() const
{
   Q_D(const QNetworkReplyDataImpl);
   return d->decodedData.size();
}

/*!
    \internal
*/
qint64 QNetworkReplyDataImpl::readData(char *data, qint64 maxlen)
{
   Q_D(QNetworkReplyDataImpl);

   // TODO idea:
   // Instead of decoding the whole data into new memory, we could decode on demand.
   // Note that this might be tricky to do.

   return d->decodedData.read(data, maxlen);
}



