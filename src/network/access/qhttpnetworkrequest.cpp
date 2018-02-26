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

#include <qhttpnetworkrequest_p.h>
#include <qnoncontiguousbytedevice_p.h>

QHttpNetworkRequestPrivate::QHttpNetworkRequestPrivate(QHttpNetworkRequest::Operation op,
      QHttpNetworkRequest::Priority pri, const QUrl &newUrl)
   : QHttpNetworkHeaderPrivate(newUrl), operation(op), priority(pri), uploadByteDevice(0),
     autoDecompress(false), pipeliningAllowed(false), spdyAllowed(false),
     withCredentials(true), preConnect(false), followRedirect(false), redirectCount(0)
{
}

QHttpNetworkRequestPrivate::QHttpNetworkRequestPrivate(const QHttpNetworkRequestPrivate &other)
   : QHttpNetworkHeaderPrivate(other)
{
   operation = other.operation;
   priority = other.priority;
   uploadByteDevice = other.uploadByteDevice;
   autoDecompress = other.autoDecompress;
   pipeliningAllowed = other.pipeliningAllowed;
   spdyAllowed = other.spdyAllowed;
   customVerb = other.customVerb;
   withCredentials = other.withCredentials;
   ssl = other.ssl;
   preConnect = other.preConnect;
   followRedirect = other.followRedirect;
   redirectCount = other.redirectCount;
}

QHttpNetworkRequestPrivate::~QHttpNetworkRequestPrivate()
{
}

bool QHttpNetworkRequestPrivate::operator==(const QHttpNetworkRequestPrivate &other) const
{
   return QHttpNetworkHeaderPrivate::operator==(other)
          && (operation == other.operation)
          && (priority == other.priority)
          && (uploadByteDevice == other.uploadByteDevice)
          && (autoDecompress == other.autoDecompress)
          && (pipeliningAllowed == other.pipeliningAllowed)
          && (spdyAllowed == other.spdyAllowed)
          && (operation != QHttpNetworkRequest::Custom || (customVerb == other.customVerb))
          && (withCredentials == other.withCredentials)
          && (ssl == other.ssl)
          && (preConnect == other.preConnect);
}

QByteArray QHttpNetworkRequest::methodName() const
{
   switch (d->operation) {
      case QHttpNetworkRequest::Get:
         return "GET";
         break;
      case QHttpNetworkRequest::Head:
         return "HEAD";
         break;
      case QHttpNetworkRequest::Post:
         return "POST";
         break;
      case QHttpNetworkRequest::Options:
         return "OPTIONS";
         break;
      case QHttpNetworkRequest::Put:
         return "PUT";
         break;
      case QHttpNetworkRequest::Delete:
         return "DELETE";
         break;
      case QHttpNetworkRequest::Trace:
         return "TRACE";
         break;
      case QHttpNetworkRequest::Connect:
         return "CONNECT";
         break;
      case QHttpNetworkRequest::Custom:
         return d->customVerb;
         break;
      default:
         break;
   }
   return QByteArray();
}

QByteArray QHttpNetworkRequest::uri(bool throughProxy) const
{
   QUrl::FormattingOptions format(QUrl::RemoveFragment | QUrl::RemoveUserInfo | QUrl::FullyEncoded);

   // for POST, query data is send as content
   if (d->operation == QHttpNetworkRequest::Post && ! d->uploadByteDevice) {
      format |= QUrl::RemoveQuery;
   }

   if (! throughProxy) {
      format |= QUrl::RemoveScheme | QUrl::RemoveAuthority;
   }

   QUrl copy = d->url;

   if (copy.path().isEmpty()) {
      copy.setPath("/");
   }

   QByteArray uri = copy.toEncoded(format);
   return uri;
}

QByteArray QHttpNetworkRequestPrivate::header(const QHttpNetworkRequest &request, bool throughProxy)
{
   QList<QPair<QByteArray, QByteArray> > fields = request.header();

   QByteArray ba;
   ba.reserve(40 + fields.length() * 25);       // very rough lower bound estimation

   ba += request.methodName();
   ba += ' ';
   ba += request.uri(throughProxy);

   ba += " HTTP/";
   ba += QByteArray::number(request.majorVersion());
   ba += '.';
   ba += QByteArray::number(request.minorVersion());
   ba += "\r\n";

   QList<QPair<QByteArray, QByteArray> >::const_iterator it    = fields.constBegin();
   QList<QPair<QByteArray, QByteArray> >::const_iterator endIt = fields.constEnd();

   for (; it != endIt; ++it) {
      ba += it->first;
      ba += ": ";
      ba += it->second;
      ba += "\r\n";
   }

   if (request.d->operation == QHttpNetworkRequest::Post) {
      // add content type, if not set in the request

      if (request.headerField("content-type").isEmpty() &&
                  ((request.d->uploadByteDevice && request.d->uploadByteDevice->size() > 0) || request.d->url.hasQuery())) {

         // warning indicates a bug in application code not setting a required header
         // if using QHttpMultipart, the content-type is set in QNetworkAccessManagerPrivate::prepareMultipart already

         qWarning("Content-Type missing in HTTP POST, defaulting to application/x-www-form-urlencoded."
                  " Use QNetworkRequest::setHeader() to fix this problem.");

         ba += "Content-Type: application/x-www-form-urlencoded\r\n";
      }

      if (! request.d->uploadByteDevice && request.d->url.hasQuery()) {
         QByteArray query = request.d->url.query(QUrl::FullyEncoded).toLatin1();
         ba += "Content-Length: ";
         ba += QByteArray::number(query.size());
         ba += "\r\n\r\n";
         ba += query;

      } else {
         ba += "\r\n";
      }

   } else {
      ba += "\r\n";
   }
   return ba;
}

QHttpNetworkRequest::QHttpNetworkRequest(const QUrl &url, Operation operation, Priority priority)
   : d(new QHttpNetworkRequestPrivate(operation, priority, url))
{
}

QHttpNetworkRequest::QHttpNetworkRequest(const QHttpNetworkRequest &other)
   : QHttpNetworkHeader(other), d(other.d)
{
}

QHttpNetworkRequest::~QHttpNetworkRequest()
{
}

QUrl QHttpNetworkRequest::url() const
{
   return d->url;
}

void QHttpNetworkRequest::setUrl(const QUrl &url)
{
   d->url = url;
}

bool QHttpNetworkRequest::isSsl() const
{
   return d->ssl;
}
void QHttpNetworkRequest::setSsl(bool s)
{
   d->ssl = s;
}
bool QHttpNetworkRequest::isPreConnect() const
{
   return d->preConnect;
}
void QHttpNetworkRequest::setPreConnect(bool preConnect)
{
   d->preConnect = preConnect;
}
bool QHttpNetworkRequest::isFollowRedirects() const
{
   return d->followRedirect;
}
void QHttpNetworkRequest::setFollowRedirects(bool followRedirect)
{
   d->followRedirect = followRedirect;
}
int QHttpNetworkRequest::redirectCount() const
{
   return d->redirectCount;
}
void QHttpNetworkRequest::setRedirectCount(int count)
{
   d->redirectCount = count;
}

qint64 QHttpNetworkRequest::contentLength() const
{
   return d->contentLength();
}

void QHttpNetworkRequest::setContentLength(qint64 length)
{
   d->setContentLength(length);
}

QList<QPair<QByteArray, QByteArray> > QHttpNetworkRequest::header() const
{
   return d->fields;
}

QByteArray QHttpNetworkRequest::headerField(const QByteArray &name, const QByteArray &defaultValue) const
{
   return d->headerField(name, defaultValue);
}

void QHttpNetworkRequest::setHeaderField(const QByteArray &name, const QByteArray &data)
{
   d->setHeaderField(name, data);
}

QHttpNetworkRequest &QHttpNetworkRequest::operator=(const QHttpNetworkRequest &other)
{
   d = other.d;
   return *this;
}

bool QHttpNetworkRequest::operator==(const QHttpNetworkRequest &other) const
{
   return d->operator==(*other.d);
}

QHttpNetworkRequest::Operation QHttpNetworkRequest::operation() const
{
   return d->operation;
}

void QHttpNetworkRequest::setOperation(Operation operation)
{
   d->operation = operation;
}

QByteArray QHttpNetworkRequest::customVerb() const
{
   return d->customVerb;
}

void QHttpNetworkRequest::setCustomVerb(const QByteArray &customVerb)
{
   d->customVerb = customVerb;
}

QHttpNetworkRequest::Priority QHttpNetworkRequest::priority() const
{
   return d->priority;
}

void QHttpNetworkRequest::setPriority(Priority priority)
{
   d->priority = priority;
}

bool QHttpNetworkRequest::isPipeliningAllowed() const
{
   return d->pipeliningAllowed;
}

void QHttpNetworkRequest::setPipeliningAllowed(bool b)
{
   d->pipeliningAllowed = b;
}

bool QHttpNetworkRequest::isSPDYAllowed() const
{
   return d->spdyAllowed;
}
void QHttpNetworkRequest::setSPDYAllowed(bool b)
{
   d->spdyAllowed = b;
}
bool QHttpNetworkRequest::withCredentials() const
{
   return d->withCredentials;
}

void QHttpNetworkRequest::setWithCredentials(bool b)
{
   d->withCredentials = b;
}

void QHttpNetworkRequest::setUploadByteDevice(QNonContiguousByteDevice *bd)
{
   d->uploadByteDevice = bd;
}

QNonContiguousByteDevice *QHttpNetworkRequest::uploadByteDevice() const
{
   return d->uploadByteDevice;
}

int QHttpNetworkRequest::majorVersion() const
{
   return 1;
}

int QHttpNetworkRequest::minorVersion() const
{
   return 1;
}


