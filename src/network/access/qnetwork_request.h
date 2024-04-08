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

#ifndef QNETWORK_REQUEST_H
#define QNETWORK_REQUEST_H

#include <qshareddatapointer.h>
#include <qstring.h>
#include <qurl.h>
#include <qvariant.h>

class QSslConfiguration;
class QNetworkRequestPrivate;

class Q_NETWORK_EXPORT QNetworkRequest
{
 public:
   enum KnownHeaders {
      ContentTypeHeader,
      ContentLengthHeader,
      LocationHeader,
      LastModifiedHeader,
      CookieHeader,
      SetCookieHeader,
      ContentDispositionHeader,  // added for QMultipartMessage
      UserAgentHeader,
      ServerHeader
   };

   enum Attribute {
      HttpStatusCodeAttribute,
      HttpReasonPhraseAttribute,
      RedirectionTargetAttribute,
      ConnectionEncryptedAttribute,
      CacheLoadControlAttribute,
      CacheSaveControlAttribute,
      SourceIsFromCacheAttribute,
      DoNotBufferUploadDataAttribute,
      HttpPipeliningAllowedAttribute,
      HttpPipeliningWasUsedAttribute,
      CustomVerbAttribute,
      CookieLoadControlAttribute,
      AuthenticationReuseAttribute,
      CookieSaveControlAttribute,
      MaximumDownloadBufferSizeAttribute, // internal
      DownloadBufferAttribute, // internal
      SynchronousRequestAttribute, // internal
      BackgroundRequestAttribute,
      SpdyAllowedAttribute,
      SpdyWasUsedAttribute,
      EmitAllUploadProgressSignalsAttribute,
      FollowRedirectsAttribute,

      User = 1000,
      UserMax = 32767
   };

   enum CacheLoadControl {
      AlwaysNetwork,
      PreferNetwork,
      PreferCache,
      AlwaysCache
   };

   enum LoadControl {
      Automatic = 0,
      Manual
   };

   enum Priority {
      HighPriority = 1,
      NormalPriority = 3,
      LowPriority = 5
   };

   explicit QNetworkRequest(const QUrl &url = QUrl());
   QNetworkRequest(const QNetworkRequest &other);
   ~QNetworkRequest();

   QNetworkRequest &operator=(QNetworkRequest &&other) {
      swap(other);
      return *this;
   }

   QNetworkRequest &operator=(const QNetworkRequest &other);

   void swap(QNetworkRequest &other)  {
      qSwap(d, other.d);
   }

   bool operator==(const QNetworkRequest &other) const;
   bool operator!=(const QNetworkRequest &other) const {
      return !operator==(other);
   }

   QUrl url() const;
   void setUrl(const QUrl &url);

   // "cooked" headers
   QVariant header(KnownHeaders header) const;
   void setHeader(KnownHeaders header, const QVariant &value);

   // raw headers:
   bool hasRawHeader(const QByteArray &headerName) const;
   QList<QByteArray> rawHeaderList() const;
   QByteArray rawHeader(const QByteArray &headerName) const;
   void setRawHeader(const QByteArray &headerName, const QByteArray &headerValue);

   // attributes
   QVariant attribute(Attribute code, const QVariant &defaultValue = QVariant()) const;
   void setAttribute(Attribute code, const QVariant &value);

#ifdef QT_SSL
   QSslConfiguration sslConfiguration() const;
   void setSslConfiguration(const QSslConfiguration &configuration);
#endif

   void setOriginatingObject(QObject *object);
   QObject *originatingObject() const;

   Priority priority() const;
   void setPriority(Priority priority);

   // HTTP redirect related
   int maximumRedirectsAllowed() const;
   void setMaximumRedirectsAllowed(int maxRedirectsAllowed);

 private:
   QSharedDataPointer<QNetworkRequestPrivate> d;
   friend class QNetworkRequestPrivate;
};

CS_DECLARE_METATYPE(QNetworkRequest)

#endif
