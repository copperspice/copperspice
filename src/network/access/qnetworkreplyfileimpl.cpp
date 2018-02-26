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

#include <qnetworkreplyfileimpl_p.h>
#include <qdatetime.h>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>


QNetworkReplyFileImplPrivate::QNetworkReplyFileImplPrivate()
   : QNetworkReplyPrivate(), realFileSize(0)
{
}

QNetworkReplyFileImpl::~QNetworkReplyFileImpl()
{
}

QNetworkReplyFileImpl::QNetworkReplyFileImpl(QObject *parent, const QNetworkRequest &req,
      const QNetworkAccessManager::Operation op)
   : QNetworkReply(*new QNetworkReplyFileImplPrivate(), parent)
{
   setRequest(req);
   setUrl(req.url());
   setOperation(op);
   setFinished(true);
   QNetworkReply::open(QIODevice::ReadOnly);

   QNetworkReplyFileImplPrivate *d = (QNetworkReplyFileImplPrivate *) d_func();

   QUrl url = req.url();
   if (url.host() == "localhost") {
      url.setHost(QString());
   }

#if !defined(Q_OS_WIN)
   // do not allow UNC paths on Unix
   if (!url.host().isEmpty()) {
      // we handle only local files
      QString msg = QCoreApplication::translate("QNetworkAccessFileBackend",
                    "Request for opening non-local file %1").arg(url.toString());

      setError(QNetworkReply::ProtocolInvalidOperationError, msg);
      QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                Q_ARG(QNetworkReply::NetworkError, QNetworkReply::ProtocolInvalidOperationError));
      QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
      return;
   }
#endif

   if (url.path().isEmpty()) {
      url.setPath(QLatin1String("/"));
   }
   setUrl(url);


   QString fileName = url.toLocalFile();
   if (fileName.isEmpty()) {
      if (url.scheme() == QLatin1String("qrc")) {
         fileName = QLatin1Char(':') + url.path();
      } else {
         fileName = url.toString(QUrl::RemoveAuthority | QUrl::RemoveFragment | QUrl::RemoveQuery);
      }
   }

   QFileInfo fi(fileName);
   if (fi.isDir()) {
      QString msg = QCoreApplication::translate("QNetworkAccessFileBackend",
                    "Can not open %1: Path is a directory").arg(url.toString());

      setError(QNetworkReply::ContentOperationNotPermittedError, msg);
      QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                Q_ARG(QNetworkReply::NetworkError, QNetworkReply::ContentOperationNotPermittedError));
      QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
      return;
   }

   d->realFile.setFileName(fileName);
   bool opened = d->realFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered);

   // could we open the file?
   if (!opened) {
      QString msg = QCoreApplication::translate("QNetworkAccessFileBackend", "Error opening %1: %2")
                    .arg(d->realFile.fileName(), d->realFile.errorString());

      if (d->realFile.exists()) {
         setError(QNetworkReply::ContentAccessDenied, msg);
         QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                   Q_ARG(QNetworkReply::NetworkError, QNetworkReply::ContentAccessDenied));
      } else {
         setError(QNetworkReply::ContentNotFoundError, msg);
         QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                   Q_ARG(QNetworkReply::NetworkError, QNetworkReply::ContentNotFoundError));
      }
      QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
      return;
   }

   setHeader(QNetworkRequest::LastModifiedHeader, fi.lastModified());
   d->realFileSize = fi.size();
   setHeader(QNetworkRequest::ContentLengthHeader, d->realFileSize);

   QMetaObject::invokeMethod(this, "metaDataChanged", Qt::QueuedConnection);

   QMetaObject::invokeMethod(this, "downloadProgress", Qt::QueuedConnection,
                             Q_ARG(qint64, d->realFileSize), Q_ARG(qint64, d->realFileSize));

   QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);
   QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
}
void QNetworkReplyFileImpl::close()
{
   Q_D(QNetworkReplyFileImpl);
   QNetworkReply::close();
   d->realFile.close();
}

void QNetworkReplyFileImpl::abort()
{
   Q_D(QNetworkReplyFileImpl);
   QNetworkReply::close();
   d->realFile.close();
}

qint64 QNetworkReplyFileImpl::bytesAvailable() const
{
   Q_D(const QNetworkReplyFileImpl);
   if (!d->realFile.isOpen()) {
      return QNetworkReply::bytesAvailable();
   }
   return QNetworkReply::bytesAvailable() + d->realFile.bytesAvailable();
}

bool QNetworkReplyFileImpl::isSequential () const
{
   return true;
}

qint64 QNetworkReplyFileImpl::size() const
{
   Q_D(const QNetworkReplyFileImpl);
   return d->realFileSize;
}

/*!
    \internal
*/
qint64 QNetworkReplyFileImpl::readData(char *data, qint64 maxlen)
{
   Q_D(QNetworkReplyFileImpl);
   if (! d->realFile.isOpen()) {
      return -1;
   }

   qint64 ret = d->realFile.read(data, maxlen);
   if (bytesAvailable() == 0 && d->realFile.isOpen()) {
      d->realFile.close();
   }

   if (ret == 0 && bytesAvailable() == 0) {
      return -1;

   } else {
      setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
      setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QLatin1String("OK"));
      return ret;
   }
}


