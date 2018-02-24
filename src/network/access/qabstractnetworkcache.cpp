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

#include <qabstractnetworkcache.h>
#include <qabstractnetworkcache_p.h>

#include <qdatetime.h>
#include <qurl.h>
#include <qdebug.h>
#include <QScopedPointer>

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
Q_GLOBAL_STATIC(QNetworkCacheMetaDataPrivate, metadata_shared_invalid)


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

/*!
    Makes a copy of the \a other QNetworkCacheMetaData and returns a reference to the copy.
 */
QNetworkCacheMetaData &QNetworkCacheMetaData::operator=(const QNetworkCacheMetaData &other)
{
   d = other.d;
   return *this;
}

/*!
    Returns true if this meta data is equal to the \a other meta data; otherwise returns false.

    \sa operator!=()
 */
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

/*!
    \fn bool QNetworkCacheMetaData::operator!=(const QNetworkCacheMetaData &other) const

    Returns true if this meta data is not equal to the \a other meta data; otherwise returns false.

    \sa operator==()
 */

/*!
    Returns true if this network cache meta data has attributes that have been set otherwise false.
 */
bool QNetworkCacheMetaData::isValid() const
{
   return !(*d == *metadata_shared_invalid());
}

/*!
    Returns is this cache should be allowed to be stored on disk.

    Some cache implementations can keep these cache items in memory for performance reasons,
    but for security reasons they should not be written to disk.

    Specifically with http, documents marked with Pragma: no-cache, or have a Cache-control set to
    no-store or no-cache or any https document that doesn't have "Cache-control: public" set will
    set the saveToDisk to false.

    \sa setSaveToDisk()
 */
bool QNetworkCacheMetaData::saveToDisk() const
{
   return d->saveToDisk;
}

/*!
    Sets whether this network cache meta data and associated content should be
    allowed to be stored on disk to \a allow.

    \sa saveToDisk()
 */
void QNetworkCacheMetaData::setSaveToDisk(bool allow)
{
   d->saveToDisk = allow;
}

/*!
    Returns the URL this network cache meta data is referring to.

    \sa setUrl()
 */
QUrl QNetworkCacheMetaData::url() const
{
   return d->url;
}

/*!
    Sets the URL this network cache meta data to to be \a url.

    The password and fragment are removed from the url.

    \sa url()
 */
void QNetworkCacheMetaData::setUrl(const QUrl &url)
{
   d->url = url;
   d->url.setPassword(QString());
   d->url.setFragment(QString());
}

/*!
    Returns a list of all raw headers that are set in this meta data.
    The list is in the same order that the headers were set.

    \sa setRawHeaders()
 */
QNetworkCacheMetaData::RawHeaderList QNetworkCacheMetaData::rawHeaders() const
{
   return d->headers;
}

/*!
    Sets the raw headers to \a list.

    \sa rawHeaders()
 */
void QNetworkCacheMetaData::setRawHeaders(const RawHeaderList &list)
{
   d->headers = list;
}

/*!
    Returns the date and time when the meta data was last modified.
 */
QDateTime QNetworkCacheMetaData::lastModified() const
{
   return d->lastModified;
}

/*!
    Sets the date and time when the meta data was last modified to \a dateTime.
 */
void QNetworkCacheMetaData::setLastModified(const QDateTime &dateTime)
{
   d->lastModified = dateTime;
}

/*!
    Returns the date and time when the meta data expires.
 */
QDateTime QNetworkCacheMetaData::expirationDate() const
{
   return d->expirationDate;
}

/*!
    Sets the date and time when the meta data expires to \a dateTime.
 */
void QNetworkCacheMetaData::setExpirationDate(const QDateTime &dateTime)
{
   d->expirationDate = dateTime;
}

/*!
    \since 4.6

    Returns all the attributes stored with this cache item.

    \sa setAttributes(), QNetworkRequest::Attribute
*/
QNetworkCacheMetaData::AttributesMap QNetworkCacheMetaData::attributes() const
{
   return d->attributes;
}

/*!
    \since 4.6

    Sets all attributes of this cache item to be the map \a attributes.

    \sa attributes(), QNetworkRequest::setAttribute()
*/
void QNetworkCacheMetaData::setAttributes(const AttributesMap &attributes)
{
   d->attributes = attributes;
}

/*!
    \relates QNetworkCacheMetaData
    \since 4.5

    Writes \a metaData to the \a out stream.

    \sa {Serializing Qt Data Types}
*/
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
