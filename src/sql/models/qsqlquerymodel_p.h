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

#ifndef QSQLQUERYMODEL_P_H
#define QSQLQUERYMODEL_P_H

#include <qabstractitemmodel_p.h>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <qhash.h>
#include <qvarlengtharray.h>
#include <qvector.h>


class QSqlQueryModelPrivate: public QAbstractItemModelPrivate
{
   Q_DECLARE_PUBLIC(QSqlQueryModel)

 public:
   QSqlQueryModelPrivate() : atEnd(false), nestedResetLevel(0) {}
   ~QSqlQueryModelPrivate();

   void prefetch(int);
   void initColOffsets(int size);
   int columnInQuery(int modelColumn) const;

   mutable QSqlQuery query;
   mutable QSqlError error;
   QModelIndex bottom;
   QSqlRecord rec;
   uint atEnd : 1;
   QVector<QHash<int, QVariant>> headers;
   QVarLengthArray<int, 56> colOffsets; // used to calculate indexInQuery of columns
   int nestedResetLevel;
};

class QSqlQueryModelSql
{
 public:
   // SQL keywords
   const static QString as() {
      return QString("AS");
   }
   const static QString asc() {
      return QString("ASC");
   }
   const static QString comma() {
      return QString(",");
   }
   const static QString desc() {
      return QString("DESC");
   }
   const static QString eq() {
      return QString("=");
   }

   // "and" is a C++ keyword
   const static QString et() {
      return QString("AND");
   }
   const static QString from() {
      return QString("FROM");
   }
   const static QString leftJoin() {
      return QString("LEFT JOIN");
   }
   const static QString on() {
      return QString("ON");
   }
   const static QString orderBy() {
      return QString("ORDER BY");
   }
   const static QString parenClose() {
      return QString(")");
   }
   const static QString parenOpen() {
      return QString("(");
   }
   const static QString select() {
      return QString("SELECT");
   }
   const static QString sp() {
      return QString(" ");
   }
   const static QString where() {
      return QString("WHERE");
   }

   // Build expressions based on key words
   const static QString as(const QString &a, const QString &b) {
      return b.isEmpty() ? a : concat(concat(a, as()), b);
   }
   const static QString asc(const QString &s) {
      return concat(s, asc());
   }
   const static QString comma(const QString &a, const QString &b) {
      return a.isEmpty() ? b : b.isEmpty() ? a : QString(a).append(comma()).append(b);
   }
   const static QString concat(const QString &a, const QString &b) {
      return a.isEmpty() ? b : b.isEmpty() ? a : QString(a).append(sp()).append(b);
   }
   const static QString desc(const QString &s) {
      return concat(s, desc());
   }
   const static QString eq(const QString &a, const QString &b) {
      return QString(a).append(eq()).append(b);
   }
   const static QString et(const QString &a, const QString &b) {
      return a.isEmpty() ? b : b.isEmpty() ? a : concat(concat(a, et()), b);
   }
   const static QString from(const QString &s) {
      return concat(from(), s);
   }
   const static QString leftJoin(const QString &s) {
      return concat(leftJoin(), s);
   }
   const static QString on(const QString &s) {
      return concat(on(), s);
   }
   const static QString orderBy(const QString &s) {
      return s.isEmpty() ? s : concat(orderBy(), s);
   }
   const static QString paren(const QString &s) {
      return s.isEmpty() ? s : parenOpen() + s + parenClose();
   }
   const static QString select(const QString &s) {
      return concat(select(), s);
   }
   const static QString where(const QString &s) {
      return s.isEmpty() ? s : concat(where(), s);
   }
};

#endif // QSQLQUERYMODEL_P_H
