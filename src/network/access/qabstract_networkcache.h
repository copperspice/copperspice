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

#ifndef QABSTRACT_NETWORKCACHE_H
#define QABSTRACT_NETWORKCACHE_H

#include <qobject.h>
#include <qshareddata.h>
#include <qpair.h>
#include <qnetwork_request.h>

class QIODevice;
class QDateTime;
class QNetworkCacheMetaDataPrivate;
class QAbstractNetworkCachePrivate;
class QUrl;

template <class T>
class QList;

class Q_NETWORK_EXPORT QNetworkCacheMetaData
{
 public:
   typedef QPair<QByteArray, QByteArray> RawHeader;
   typedef QList<RawHeader> RawHeaderList;
   typedef QHash<QNetworkRequest::Attribute, QVariant> AttributesMap;

   QNetworkCacheMetaData();
   QNetworkCacheMetaData(const QNetworkCacheMetaData &other);
   ~QNetworkCacheMetaData();

   QNetworkCacheMetaData &operator=(QNetworkCacheMetaData &&other)  {
      swap(other);
      return *this;
   }

   QNetworkCacheMetaData &operator=(const QNetworkCacheMetaData &other);

   void swap(QNetworkCacheMetaData &other) {
      qSwap(d, other.d);
   }

   bool operator==(const QNetworkCacheMetaData &other) const;

   bool operator!=(const QNetworkCacheMetaData &other) const {
      return !(*this == other);
   }

   bool isValid() const;

   QUrl url() const;
   void setUrl(const QUrl &url);

   RawHeaderList rawHeaders() const;
   void setRawHeaders(const RawHeaderList &list);

   QDateTime lastModified() const;
   void setLastModified(const QDateTime &dateTime);

   QDateTime expirationDate() const;
   void setExpirationDate(const QDateTime &dateTime);

   bool saveToDisk() const;
   void setSaveToDisk(bool allow);

   AttributesMap attributes() const;
   void setAttributes(const AttributesMap &attributes);

 private:
   friend class QNetworkCacheMetaDataPrivate;
   QSharedDataPointer<QNetworkCacheMetaDataPrivate> d;
};

Q_NETWORK_EXPORT QDataStream &operator<<(QDataStream &, const QNetworkCacheMetaData &);
Q_NETWORK_EXPORT QDataStream &operator>>(QDataStream &, QNetworkCacheMetaData &);

class Q_NETWORK_EXPORT QAbstractNetworkCache : public QObject
{
   NET_CS_OBJECT(QAbstractNetworkCache)

 public:
   QAbstractNetworkCache(const QAbstractNetworkCache &) = delete;
   QAbstractNetworkCache &operator=(const QAbstractNetworkCache &) = delete;

   virtual ~QAbstractNetworkCache();

   virtual QNetworkCacheMetaData metaData(const QUrl &url) = 0;
   virtual void updateMetaData(const QNetworkCacheMetaData &metaData) = 0;
   virtual QIODevice *data(const QUrl &url) = 0;
   virtual bool remove(const QUrl &url) = 0;
   virtual qint64 cacheSize() const = 0;

   virtual QIODevice *prepare(const QNetworkCacheMetaData &metaData) = 0;
   virtual void insert(QIODevice *device) = 0;

   NET_CS_SLOT_1(Public, virtual void clear() = 0)
   NET_CS_SLOT_2(clear)

 protected:
   explicit QAbstractNetworkCache(QObject *parent = nullptr);
   QAbstractNetworkCache(QAbstractNetworkCachePrivate &dd, QObject *parent);

   QScopedPointer<QAbstractNetworkCachePrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QAbstractNetworkCache)
};

#endif
