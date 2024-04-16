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

#include <qurlquery.h>

#include <qhashfunc.h>
#include <qstringlist.h>

#include <qurl_p.h>

using Map = QList<QPair<QString, QString>>;

class QUrlQueryPrivate : public QSharedData
{
 public:
   QUrlQueryPrivate(const QString &query = QString())
      : valueDelimiter(QUrlQuery::defaultQueryValueDelimiter()),
        pairDelimiter(QUrlQuery::defaultQueryPairDelimiter())
   {

      if (! query.isEmpty()) {
         setQuery(query);
      }
   }

   QString recodeFromUser(const QString &input) const;
   QString recodeToUser(const QString &input, QUrl::FormattingOptions encoding) const;

   void setQuery(const QString &query);

   void addQueryItem(const QString &key, const QString &value) {
      itemList.append(qMakePair(recodeFromUser(key), recodeFromUser(value)));
   }

   int findRecodedKey(const QString &key, int from = 0) const {
      for (int i = from; i < itemList.size(); ++i)
         if (itemList.at(i).first == key) {
            return i;
         }

      return itemList.size();
   }

   Map::const_iterator findKey(const QString &key) const {
      return itemList.constBegin() + findRecodedKey(recodeFromUser(key));
   }

   Map::iterator findKey(const QString &key) {
      return itemList.begin() + findRecodedKey(recodeFromUser(key));
   }

   // use QMap so we end up sorting the items by key
   Map itemList;
   QChar valueDelimiter;
   QChar pairDelimiter;
};

template <>
void QSharedDataPointer<QUrlQueryPrivate>::detach()
{
   if (d && d->ref.load() == 1) {
      return;
   }

   QUrlQueryPrivate *x = (d ? new QUrlQueryPrivate(*d)
               : new QUrlQueryPrivate);
   x->ref.ref();

   if (d && !d->ref.deref()) {
      delete d;
   }

   d = x;
}

// Encoding in QUrlQuery
// The RFC says these are the delimiters:
//    gen-delims    = ":" / "/" / "?" / "#" / "[" / "]" / "@"
//    sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
//                  / "*" / "+" / "," / ";" / "="
// And the definition of query is:
//    query         = *( pchar / "/" / "?" )
//    pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
//
// The strict definition of query says that it can have unencoded any
// unreserved, sub-delim, ":", "@", "/" and "?". Or, by exclusion, excluded
// delimiters are "#", "[" and "]" -- if those are present, they must be
// percent-encoded. The fact that "[" and "]" should be encoded is probably a
// mistake in the spec, so we ignore it and leave the decoded.
//
// The internal storage in the Map is equivalent to PrettyDecoded. That means
// the getter methods, when called with the default encoding value, will not
// have to recode anything (except for toString()).
//
// QUrlQuery handling of delimiters is quite simple: we never touch any of
// them, except for the "#" character and the pair and value delimiters. Those
// are always kept in their decoded forms.
//
// But when recreating the query string, in toString(), we must take care of
// the special delimiters: the pair and value delimiters, as well as the "#"
// character if unambiguous decoding is requested.

#define decode(x) ushort(x)
#define leave(x)  ushort(0x100 | (x))
#define encode(x) ushort(0x200 | (x))

inline QString QUrlQueryPrivate::recodeFromUser(const QString &input) const
{
   // note: duplicated in setQuery()
   QString output;

   ushort prettyDecodedActions[] = {
      decode(pairDelimiter.unicode()),
      decode(valueDelimiter.unicode()),
      decode('#'),
      0
   };

   if (qt_urlRecode(output, input.begin(), input.end(), QUrl::DecodeReserved, prettyDecodedActions)) {

      return output;
   }

   return input;
}

inline bool idempotentRecodeToUser(QUrl::FormattingOptions encoding)
{
   return encoding == QUrl::PrettyDecoded;
}

inline QString QUrlQueryPrivate::recodeToUser(const QString &input, QUrl::FormattingOptions encoding) const
{
   // internal formats are stored in "PrettyDecoded" form
   // and there are no ambiguous characters

   if (idempotentRecodeToUser(encoding)) {
      return input;
   }

   if (! (encoding & QUrl::EncodeDelimiters)) {
      QString output;

      if (qt_urlRecode(output, input.begin(), input.end(), encoding, nullptr)) {
         return output;
      }

      return input;
   }

   // re-encode the "#" character and the query delimiter pair
   ushort actions[] = { encode(pairDelimiter.unicode()), encode(valueDelimiter.unicode()), encode('#'), 0 };

   QString output;

   if (qt_urlRecode(output, input.begin(), input.end(), encoding, actions)) {
      return output;
   }

   return input;
}

void QUrlQueryPrivate::setQuery(const QString &query)
{
   ushort prettyDecodedActions[] = {
      decode(pairDelimiter.unicode()),
      decode(valueDelimiter.unicode()),
      decode('#'),
      0
   };

   itemList.clear();

   QString::const_iterator iter = query.begin();
   QString::const_iterator end  = query.end();

   while (iter != end) {
      QString::const_iterator begin     = iter;
      QString::const_iterator delimiter = end;

      while (iter != end) {
         // scan for the component parts of this pair
         if (delimiter == end && *iter == valueDelimiter) {
            delimiter = iter;
         }

         if (*iter == pairDelimiter) {
            break;
         }

         ++iter;
      }

      if (delimiter == end) {
         delimiter = iter;
      }

      // pos is the end of this pair (the end of the string or the pair delimiter)
      // delimiter points to the value delimiter or to the end of this pair

      QString key;

      if (! qt_urlRecode(key, begin, delimiter, QUrl::DecodeReserved, prettyDecodedActions)) {
         key = QString(begin, delimiter);
      }

      if (delimiter == iter) {
         // the value delimiter wasn't found, store a null value
         itemList.append(qMakePair(key, QString()));

      } else if (delimiter + 1 == iter) {
         itemList.append(qMakePair(key, QString()));

      } else {
         QString value;

         if (! qt_urlRecode(value, delimiter + 1, iter, QUrl::DecodeReserved, prettyDecodedActions)) {
            value = QString(delimiter + 1, iter);
         }

         itemList.append(qMakePair(key, value));
      }

      if (iter != end) {
         ++iter;
      }
   }
}

// allow QUrlQueryPrivate to detach from null
template <> inline QUrlQueryPrivate *
QSharedDataPointer<QUrlQueryPrivate>::clone()
{
   return d ? new QUrlQueryPrivate(*d) : new QUrlQueryPrivate;
}

QUrlQuery::QUrlQuery()
   : d(nullptr)
{
}

QUrlQuery::QUrlQuery(const QString &queryString)
   : d(queryString.isEmpty() ? nullptr : new QUrlQueryPrivate(queryString))
{
}

QUrlQuery::QUrlQuery(const QUrl &url)
   : d(nullptr)
{
   if (url.hasQuery()) {
      d = new QUrlQueryPrivate(url.query());
   }
}

QUrlQuery::QUrlQuery(const QUrlQuery &other)
   : d(other.d)
{
}

QUrlQuery &QUrlQuery::operator =(const QUrlQuery &other)
{
   d = other.d;
   return *this;
}

QUrlQuery::~QUrlQuery()
{
}

bool QUrlQuery::operator ==(const QUrlQuery &other) const
{
   if (d == other.d) {
      return true;
   }

   if (d && other.d) {
      // keep in sync with qHash(QUrlQuery)

      return d->valueDelimiter == other.d->valueDelimiter &&
            d->pairDelimiter == other.d->pairDelimiter &&
            d->itemList == other.d->itemList;
   }

   return false;
}

uint qHash(const QUrlQuery &key, uint seed)
{
   if (const QUrlQueryPrivate *d = key.d) {
      // keep in sync with operator==

      seed = qHash(d->valueDelimiter, seed);
      seed = qHash(d->pairDelimiter, seed);
      seed = qHash(d->itemList, seed);
   }

   return seed;
}

bool QUrlQuery::isEmpty() const
{
   return d ? d->itemList.isEmpty() : true;
}

bool QUrlQuery::isDetached() const
{
   return d && d->ref.load() == 1;
}

void QUrlQuery::clear()
{
   if (d.constData()) {
      d->itemList.clear();
   }
}

void QUrlQuery::setQuery(const QString &queryString)
{
   d->setQuery(queryString);
}

static void recodeAndAppend(QString &to, const QString &input, QUrl::FormattingOptions encoding,
      const ushort *tableModifications)
{
   if (!qt_urlRecode(to, input.constBegin(), input.constEnd(), encoding, tableModifications)) {
      to += input;
   }
}

QString QUrlQuery::query(QUrl::FormattingOptions encoding) const
{
   if (! d) {
      return QString();
   }

   // unlike the component encoding, for the whole query we need to modify a little:
   //  - the "#" character is unambiguous, so we encode it in EncodeDelimiters mode
   //  - the query delimiter pair must always be encoded

   // start with what's always encoded
   ushort tableActions[] = {
      encode(d->pairDelimiter.unicode()),
      encode(d->valueDelimiter.unicode()),
      0,
      0
   };

   if (encoding & QUrl::EncodeDelimiters) {
      tableActions[2] = encode('#');
   }

   QString result;
   Map::const_iterator it = d->itemList.constBegin();
   Map::const_iterator end = d->itemList.constEnd();

   for (it = d->itemList.constBegin(); it != end; ++it) {
      if (!result.isEmpty()) {
         result += QChar(d->pairDelimiter);
      }

      recodeAndAppend(result, it->first, encoding, tableActions);

      if (! it->second.isEmpty()) {
         result += QChar(d->valueDelimiter);
         recodeAndAppend(result, it->second, encoding, tableActions);
      }
   }

   return result;
}

void QUrlQuery::setQueryDelimiters(QChar valueDelimiter, QChar pairDelimiter)
{
   d->valueDelimiter = valueDelimiter;
   d->pairDelimiter  = pairDelimiter;
}

QChar QUrlQuery::queryValueDelimiter() const
{
   return d ? d->valueDelimiter : defaultQueryValueDelimiter();
}

QChar QUrlQuery::queryPairDelimiter() const
{
   return d ? d->pairDelimiter : defaultQueryPairDelimiter();
}

void QUrlQuery::setQueryItems(const QList<QPair<QString, QString>> &query)
{
   clear();

   if (query.isEmpty()) {
      return;
   }

   QUrlQueryPrivate *dd = d;

   QList<QPair<QString, QString>>::const_iterator it = query.constBegin();
   QList<QPair<QString, QString>>::const_iterator end = query.constEnd();

   for ( ; it != end; ++it) {
      dd->addQueryItem(it->first, it->second);
   }
}

QList<QPair<QString, QString>> QUrlQuery::queryItems(QUrl::FormattingOptions encoding) const
{
   if (! d) {
      return QList<QPair<QString, QString>>();
   }

   if (idempotentRecodeToUser(encoding)) {
      return d->itemList;
   }

   QList<QPair<QString, QString>> result;

   Map::const_iterator it  = d->itemList.constBegin();
   Map::const_iterator end = d->itemList.constEnd();

   for ( ; it != end; ++it) {
      result << qMakePair(d->recodeToUser(it->first, encoding), d->recodeToUser(it->second, encoding));
   }

   return result;
}

bool QUrlQuery::hasQueryItem(const QString &key) const
{
   if (!d) {
      return false;
   }

   return d->findKey(key) != d->itemList.constEnd();
}

void QUrlQuery::addQueryItem(const QString &key, const QString &value)
{
   d->addQueryItem(key, value);
}

QString QUrlQuery::queryItemValue(const QString &key, QUrl::FormattingOptions encoding) const
{
   QString result;

   if (d) {
      Map::const_iterator it = d->findKey(key);

      if (it != d->itemList.constEnd()) {
         result = d->recodeToUser(it->second, encoding);
      }
   }

   return result;
}

QStringList QUrlQuery::allQueryItemValues(const QString &key, QUrl::FormattingOptions encoding) const
{
   QStringList result;

   if (d) {
      QString encodedKey = d->recodeFromUser(key);
      int idx = d->findRecodedKey(encodedKey);

      while (idx < d->itemList.size()) {
         result << d->recodeToUser(d->itemList.at(idx).second, encoding);
         idx = d->findRecodedKey(encodedKey, idx + 1);
      }
   }

   return result;
}

void QUrlQuery::removeQueryItem(const QString &key)
{
   if (d) {
      Map::iterator it = d->findKey(key);

      if (it != d->itemList.end()) {
         d->itemList.erase(it);
      }
   }
}

void QUrlQuery::removeAllQueryItems(const QString &key)
{
   if (d.constData()) {
      QString encodedKey = d->recodeFromUser(key);
      Map::iterator it = d->itemList.begin();

      while (it != d->itemList.end()) {
         if (it->first == encodedKey) {
            it = d->itemList.erase(it);
         } else {
            ++it;
         }
      }
   }
}
