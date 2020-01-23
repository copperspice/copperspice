/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QSQLINDEX_H
#define QSQLINDEX_H

#include <QtSql/qsqlrecord.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class Q_SQL_EXPORT QSqlIndex : public QSqlRecord
{
 public:
   QSqlIndex(const QString &cursorName = QString(), const QString &name = QString());
   QSqlIndex(const QSqlIndex &other);
   ~QSqlIndex();
   QSqlIndex &operator=(const QSqlIndex &other);
   void setCursorName(const QString &cursorName);
   inline QString cursorName() const {
      return cursor;
   }
   void setName(const QString &name);
   inline QString name() const {
      return nm;
   }

   void append(const QSqlField &field);
   void append(const QSqlField &field, bool desc);

   bool isDescending(int i) const;
   void setDescending(int i, bool desc);

 private:
   QString createField(int i, const QString &prefix, bool verbose) const;
   QString cursor;
   QString nm;
   QList<bool> sorts;
};

QT_END_NAMESPACE

#endif // QSQLINDEX_H
