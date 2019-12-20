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
QSqlDriver::QSqlDriver(QSqlDriverPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
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

/*!
    Returns the primary index for table \a tableName. Returns an empty
    QSqlIndex if the table doesn't have a primary index. The default
    implementation returns an empty index.
*/

QSqlIndex QSqlDriver::primaryIndex(const QString &) const
{
   return QSqlIndex();
}


/*!
    Returns a QSqlRecord populated with the names of the fields in
    table \a tableName. If no such table exists, an empty record is
    returned. The default implementation returns an empty record.
*/

QSqlRecord QSqlDriver::record(const QString & /* tableName */) const
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
      && identifier.startsWith(QLatin1Char('"')) //left delimited
      && identifier.endsWith(QLatin1Char('"')); //right delimited
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

QString QSqlDriver::sqlStatement(StatementType type, const QString &tableName, const QSqlRecord &rec, bool preparedStatement) const
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
            : prepareIdentifier(tableName, QSqlDriver::TableName, this) + QLatin1Char('.');

         for (int i = 0; i < rec.count(); ++i) {
            s.append(i ? QString(" AND ") : QString("WHERE "));
            s.append(tableNamePrefix);
            s.append(prepareIdentifier(rec.fieldName(i), QSqlDriver::FieldName, this));

            if (rec.isNull(i)) {
               s.append(QLatin1String(" IS NULL"));
            } else if (preparedStatement) {
               s.append(QLatin1String(" = ?"));
            } else {
               s.append(QLatin1String(" = ")).append(formatValue(rec.field(i)));
            }
         }
         break;
      }

      case UpdateStatement:
         s.append(QLatin1String("UPDATE ")).append(tableName).append(
            QLatin1String(" SET "));
         for (i = 0; i < rec.count(); ++i) {
            if (!rec.isGenerated(i)) {
               continue;
            }
            s.append(prepareIdentifier(rec.fieldName(i), QSqlDriver::FieldName, this)).append(QLatin1Char('='));
            if (preparedStatement) {
               s.append(QLatin1Char('?'));
            } else {
               s.append(formatValue(rec.field(i)));
            }
            s.append(QLatin1String(", "));
         }
         if (s.endsWith(QLatin1String(", "))) {
            s.chop(2);
         } else {
            s.clear();
         }
         break;

      case DeleteStatement:
         s.append(QLatin1String("DELETE FROM ")).append(tableName);
         break;

      case InsertStatement: {
         s.append(QLatin1String("INSERT INTO ")).append(tableName).append(QLatin1String(" ("));

         QString vals;

         for (i = 0; i < rec.count(); ++i) {
            if (! rec.isGenerated(i)) {
               continue;
            }
            s.append(prepareIdentifier(rec.fieldName(i), QSqlDriver::FieldName, this)).append(QLatin1String(", "));
            if (preparedStatement) {
               vals.append(QLatin1Char('?'));
            } else {
               vals.append(formatValue(rec.field(i)));
            }
            vals.append(QLatin1String(", "));
         }

         if (vals.isEmpty()) {
            s.clear();

         } else {
            vals.chop(2);                // remove trailing comma

            s.chop(2);
            s.append(") VALUES (").append(vals).append(QLatin1Char(')'));
         }
         break;
      }
   }
   return s;
}

QString QSqlDriver::formatValue(const QSqlField &field, bool trimStrings) const
{
   const QLatin1String nullTxt("NULL");

   QString r;
   if (field.isNull()) {
      r = nullTxt;

   } else {
      switch (field.type()) {
         case QVariant::Int:
         case QVariant::UInt:
            if (field.value().type() == QVariant::Bool) {
               r = field.value().toBool() ? QLatin1String("1") : QLatin1String("0");
            } else {
               r = field.value().toString();
            }
            break;

#ifndef QT_NO_DATESTRING
         case QVariant::Date:
            if (field.value().toDate().isValid())
               r = QLatin1Char('\'') + field.value().toDate().toString(Qt::ISODate) + QLatin1Char('\'');
            else {
               r = nullTxt;
            }
            break;
         case QVariant::Time:
            if (field.value().toTime().isValid())
               r =  QLatin1Char('\'') + field.value().toTime().toString(Qt::ISODate)
                  + QLatin1Char('\'');
            else {
               r = nullTxt;
            }
            break;
         case QVariant::DateTime:
            if (field.value().toDateTime().isValid())
               r = QLatin1Char('\'') +
                  field.value().toDateTime().toString(Qt::ISODate) + QLatin1Char('\'');
            else {
               r = nullTxt;
            }
            break;
#endif
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

