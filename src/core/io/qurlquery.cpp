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

#include <qurlquery.h>
#include <qurl_p.h>

#include <qhashfunc.h>
#include <qstringlist.h>


typedef QList<QPair<QString, QString> > Map;

class QUrlQueryPrivate : public QSharedData
{
 public:
   QUrlQueryPrivate(const QString &query = QString())
      : valueDelimiter(QUrlQuery::defaultQueryValueDelimiter()),
        pairDelimiter(QUrlQuery::defaultQueryPairDelimiter()) {
      if (!query.isEmpty()) {
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

template<> void QSharedDataPointer<QUrlQueryPrivate>::detach()
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

// Here's how we do the encoding in QUrlQuery
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
   if (qt_urlRecode(output, input.constData(), input.constData() + input.length(),
                    QUrl::DecodeReserved,
                    prettyDecodedActions)) {
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
   // our internal formats are stored in "PrettyDecoded" form
   // and there are no ambiguous characters
   if (idempotentRecodeToUser(encoding)) {
      return input;
   }

   if (!(encoding & QUrl::EncodeDelimiters)) {
      QString output;
      if (qt_urlRecode(output, input.constData(), input.constData() + input.length(),
                       encoding, 0)) {
         return output;
      }
      return input;
   }

   // re-encode the "#" character and the query delimiter pair
   ushort actions[] = { encode(pairDelimiter.unicode()), encode(valueDelimiter.unicode()),
                        encode('#'), 0
                      };
   QString output;
   if (qt_urlRecode(output, input.constData(), input.constData() + input.length(), encoding, actions)) {
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
   const QChar *pos = query.constData();
   const QChar *const end = pos + query.size();
   while (pos != end) {
      const QChar *begin = pos;
      const QChar *delimiter = 0;
      while (pos != end) {
         // scan for the component parts of this pair
         if (!delimiter && pos->unicode() == valueDelimiter) {
            delimiter = pos;
         }
         if (pos->unicode() == pairDelimiter) {
            break;
         }
         ++pos;
      }
      if (!delimiter) {
         delimiter = pos;
      }

      // pos is the end of this pair (the end of the string or the pair delimiter)
      // delimiter points to the value delimiter or to the end of this pair

      QString key;
      if (!qt_urlRecode(key, begin, delimiter,
                        QUrl::DecodeReserved,
                        prettyDecodedActions)) {
         key = QString(begin, delimiter - begin);
      }

      if (delimiter == pos) {
         // the value delimiter wasn't found, store a null value
         itemList.append(qMakePair(key, QString()));
      } else if (delimiter + 1 == pos) {
         // if the delimiter was found but the value is empty, store empty-but-not-null
         itemList.append(qMakePair(key, QString(0, Qt::Uninitialized)));
      } else {
         QString value;
         if (!qt_urlRecode(value, delimiter + 1, pos,
                           QUrl::DecodeReserved,
                           prettyDecodedActions)) {
            value = QString(delimiter + 1, pos - delimiter - 1);
         }
         itemList.append(qMakePair(key, value));
      }

      if (pos != end) {
         ++pos;
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
   : d(0)
{
}

QUrlQuery::QUrlQuery(const QString &queryString)
   : d(queryString.isEmpty() ? 0 : new QUrlQueryPrivate(queryString))
{
}

QUrlQuery::QUrlQuery(const QUrl &url)
   : d(0)
{
   // use internals to avoid unnecessary recoding
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
      // keep in sync with qHash(QUrlQuery):

      return d->valueDelimiter == other.d->valueDelimiter &&
             d->pairDelimiter == other.d->pairDelimiter &&
             d->itemList == other.d->itemList;
   }

   return false;
}

uint qHash(const QUrlQuery &key, uint seed)
{
   if (const QUrlQueryPrivate *d = key.d) {
      // keep in sync with operator==:

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

/*!
    \internal
*/
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

static void recodeAndAppend(QString &to, const QString &input,
                            QUrl::FormattingOptions encoding, const ushort *tableModifications)
{
   if (!qt_urlRecode(to, input.constData(), input.constData() + input.length(), encoding, tableModifications)) {
      to += input;
   }
}

/*!
    Returns the reconstructed query string, formed from the key-value pairs
    currently stored in this QUrlQuery object and separated by the query
    delimiters chosen for this object. The keys and values are encoded using
    the options given by the \a encoding parameter.

    For this function, the only ambiguous delimiter is the hash ("#"), as in
    URLs it is used to separate the query string from the fragment that may
    follow.

    The order of the key-value pairs in the returned string is exactly the same
    as in the original query.

    \sa setQuery(), QUrl::setQuery(), QUrl::fragment(), {encoding}{Encoding}
*/
QString QUrlQuery::query(QUrl::FormattingOptions encoding) const
{
   if (!d) {
      return QString();
   }

   // unlike the component encoding, for the whole query we need to modify a little:
   //  - the "#" character is unambiguous, so we encode it in EncodeDelimiters mode
   //  - the query delimiter pair must always be encoded

   // start with what's always encoded
   ushort tableActions[] = {
      encode(d->pairDelimiter.unicode()),  // 0
      encode(d->valueDelimiter.unicode()), // 1
      0,                                   // 2
      0
   };
   if (encoding & QUrl::EncodeDelimiters) {
      tableActions[2] = encode('#');
   }

   QString result;
   Map::const_iterator it = d->itemList.constBegin();
   Map::const_iterator end = d->itemList.constEnd();

   {
      int size = 0;
      for ( ; it != end; ++it) {
         size += it->first.length() + 1 + it->second.length() + 1;
      }
      result.reserve(size + size / 4);
   }

   for (it = d->itemList.constBegin(); it != end; ++it) {
      if (!result.isEmpty()) {
         result += QChar(d->pairDelimiter);
      }
      recodeAndAppend(result, it->first, encoding, tableActions);
      if (!it->second.isNull()) {
         result += QChar(d->valueDelimiter);
         recodeAndAppend(result, it->second, encoding, tableActions);
      }
   }
   return result;
}

/*!
    Sets the characters used for delimiting between keys and values,
    and between key-value pairs in the URL's query string. The default
    value delimiter is '=' and the default pair delimiter is '&'.

    \image qurl-querystring.png

    \a valueDelimiter will be used for separating keys from values,
    and \a pairDelimiter will be used to separate key-value pairs.
    Any occurrences of these delimiting characters in the encoded
    representation of the keys and values of the query string are
    percent encoded when returned in query().

    If \a valueDelimiter is set to '(' and \a pairDelimiter is ')',
    the above query string would instead be represented like this:

    \snippet code/src_corelib_io_qurl.cpp 4

    \note Non-standard delimiters should be chosen from among what RFC 3986 calls
    "sub-delimiters". They are:

    \code
      sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
                    / "*" / "+" / "," / ";" / "="
    \endcode

    Use of other characters is not supported and may result in unexpected
    behaviour. This method does not verify that you passed a valid delimiter.

    \sa queryValueDelimiter(), queryPairDelimiter()
*/
void QUrlQuery::setQueryDelimiters(QChar valueDelimiter, QChar pairDelimiter)
{
   d->valueDelimiter = valueDelimiter.unicode();
   d->pairDelimiter = pairDelimiter.unicode();
}

/*!
    Returns the character used to delimit between keys and values when
    reconstructing the query string in query() or when parsing in setQuery().

    \sa setQueryDelimiters(), queryPairDelimiter()
*/
QChar QUrlQuery::queryValueDelimiter() const
{
   return d ? d->valueDelimiter : defaultQueryValueDelimiter();
}

/*!
    Returns the character used to delimit between keys-value pairs when
    reconstructing the query string in query() or when parsing in setQuery().

    \sa setQueryDelimiters(), queryValueDelimiter()
*/
QChar QUrlQuery::queryPairDelimiter() const
{
   return d ? d->pairDelimiter : defaultQueryPairDelimiter();
}

/*!
    Sets the items in this QUrlQuery object to \a query. The order of the
    elements in \a query is preserved.

    \note This method does not treat spaces (ASCII 0x20) and plus ("+") signs
    as the same, like HTML forms do. If you need spaces to be represented as
    plus signs, use actual plus signs.

    \sa queryItems(), isEmpty()
*/
void QUrlQuery::setQueryItems(const QList<QPair<QString, QString> > &query)
{
   clear();
   if (query.isEmpty()) {
      return;
   }

   QUrlQueryPrivate *dd = d;
   QList<QPair<QString, QString> >::const_iterator it = query.constBegin(),
                                                   end = query.constEnd();
   for ( ; it != end; ++it) {
      dd->addQueryItem(it->first, it->second);
   }
}

/*!
    Returns the query string of the URL, as a map of keys and values, using the
    options specified in \a encoding to encode the items. The order of the
    elements is the same as the one found in the query string or set with
    setQueryItems().

    \sa setQueryItems(), {encoding}{Encoding}
*/
QList<QPair<QString, QString> > QUrlQuery::queryItems(QUrl::FormattingOptions encoding) const
{
   if (!d) {
      return QList<QPair<QString, QString> >();
   }
   if (idempotentRecodeToUser(encoding)) {
      return d->itemList;
   }

   QList<QPair<QString, QString> > result;
   Map::const_iterator it = d->itemList.constBegin();
   Map::const_iterator end = d->itemList.constEnd();
   result.reserve(d->itemList.count());
   for ( ; it != end; ++it)
      result << qMakePair(d->recodeToUser(it->first, encoding),
                          d->recodeToUser(it->second, encoding));
   return result;
}

/*!
    Returns \c true if there is a query string pair whose key is equal
    to \a key from the URL.

    \sa addQueryItem(), queryItemValue()
*/
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
