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

#include <qabstract_networkcache.h>

#include <qdatetime.h>
#include <qurl.h>
#include <qdebug.h>
#include <qscopedpointer.h>

#include <qabstract_networkcache_p.h>

class QNetworkCacheMetaDataPrivate : public QSharedData
{
 public:
   QNetworkCacheMetaDataPrivate()
      : QSharedData(), saveToDisk(true) {
   }

   bool operator==(const QNetworkCacheMetaDataPrivate &other) const {
      return
         url == other.url
         && lastModified == other.lastModified
         && expirationDate == other.expirationDate
         && headers == other.headers
         && saveToDisk == other.saveToDisk;
   }

   QUrl url;
   QDateTime lastModified;
   QDateTime expirationDate;
   QNetworkCacheMetaData::RawHeaderList headers;            // QList
   QNetworkCacheMetaData::AttributesMap attributes;         // QHash
   bool saveToDisk;

   static void save(QDataStream &out, const QNetworkCacheMetaData &metaData);
   static void load(QDataStream &in, QNetworkCacheMetaData &metaData);
};

static QNetworkCacheMetaDataPrivate *metadata_shared_invalid()
{
   static QNetworkCacheMetaDataPrivate retval;
   return &retval;
}

QNetworkCacheMetaData::QNetworkCacheMetaData()
   : d(new QNetworkCacheMetaDataPrivate)
{
}

QNetworkCacheMetaData::~QNetworkCacheMetaData()
{
   // QSharedDataPointer takes care of freeing d
}

QNetworkCacheMetaData::QNetworkCacheMetaData(const QNetworkCacheMetaData &other)
   : d(other.d)
{
}

QNetworkCacheMetaData &QNetworkCacheMetaData::operator=(const QNetworkCacheMetaData &other)
{
   d = other.d;
   return *this;
}

bool QNetworkCacheMetaData::operator==(const QNetworkCacheMetaData &other) const
{
   if (d == other.d) {
      return true;
   }
   if (d && other.d) {
      return *d == *other.d;
   }
   return false;
}

bool QNetworkCacheMetaData::isValid() const
{
   return !(*d == *metadata_shared_invalid());
}

bool QNetworkCacheMetaData::saveToDisk() const
{
   return d->saveToDisk;
}

void QNetworkCacheMetaData::setSaveToDisk(bool allow)
{
   d->saveToDisk = allow;
}

QUrl QNetworkCacheMetaData::url() const
{
   return d->url;
}

void QNetworkCacheMetaData::setUrl(const QUrl &url)
{
   d->url = url;
   d->url.setPassword(QString());
   d->url.setFragment(QString());
}

QNetworkCacheMetaData::RawHeaderList QNetworkCacheMetaData::rawHeaders() const
{
   return d->headers;
}

void QNetworkCacheMetaData::setRawHeaders(const RawHeaderList &list)
{
   d->headers = list;
}

QDateTime QNetworkCacheMetaData::lastModified() const
{
   return d->lastModified;
}

void QNetworkCacheMetaData::setLastModified(const QDateTime &dateTime)
{
   d->lastModified = dateTime;
}

QDateTime QNetworkCacheMetaData::expirationDate() const
{
   return d->expirationDate;
}

void QNetworkCacheMetaData::setExpirationDate(const QDateTime &dateTime)
{
   d->expirationDate = dateTime;
}

QNetworkCacheMetaData::AttributesMap QNetworkCacheMetaData::attributes() const
{
   return d->attributes;
}

void QNetworkCacheMetaData::setAttributes(const AttributesMap &attributes)
{
   d->attributes = attributes;
}

QDataStream &operator<<(QDataStream &out, const QNetworkCacheMetaData &metaData)
{
   QNetworkCacheMetaDataPrivate::save(out, metaData);
   return out;
}

static inline QDataStream &operator<<(QDataStream &out, const QNetworkCacheMetaData::AttributesMap &hash)
{
   out << quint32(hash.size());

   for (auto iter = hash.begin(); iter != hash.end(); ++iter) {
      out << int(iter.key()) << iter.value();
   }

   return out;
}

void QNetworkCacheMetaDataPrivate::save(QDataStream &out, const QNetworkCacheMetaData &metaData)
{
   // note: if you change the contents of the meta data here
   // remember to bump the cache version in qnetworkdiskcache.cpp CurrentCacheVersion

   out << metaData.url();
   out << metaData.expirationDate();
   out << metaData.lastModified();
   out << metaData.saveToDisk();
   out << metaData.attributes();
   out << metaData.rawHeaders();
}

QDataStream &operator>>(QDataStream &in, QNetworkCacheMetaData &metaData)
{
   QNetworkCacheMetaDataPrivate::load(in, metaData);
   return in;
}

static inline QDataStream &operator>>(QDataStream &in, QNetworkCacheMetaData::AttributesMap &hash)
{
   hash.clear();

   QDataStream::Status oldStatus = in.status();
   in.resetStatus();

   quint32 n;
   in >> n;

   for (quint32 i = 0; i < n; ++i) {
      if (in.status() != QDataStream::Ok) {
         break;
      }

      int key;
      QVariant value;

      in >> key;
      in >> value;

      hash.insert(QNetworkRequest::Attribute(key), value);
   }

   if (in.status() != QDataStream::Ok) {
      hash.clear();
   }

   if (oldStatus != QDataStream::Ok) {
      in.setStatus(oldStatus);
   }

   return in;
}

void QNetworkCacheMetaDataPrivate::load(QDataStream &in, QNetworkCacheMetaData &metaData)
{
   in >> metaData.d->url;
   in >> metaData.d->expirationDate;
   in >> metaData.d->lastModified;
   in >> metaData.d->saveToDisk;
   in >> metaData.d->attributes;         // QHash
   in >> metaData.d->headers;
}

QAbstractNetworkCache::QAbstractNetworkCache(QObject *parent)
   : QObject(parent), d_ptr(new QAbstractNetworkCachePrivate)
{
   d_ptr->q_ptr = this;
}

QAbstractNetworkCache::QAbstractNetworkCache(QAbstractNetworkCachePrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
}

QAbstractNetworkCache::~QAbstractNetworkCache()
{
}
