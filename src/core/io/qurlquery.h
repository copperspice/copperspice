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

#ifndef QURL_QUERY_H
#define QURL_QUERY_H

#include <qpair.h>
#include <qshareddata.h>
#include <qstringlist.h>
#include <qurl.h>

class QUrlQueryPrivate;

Q_CORE_EXPORT uint qHash(const QUrlQuery &key, uint seed = 0);

class Q_CORE_EXPORT QUrlQuery
{
 public:
   QUrlQuery();
   QUrlQuery(const QUrlQuery &other);

   explicit QUrlQuery(const QUrl &url);
   explicit QUrlQuery(const QString &str);

   ~QUrlQuery();

   // methods
   void addQueryItem(const QString &key, const QString &value);
   QStringList allQueryItemValues(const QString &key, QUrl::FormattingOptions encoding = QUrl::PrettyDecoded) const;

   void clear();

   static QChar defaultQueryValueDelimiter() {
      return QChar(ushort('='));
   }

   static QChar defaultQueryPairDelimiter() {
      return QChar(ushort('&'));
   }

   bool isEmpty() const;
   bool isDetached() const;

   bool hasQueryItem(const QString &key) const;

   QString queryItemValue(const QString &key, QUrl::FormattingOptions encoding = QUrl::PrettyDecoded) const;

   QString query(QUrl::FormattingOptions encoding = QUrl::PrettyDecoded) const;

   QList<QPair<QString, QString>> queryItems(QUrl::FormattingOptions encoding = QUrl::PrettyDecoded) const;
   QChar queryPairDelimiter() const;
   QChar queryValueDelimiter() const;

   void setQuery(const QString &str);
   void setQueryDelimiters(QChar valueDelimiter, QChar pairDelimiter);
   void setQueryItems(const QList<QPair<QString, QString> > &query);

   void swap(QUrlQuery &other) {
      qSwap(d, other.d);
   }

   void removeQueryItem(const QString &key);
   void removeAllQueryItems(const QString &key);

   QString toString(QUrl::FormattingOptions encoding = QUrl::PrettyDecoded) const {
      return query(encoding);
   }


   typedef QSharedDataPointer<QUrlQueryPrivate> DataPtr;
   inline DataPtr &data_ptr() {
      return d;
   }

   // operators
   QUrlQuery &operator=(const QUrlQuery &other);

   QUrlQuery &operator=(QUrlQuery &&other) {
      swap(other);
      return *this;
   }

   bool operator==(const QUrlQuery &other) const;

   bool operator!=(const QUrlQuery &other) const {
      return !(*this == other);
   }

 private:
   QSharedDataPointer<QUrlQueryPrivate> d;

   friend class QUrl;
   friend Q_CORE_EXPORT uint qHash(const QUrlQuery &key, uint seed);
};

Q_DECLARE_SHARED(QUrlQuery)

inline void QUrl::setQueryItems(const QList<QPair<QString, QString> > &qry)
{
   QUrlQuery q(*this);
   q.setQueryItems(qry);
   setQuery(q);
}

inline void QUrl::addQueryItem(const QString &key, const QString &value)
{
   QUrlQuery q(*this);
   q.addQueryItem(key, value);
   setQuery(q);
}

inline QList<QPair<QString, QString> > QUrl::queryItems() const
{
   return QUrlQuery(*this).queryItems();
}

inline bool QUrl::hasQueryItem(const QString &key) const
{
   return QUrlQuery(*this).hasQueryItem(key);
}

inline QString QUrl::queryItemValue(const QString &key) const
{
   return QUrlQuery(*this).queryItemValue(key);
}

inline QStringList QUrl::allQueryItemValues(const QString &key) const
{
   return QUrlQuery(*this).allQueryItemValues(key);
}

inline void QUrl::removeQueryItem(const QString &key)
{
   QUrlQuery q(*this);
   q.removeQueryItem(key);
   setQuery(q);
}

inline void QUrl::removeAllQueryItems(const QString &key)
{
   QUrlQuery q(*this);
   q.removeAllQueryItems(key);
   setQuery(q);
}

inline void QUrl::addEncodedQueryItem(const QByteArray &key, const QByteArray &value)
{
   QUrlQuery q(*this);
   q.addQueryItem(fromEncodedComponent_helper(key), fromEncodedComponent_helper(value));
   setQuery(q);
}

inline bool QUrl::hasEncodedQueryItem(const QByteArray &key) const
{
   return QUrlQuery(*this).hasQueryItem(fromEncodedComponent_helper(key));
}

inline QByteArray QUrl::encodedQueryItemValue(const QByteArray &key) const
{
   return QUrlQuery(*this).queryItemValue(fromEncodedComponent_helper(key), QUrl::FullyEncoded).toLatin1();
}

inline void QUrl::removeEncodedQueryItem(const QByteArray &key)
{
   QUrlQuery q(*this);
   q.removeQueryItem(fromEncodedComponent_helper(key));
   setQuery(q);
}

inline void QUrl::removeAllEncodedQueryItems(const QByteArray &key)
{
   QUrlQuery q(*this);
   q.removeAllQueryItems(fromEncodedComponent_helper(key));
   setQuery(q);
}

inline void QUrl::setEncodedQueryItems(const QList<QPair<QByteArray, QByteArray> > &qry)
{
   QUrlQuery q;
   QList<QPair<QByteArray, QByteArray> >::ConstIterator it = qry.constBegin();
   for ( ; it != qry.constEnd(); ++it) {
      q.addQueryItem(fromEncodedComponent_helper(it->first), fromEncodedComponent_helper(it->second));
   }
   setQuery(q);
}

inline QList<QPair<QByteArray, QByteArray> > QUrl::encodedQueryItems() const
{
   QList<QPair<QString, QString> > items = QUrlQuery(*this).queryItems(QUrl::FullyEncoded);
   QList<QPair<QString, QString> >::ConstIterator it = items.constBegin();
   QList<QPair<QByteArray, QByteArray> > result;

   result.reserve(items.size());
   for ( ; it != items.constEnd(); ++it) {
      result << qMakePair(it->first.toLatin1(), it->second.toLatin1());
   }
   return result;
}

inline QList<QByteArray> QUrl::allEncodedQueryItemValues(const QByteArray &key) const
{
   QStringList items = QUrlQuery(*this).allQueryItemValues(fromEncodedComponent_helper(key), QUrl::FullyEncoded);
   QList<QByteArray> result;
   result.reserve(items.size());

   for (const QString &item : items) {
      result << item.toLatin1();
   }

   return result;
}

#endif
