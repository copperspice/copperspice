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

#include <qsqldriver.h>

#include <qdatetime.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqldriver_p.h>

static QString prepareIdentifier(const QString &identifier, QSqlDriver::IdentifierType type, const QSqlDriver *driver)
{
   Q_ASSERT( driver != NULL );
   QString ret = identifier;

   if (! driver->isIdentifierEscaped(identifier, type)) {
      ret = driver->escapeIdentifier(identifier, type);
   }
   return ret;
}

QSqlDriverPrivate::~QSqlDriverPrivate()
{
}

QSqlDriver::QSqlDriver(QObject *parent)
   : QObject(parent), d_ptr(new QSqlDriverPrivate)
{
}
QSqlDriver::QSqlDriver(QSqlDriverPrivate &obj, QObject *parent)
   : QObject(parent), d_ptr(&obj)
{
}

QSqlDriver::~QSqlDriver()
{
}

bool QSqlDriver::isOpen() const
{
   return d_func()->isOpen;
}

bool QSqlDriver::isOpenError() const
{
   return d_func()->isOpenError;
}

void QSqlDriver::setOpen(bool open)
{
   d_func()->isOpen = open;
}

void QSqlDriver::setOpenError(bool error)
{
   d_func()->isOpenError = error;
   if (error) {
      d_func()->isOpen = false;
   }
}

bool QSqlDriver::beginTransaction()
{
   return false;
}

bool QSqlDriver::commitTransaction()
{
   return false;
}

bool QSqlDriver::rollbackTransaction()
{
   return false;
}

void QSqlDriver::setLastError(const QSqlError &error)
{
   d_func()->error = error;
}

QSqlError QSqlDriver::lastError() const
{
   return d_func()->error;
}

QStringList QSqlDriver::tables(QSql::TableType) const
{
   return QStringList();
}

QSqlIndex QSqlDriver::primaryIndex(const QString &) const
{
   return QSqlIndex();
}

QSqlRecord QSqlDriver::record(const QString &) const
{
   return QSqlRecord();
}

QString QSqlDriver::escapeIdentifier(const QString &identifier, IdentifierType) const
{
   return identifier;
}

bool QSqlDriver::isIdentifierEscaped(const QString &identifier, IdentifierType type) const
{
   (void) type;

   return identifier.size() > 2
      && identifier.startsWith(QChar('"'))     //left delimited
      && identifier.endsWith(QChar('"'));      //right delimited
}

QString QSqlDriver::stripDelimiters(const QString &identifier, IdentifierType type) const
{
   QString ret;

   if (isIdentifierEscaped(identifier, type)) {
      ret = identifier.mid(1);
      ret.chop(1);
   } else {
      ret = identifier;
   }

   return ret;
}

QString QSqlDriver::sqlStatement(StatementType type, const QString &tableName,
                  const QSqlRecord &rec, bool preparedStatement) const
{
   int i;
   QString s;

   switch (type) {
      case SelectStatement:
         for (i = 0; i < rec.count(); ++i) {
            if (rec.isGenerated(i)) {
               s.append(prepareIdentifier(rec.fieldName(i), QSqlDriver::FieldName, this)).append(", ");
            }
         }

         if (s.isEmpty()) {
            return s;
         }

         s.chop(2);
         s.prepend("SELECT ").append(" FROM ").append(tableName);
         break;

      case WhereStatement: {
         const QString tableNamePrefix = tableName.isEmpty()
            ? QString()
            : prepareIdentifier(tableName, QSqlDriver::TableName, this) + QChar('.');

         for (int i = 0; i < rec.count(); ++i) {
            s.append(i ? QString(" AND ") : QString("WHERE "));
            s.append(tableNamePrefix);
            s.append(prepareIdentifier(rec.fieldName(i), QSqlDriver::FieldName, this));

            if (rec.isNull(i)) {
               s.append(QString(" IS NULL"));

            } else if (preparedStatement) {
               s.append(QString(" = ?"));

            } else {
               s.append(QString(" = ")).append(formatValue(rec.field(i)));
            }
         }
         break;
      }

      case UpdateStatement:
         s.append("UPDATE " + tableName + " SET ");

         for (i = 0; i < rec.count(); ++i) {
            if (! rec.isGenerated(i)) {
               continue;
            }

            s.append(prepareIdentifier(rec.fieldName(i), QSqlDriver::FieldName, this)).append(QChar('='));
            if (preparedStatement) {
               s.append(QChar('?'));
            } else {
               s.append(formatValue(rec.field(i)));
            }
            s.append(QString(", "));
         }

         if (s.endsWith(QString(", "))) {
            s.chop(2);
         } else {
            s.clear();
         }
         break;

      case DeleteStatement:
         s.append(QString("DELETE FROM ")).append(tableName);
         break;

      case InsertStatement: {
         s.append(QString("INSERT INTO ")).append(tableName).append(QString(" ("));

         QString vals;

         for (i = 0; i < rec.count(); ++i) {
            if (! rec.isGenerated(i)) {
               continue;
            }
            s.append(prepareIdentifier(rec.fieldName(i), QSqlDriver::FieldName, this)).append(QString(", "));
            if (preparedStatement) {
               vals.append(QChar('?'));
            } else {
               vals.append(formatValue(rec.field(i)));
            }
            vals.append(QString(", "));
         }

         if (vals.isEmpty()) {
            s.clear();

         } else {
            vals.chop(2);                // remove trailing comma

            s.chop(2);
            s.append(") VALUES (").append(vals).append(QChar(')'));
         }
         break;
      }
   }
   return s;
}

QString QSqlDriver::formatValue(const QSqlField &field, bool trimStrings) const
{
   const QString nullTxt("NULL");

   QString r;
   if (! field.isValid()) {
      r = nullTxt;

   } else {
      switch (field.type()) {
         case QVariant::Int:
         case QVariant::UInt:
            if (field.value().type() == QVariant::Bool) {
               r = field.value().toBool() ? QString("1") : QString("0");
            } else {
               r = field.value().toString();
            }
            break;

         case QVariant::Date:
            if (field.value().toDate().isValid())
               r = QChar('\'') + field.value().toDate().toString(Qt::ISODate) + QChar('\'');
            else {
               r = nullTxt;
            }
            break;

         case QVariant::Time:
            if (field.value().toTime().isValid())
               r =  QChar('\'') + field.value().toTime().toString(Qt::ISODate) + QChar('\'');
            else {
               r = nullTxt;
            }
            break;

         case QVariant::DateTime:
            if (field.value().toDateTime().isValid())
               r = QChar('\'') + field.value().toDateTime().toString(Qt::ISODate) + QChar('\'');
            else {
               r = nullTxt;
            }
            break;

         case QVariant::String:
         case QVariant::Char: {
            QString result = field.value().toString();

            if (trimStrings) {
               int end = result.length();
               while (end && result.at(end - 1).isSpace()) {
                  // skip white space from end
                  end--;
               }
               result.truncate(end);
            }

            /* escape the "'" character */
            result.replace('\'', "''");
            r = QChar('\'') + result + QChar('\'');
            break;
         }

         case QVariant::Bool:
            r = QString::number(field.value().toBool());
            break;

         case QVariant::ByteArray : {
            if (hasFeature(BLOB)) {
               QByteArray ba = field.value().toByteArray();
               QString res;

               static const char hexchars[] = "0123456789abcdef";

               for (int i = 0; i < ba.size(); ++i) {
                  uchar s = (uchar) ba[i];
                  res += QChar(hexchars[s >> 4]);
                  res += QChar(hexchars[s & 0x0f]);
               }

               r = QChar('\'') + res +  QChar('\'');
               break;
            }

            [[fallthrough]];
         }

         default:
            r = field.value().toString();
            break;
      }
   }

   return r;
}

QVariant QSqlDriver::handle() const
{
   return QVariant();
}

bool QSqlDriver::subscribeToNotification(const QString &name)
{
   (void) name;

   return false;
}

bool QSqlDriver::unsubscribeFromNotification(const QString &name)
{
   (void) name;

   return false;
}

QStringList QSqlDriver::subscribedToNotifications() const
{
   return QStringList();
}

void QSqlDriver::setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy)
{
   d_func()->precisionPolicy = precisionPolicy;
}

QSql::NumericalPrecisionPolicy QSqlDriver::numericalPrecisionPolicy() const
{
   return d_func()->precisionPolicy;
}

QSqlDriver::DbmsType QSqlDriver::dbmsType() const
{
   return d_func()->dbmsType;
}
bool QSqlDriver::cancelQuery()
{
   return false;
}

