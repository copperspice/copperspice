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

#ifndef QSQLNULLDRIVER_P_H
#define QSQLNULLDRIVER_P_H

#include <qvariant.h>
#include <qsqldriver.h>
#include <qsqlerror.h>
#include <qsqlresult.h>

class QSqlNullResult : public QSqlResult
{

 public:
   explicit QSqlNullResult(const QSqlDriver *d): QSqlResult(d) {
      QSqlResult::setLastError(QSqlError(QLatin1String("Driver not loaded"),
            QLatin1String("Driver not loaded"), QSqlError::ConnectionError));
   }

 protected:
   QVariant data(int) override {
      return QVariant();
   }

   bool reset (const QString &) override {
      return false;
   }

   bool fetch(int) override {
      return false;
   }

   bool fetchFirst() override {
      return false;
   }

   bool fetchLast() override {
      return false;
   }

   bool isNull(int) override {
      return false;
   }

   int size()  override {
      return -1;
   }

   int numRowsAffected() override {
      return 0;
   }

   void setAt(int) override {
   }

   void setActive(bool) override {
   }

   void setLastError(const QSqlError &) override {
   }

   void setQuery(const QString &) override {
   }

   void setSelect(bool) override {
   }

   void setForwardOnly(bool) override {
   }

   bool exec() override {
      return false;
   }

   bool prepare(const QString &) override {
      return false;
   }

   bool savePrepare(const QString &) override {
      return false;
   }

   void bindValue(int, const QVariant &, QSql::ParamType) override {
   }

   void bindValue(const QString &, const QVariant &, QSql::ParamType) override {
   }
};

class QSqlNullDriver : public QSqlDriver
{
 public:
   QSqlNullDriver(): QSqlDriver() {
      QSqlDriver::setLastError(QSqlError(QLatin1String("Driver not loaded"),
            QLatin1String("Driver not loaded"), QSqlError::ConnectionError));
   }

   bool hasFeature(DriverFeature) const override {
      return false;
   }

   bool open(const QString &, const QString &, const QString &, const QString &, int, const QString &) override {
      return false;
   }

   void close() override {
   }

   QSqlResult *createResult() const override {
      return new QSqlNullResult(this);
   }

 protected:
   void setOpen(bool) override {
   }

   void setOpenError(bool) override {
   }

   void setLastError(const QSqlError &) override {
   }
};

#endif // QSQLNULLDRIVER_P_H
