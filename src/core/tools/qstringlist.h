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

#ifndef QSTRINGLIST_H
#define QSTRINGLIST_H

#include <qalgorithms.h>
#include <qdatastream.h>
#include <qlist.h>
#include <qregexp.h>
#include <qstring.h>

class QRegExp;

using QStringListIterator        = QListIterator<QString>;
using QMutableStringListIterator = QMutableListIterator<QString>;

class Q_CORE_EXPORT QStringList : public QList<QString>
{
 public:
   QStringList() { }

   explicit QStringList(const QString &value) {
      append(value);
   }

   QStringList(const QStringList &other)
      : QList<QString>(other) { }

   QStringList(const QList<QString> &other)
      : QList<QString>(other) { }

   QStringList(std::initializer_list<QString> args)
      : QList<QString>(args) { }

   // methods
   bool contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

   QStringList filter(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

   QString join(const QString &sep) const;

   int removeDuplicates();
   QStringList &replaceInStrings(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);

   void sort();

   // operators
   QStringList operator+(const QStringList &other) const {
      QStringList n = *this;
      n += other;
      return n;
   }

   QStringList &operator<<(const QString &str) {
      append(str);
      return *this;
   }

   QStringList &operator<<(const QStringList &value) {
      *this += value;
      return *this;
   }

#ifndef QT_NO_REGEXP
   QStringList filter(const QRegExp &rx) const;

   int indexOf(const QRegExp &rx, int from = 0) const;
   int lastIndexOf(const QRegExp &rx, int from = -1) const;
   int indexOf(QRegExp &rx, int from = 0) const;
   int lastIndexOf(QRegExp &rx, int from = -1) const;

   QStringList &replaceInStrings(const QRegExp &rx, const QString &after);
#endif

   using QList<QString>::indexOf;
   using QList<QString>::lastIndexOf;
};

Q_DECLARE_TYPEINFO(QStringList, Q_MOVABLE_TYPE);

#ifndef QT_NO_DATASTREAM

inline QDataStream &operator>>(QDataStream &in, QStringList &list)
{
   return operator>>(in, static_cast<QList<QString> &>(list));
}

inline QDataStream &operator<<(QDataStream &out, const QStringList &list)
{
   return operator<<(out, static_cast<const QList<QString> &>(list));
}

#endif

#endif
