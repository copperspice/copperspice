/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QSQLRECORD_H
#define QSQLRECORD_H

#include <qstring.h>

class QDebug;
class QSqlField;
class QStringList;
class QVariant;
class QSqlRecordPrivate;

class Q_SQL_EXPORT QSqlRecord
{
 public:
   QSqlRecord();
   QSqlRecord(const QSqlRecord &other);
   QSqlRecord &operator=(const QSqlRecord &other);
   ~QSqlRecord();

   bool operator==(const QSqlRecord &other) const;
   inline bool operator!=(const QSqlRecord &other) const {
      return !operator==(other);
   }

   QVariant value(int i) const;
   QVariant value(const QString &name) const;
   void setValue(int i, const QVariant &val);
   void setValue(const QString &name, const QVariant &val);

   void setNull(int i);
   void setNull(const QString &name);
   bool isNull(int i) const;
   bool isNull(const QString &name) const;

   int indexOf(const QString &name) const;
   QString fieldName(int i) const;

   QSqlField field(int i) const;
   QSqlField field(const QString &name) const;

   bool isGenerated(int i) const;
   bool isGenerated(const QString &name) const;
   void setGenerated(const QString &name, bool generated);
   void setGenerated(int i, bool generated);

   void append(const QSqlField &field);
   void replace(int pos, const QSqlField &field);
   void insert(int pos, const QSqlField &field);
   void remove(int pos);

   bool isEmpty() const;
   bool contains(const QString &name) const;
   void clear();
   void clearValues();
   int count() const;

 private:
   void detach();
   QSqlRecordPrivate *d;
};

Q_SQL_EXPORT QDebug operator<<(QDebug, const QSqlRecord &);

#endif // QSQLRECORD_H
