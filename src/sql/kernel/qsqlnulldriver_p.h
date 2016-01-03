/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSQLNULLDRIVER_P_H
#define QSQLNULLDRIVER_P_H

#include <QtCore/qvariant.h>
#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlerror.h>
#include <QtSql/qsqlresult.h>

QT_BEGIN_NAMESPACE

class QSqlNullResult : public QSqlResult
{

 public:
   inline explicit QSqlNullResult(const QSqlDriver *d): QSqlResult(d) {
      QSqlResult::setLastError(QSqlError(QLatin1String("Driver not loaded"),
                                         QLatin1String("Driver not loaded"), QSqlError::ConnectionError));
   }

 protected:
   inline QVariant data(int) {
      return QVariant();
   }
   inline bool reset (const QString &) {
      return false;
   }
   inline bool fetch(int) {
      return false;
   }
   inline bool fetchFirst() {
      return false;
   }
   inline bool fetchLast() {
      return false;
   }
   inline bool isNull(int) {
      return false;
   }
   inline int size()  {
      return -1;
   }
   inline int numRowsAffected() {
      return 0;
   }

   inline void setAt(int) {}
   inline void setActive(bool) {}
   inline void setLastError(const QSqlError &) {}
   inline void setQuery(const QString &) {}
   inline void setSelect(bool) {}
   inline void setForwardOnly(bool) {}

   inline bool exec() {
      return false;
   }
   inline bool prepare(const QString &) {
      return false;
   }
   inline bool savePrepare(const QString &) {
      return false;
   }
   inline void bindValue(int, const QVariant &, QSql::ParamType) {}
   inline void bindValue(const QString &, const QVariant &, QSql::ParamType) {}
};

class QSqlNullDriver : public QSqlDriver
{
 public:
   inline QSqlNullDriver(): QSqlDriver() {
      QSqlDriver::setLastError(QSqlError(QLatin1String("Driver not loaded"),
                                         QLatin1String("Driver not loaded"), QSqlError::ConnectionError));
   }

   inline bool hasFeature(DriverFeature) const {
      return false;
   }
   inline bool open(const QString &, const QString &, const QString &, const QString &, int, const QString &) {
      return false;
   }

   inline void close() {}
   inline QSqlResult *createResult() const {
      return new QSqlNullResult(this);
   }

 protected:
   inline void setOpen(bool) {}
   inline void setOpenError(bool) {}
   inline void setLastError(const QSqlError &) {}
};

QT_END_NAMESPACE

#endif // QSQLNULLDRIVER_P_H
