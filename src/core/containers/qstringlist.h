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

#ifndef QSTRINGLIST_H
#define QSTRINGLIST_H

#include <qlist.h>
#include <qstring.h>

class QDataStream;

using QStringListIterator        = QListIterator<QString>;
using QMutableStringListIterator = QMutableListIterator<QString>;

class QStringList : public QList<QString>
{
 public:
   QStringList()
   { }

   explicit QStringList(const QString &value) {
      append(value);
   }

   QStringList(const QStringList &other)
      : QList<QString>(other)
   { }

   QStringList(const QList<QString> &other)
      : QList<QString>(other)
   { }

   QStringList(std::initializer_list<QString> args)
      : QList<QString>(args)
   { }

   // methods
   bool Q_CORE_EXPORT contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

   QStringList Q_CORE_EXPORT filter(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

   QString Q_CORE_EXPORT join(const QString &separator) const;

   int Q_CORE_EXPORT removeDuplicates();
   Q_CORE_EXPORT QStringList &replaceInStrings(const QString &before, const QString &after,
         Qt::CaseSensitivity cs = Qt::CaseSensitive);

   void Q_CORE_EXPORT sort();

   // operators
   QStringList &operator=(const QStringList &other) {
      QList<QString>::operator=(other);
      return *this;
   }

   QStringList operator+(const QStringList &other) const {
      QStringList n = *this;
      n += other;
      return n;
   }

   QStringList &operator<<(const QString &str) {
      append(str);
      return *this;
   }

   QStringList &operator<<(const QStringList &other) {
      *this += other;
      return *this;
   }

   Q_CORE_EXPORT QStringList filter(const QRegularExpression8 &rx) const;

   int Q_CORE_EXPORT indexOf(const QRegularExpression8 &rx, int from = 0) const;
   int Q_CORE_EXPORT lastIndexOf(const QRegularExpression8 &rx, int from = -1) const;
   int Q_CORE_EXPORT indexOf(QRegularExpression8 &rx, int from = 0) const;
   int Q_CORE_EXPORT lastIndexOf(QRegularExpression8 &rx, int from = -1) const;

   Q_CORE_EXPORT QStringList &replaceInStrings(const QRegularExpression8 &rx, const QString &after);

   using QList<QString>::indexOf;
   using QList<QString>::lastIndexOf;
};

Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QStringList &list);
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QStringList &list);

#endif
