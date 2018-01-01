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

#include <qsqldriver.h>

#include <qdatetime.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>

QT_BEGIN_NAMESPACE

static QString prepareIdentifier(const QString &identifier, QSqlDriver::IdentifierType type, const QSqlDriver *driver)
{
   Q_ASSERT( driver != NULL );
   QString ret = identifier;

   if (!driver->isIdentifierEscaped(identifier, type)) {
      ret = driver->escapeIdentifier(identifier, type);
   }
   return ret;
}

class QSqlDriverPrivate
{

 public:
   QSqlDriverPrivate();
   virtual ~QSqlDriverPrivate();

 public:
   // @CHECK: this member is never used. It was named q, which expanded to q_func().
   QSqlDriver *q_func();
   uint isOpen : 1;
   uint isOpenError : 1;
   QSqlError error;
   QSql::NumericalPrecisionPolicy precisionPolicy;
};

inline QSqlDriverPrivate::QSqlDriverPrivate()
   : isOpen(false), isOpenError(false), precisionPolicy(QSql::LowPrecisionDouble)
{
}

QSqlDriverPrivate::~QSqlDriverPrivate()
{
}

/*!
    \class QSqlDriver
    \brief The QSqlDriver class is an abstract base class for accessing
    specific SQL databases.

    \ingroup database
    \inmodule QtSql

    This class should not be used directly. Use QSqlDatabase instead.

    If you want to create your own SQL drivers, you can subclass this
    class and reimplement its pure virtual functions and those
    virtual functions that you need. See \l{How to Write Your Own
    Database Driver} for more information.

    \sa QSqlDatabase, QSqlResult
*/

/*!
    Constructs a new driver with the given \a parent.
*/

QSqlDriver::QSqlDriver(QObject *parent)
   : QObject(parent), d_ptr(new QSqlDriverPrivate)
{
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlDriver::~QSqlDriver()
{
}

/*!
    \since 4.4

    \fn QSqlDriver::notification(const QString &name)

    This signal is emitted when the database posts an event notification
    that the driver subscribes to. \a name identifies the event notification.

    \sa subscribeToNotification()
*/

/*!
    \fn bool QSqlDriver::open(const QString &db, const QString &user, const QString& password,
                              const QString &host, int port, const QString &options)

    Derived classes must reimplement this pure virtual function to
    open a database connection on database \a db, using user name \a
    user, password \a password, host \a host, port \a port and
    connection options \a options.

    The function must return true on success and false on failure.

    \sa setOpen()
*/

/*!
    \fn bool QSqlDriver::close()

    Derived classes must reimplement this pure virtual function in
    order to close the database connection. Return true on success,
    false on failure.

    \sa open(), setOpen()
*/

/*!
    \fn QSqlResult *QSqlDriver::createResult() const

    Creates an empty SQL result on the database. Derived classes must
    reimplement this function and return a QSqlResult object
    appropriate for their database to the caller.
*/

/*!
    Returns true if the database connection is open; otherwise returns
    false.
*/

bool QSqlDriver::isOpen() const
{
   return d_func()->isOpen;
}

/*!
    Returns true if the there was an error opening the database
    connection; otherwise returns false.
*/

bool QSqlDriver::isOpenError() const
{
   return d_func()->isOpenError;
}

/*!
    \enum QSqlDriver::DriverFeature

    This enum contains a list of features a driver might support. Use
    hasFeature() to query whether a feature is supported or not.

    \value Transactions  Whether the driver supports SQL transactions.
    \value QuerySize  Whether the database is capable of reporting the size
    of a query. Note that some databases do not support returning the size
    (i.e. number of rows returned) of a query, in which case
    QSqlQuery::size() will return -1.
    \value BLOB  Whether the driver supports Binary Large Object fields.
    \value Unicode  Whether the driver supports Unicode strings if the
    database server does.
    \value PreparedQueries  Whether the driver supports prepared query execution.
    \value NamedPlaceholders  Whether the driver supports the use of named placeholders.
    \value PositionalPlaceholders  Whether the driver supports the use of positional placeholders.
    \value LastInsertId  Whether the driver supports returning the Id of the last touched row.
    \value BatchOperations  Whether the driver supports batched operations, see QSqlQuery::execBatch()
    \value SimpleLocking  Whether the driver disallows a write lock on a table while other queries have a read lock on it.
    \value LowPrecisionNumbers  Whether the driver allows fetching numerical values with low precision.
    \value EventNotifications Whether the driver supports database event notifications.
    \value FinishQuery Whether the driver can do any low-level resource cleanup when QSqlQuery::finish() is called.
    \value MultipleResultSets Whether the driver can access multiple result sets returned from batched statements or stored procedures.

    More information about supported features can be found in the
    \l{sql-driver.html}{Qt SQL driver} documentation.

    \sa hasFeature()
*/

/*!
    \enum QSqlDriver::StatementType

    This enum contains a list of SQL statement (or clause) types the
    driver can create.

    \value WhereStatement  An SQL \c WHERE statement (e.g., \c{WHERE f = 5}).
    \value SelectStatement An SQL \c SELECT statement (e.g., \c{SELECT f FROM t}).
    \value UpdateStatement An SQL \c UPDATE statement (e.g., \c{UPDATE TABLE t set f = 1}).
    \value InsertStatement An SQL \c INSERT statement (e.g., \c{INSERT INTO t (f) values (1)}).
    \value DeleteStatement An SQL \c DELETE statement (e.g., \c{DELETE FROM t}).

    \sa sqlStatement()
*/

/*!
    \enum QSqlDriver::IdentifierType

    This enum contains a list of SQL identifier types.

    \value FieldName A SQL field name
    \value TableName A SQL table name
*/

/*!
    \fn bool QSqlDriver::hasFeature(DriverFeature feature) const

    Returns true if the driver supports feature \a feature; otherwise
    returns false.

    Note that some databases need to be open() before this can be
    determined.

    \sa DriverFeature
*/

/*!
    This function sets the open state of the database to \a open.
    Derived classes can use this function to report the status of
    open().

    \sa open(), setOpenError()
*/

void QSqlDriver::setOpen(bool open)
{
   d_func()->isOpen = open;
}

/*!
    This function sets the open error state of the database to \a
    error. Derived classes can use this function to report the status
    of open(). Note that if \a error is true the open state of the
    database is set to closed (i.e., isOpen() returns false).

    \sa open(), setOpen()
*/

void QSqlDriver::setOpenError(bool error)
{
   d_func()->isOpenError = error;
   if (error) {
      d_func()->isOpen = false;
   }
}

/*!
    This function is called to begin a transaction. If successful,
    return true, otherwise return false. The default implementation
    does nothing and returns false.

    \sa commitTransaction(), rollbackTransaction()
*/

bool QSqlDriver::beginTransaction()
{
   return false;
}

/*!
    This function is called to commit a transaction. If successful,
    return true, otherwise return false. The default implementation
    does nothing and returns false.

    \sa beginTransaction(), rollbackTransaction()
*/

bool QSqlDriver::commitTransaction()
{
   return false;
}

/*!
    This function is called to rollback a transaction. If successful,
    return true, otherwise return false. The default implementation
    does nothing and returns false.

    \sa beginTransaction(), commitTransaction()
*/

bool QSqlDriver::rollbackTransaction()
{
   return false;
}

/*!
    This function is used to set the value of the last error, \a error,
    that occurred on the database.

    \sa lastError()
*/

void QSqlDriver::setLastError(const QSqlError &error)
{
   d_func()->error = error;
}

/*!
    Returns a QSqlError object which contains information about the
    last error that occurred on the database.
*/

QSqlError QSqlDriver::lastError() const
{
   return d_func()->error;
}

/*!
    Returns a list of the names of the tables in the database. The
    default implementation returns an empty list.

    The \a tableType argument describes what types of tables
    should be returned. Due to binary compatibility, the string
    contains the value of the enum QSql::TableTypes as text.
    An empty string should be treated as QSql::Tables for
    backward compatibility.
*/

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
   bool result = this->isIdentifierEscapedImplementation(identifier, type);
   return result;
}

QString QSqlDriver::stripDelimiters(const QString &identifier, IdentifierType type) const
{
   QString result = this->stripDelimitersImplementation(identifier, type);
   return result;
}

QString QSqlDriver::sqlStatement(StatementType type, const QString &tableName,
                                 const QSqlRecord &rec, bool preparedStatement) const
{
   int i;
   QString s;
   s.reserve(128);
   switch (type) {
      case SelectStatement:
         for (i = 0; i < rec.count(); ++i) {
            if (rec.isGenerated(i)) {
               s.append(prepareIdentifier(rec.fieldName(i), QSqlDriver::FieldName, this)).append(QLatin1String(", "));
            }
         }
         if (s.isEmpty()) {
            return s;
         }
         s.chop(2);
         s.prepend(QLatin1String("SELECT ")).append(QLatin1String(" FROM ")).append(tableName);
         break;
      case WhereStatement:
         if (preparedStatement) {
            for (int i = 0; i < rec.count(); ++i) {
               s.append(prepareIdentifier(rec.fieldName(i), FieldName, this));
               if (rec.isNull(i)) {
                  s.append(QLatin1String(" IS NULL"));
               } else {
                  s.append(QLatin1String(" = ?"));
               }
               s.append(QLatin1String(" AND "));
            }
         } else {
            for (i = 0; i < rec.count(); ++i) {
               s.append(prepareIdentifier(rec.fieldName(i), QSqlDriver::FieldName, this));
               QString val = formatValue(rec.field(i));
               if (val == QLatin1String("NULL")) {
                  s.append(QLatin1String(" IS NULL"));
               } else {
                  s.append(QLatin1String(" = ")).append(val);
               }
               s.append(QLatin1String(" AND "));
            }
         }
         if (!s.isEmpty()) {
            s.prepend(QLatin1String("WHERE "));
            s.chop(5); // remove tailing AND
         }
         break;
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
            if (!rec.isGenerated(i)) {
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
            vals.chop(2); // remove trailing comma
            s[s.length() - 2] = QLatin1Char(')');
            s.append(QLatin1String("VALUES (")).append(vals).append(QLatin1Char(')'));
         }
         break;
      }
   }
   return s;
}

/*!
    Returns a string representation of the \a field value for the
    database. This is used, for example, when constructing INSERT and
    UPDATE statements.

    The default implementation returns the value formatted as a string
    according to the following rules:

    \list

    \i If \a field is character data, the value is returned enclosed
    in single quotation marks, which is appropriate for many SQL
    databases. Any embedded single-quote characters are escaped
    (replaced with two single-quote characters). If \a trimStrings is
    true (the default is false), all trailing whitespace is trimmed
    from the field.

    \i If \a field is date/time data, the value is formatted in ISO
    format and enclosed in single quotation marks. If the date/time
    data is invalid, "NULL" is returned.

    \i If \a field is \link QByteArray bytearray\endlink data, and the
    driver can edit binary fields, the value is formatted as a
    hexadecimal string.

    \i For any other field type, toString() is called on its value
    and the result of this is returned.

    \endlist

    \sa QVariant::toString()

*/
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
               r = QLatin1Char('\'') + field.value().toDate().toString(Qt::ISODate)
                   + QLatin1Char('\'');
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
               while (end && result.at(end - 1).isSpace()) { /* skip white space from end */
                  end--;
               }
               result.truncate(end);
            }
            /* escape the "'" character */
            result.replace(QLatin1Char('\''), QLatin1String("''"));
            r = QLatin1Char('\'') + result + QLatin1Char('\'');
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
                  res += QLatin1Char(hexchars[s >> 4]);
                  res += QLatin1Char(hexchars[s & 0x0f]);
               }
               r = QLatin1Char('\'') + res +  QLatin1Char('\'');
               break;
            }
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
   bool result = this->subscribeToNotificationImplementation(name);
   return result;
}

bool QSqlDriver::unsubscribeFromNotification(const QString &name)
{
   bool result = this->unsubscribeFromNotificationImplementation(name);
   return result;
}

QStringList QSqlDriver::subscribedToNotifications() const
{
   QStringList result = this->subscribedToNotificationsImplementation();
   return result;
}

bool QSqlDriver::subscribeToNotificationImplementation(const QString &name)
{
   Q_UNUSED(name);
   return false;
}

bool QSqlDriver::unsubscribeFromNotificationImplementation(const QString &name)
{
   Q_UNUSED(name);
   return false;
}

QStringList QSqlDriver::subscribedToNotificationsImplementation() const
{
   return QStringList();
}

bool QSqlDriver::isIdentifierEscapedImplementation(const QString &identifier, IdentifierType type) const
{
   Q_UNUSED(type);
   return identifier.size() > 2
          && identifier.startsWith(QLatin1Char('"')) //left delimited
          && identifier.endsWith(QLatin1Char('"')); //right delimited
}

/*!
    \since 4.6

    This slot returns \a identifier with the leading and trailing delimiters removed,
    \a identifier can either be a tablename or field name, dependent on \a type.
    If \a identifier does not have leading and trailing delimiter characters, \a
    identifier is returned without modification.

    Because of binary compatibility constraints, the stripDelimiters() function
    (introduced in Qt 4.5) is not virtual.  Instead, stripDelimiters() will
    dynamically detect and call \e this slot.  It generally unnecessary
    to reimplement this slot.

    \sa stripDelimiters()
 */
QString QSqlDriver::stripDelimitersImplementation(const QString &identifier, IdentifierType type) const
{
   QString ret;
   if (this->isIdentifierEscaped(identifier, type)) {
      ret = identifier.mid(1);
      ret.chop(1);
   } else {
      ret = identifier;
   }
   return ret;
}

/*!
    \since 4.6

    Sets the default numerical precision policy used by queries created
    by this driver to \a precisionPolicy.

    Note: Setting the default precision policy to \a precisionPolicy
    doesn't affect any currently active queries.

    \sa QSql::NumericalPrecisionPolicy, numericalPrecisionPolicy(),
    QSqlQuery::setNumericalPrecisionPolicy(), QSqlQuery::numericalPrecisionPolicy()
*/
void QSqlDriver::setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy)
{
   d_func()->precisionPolicy = precisionPolicy;
}

/*!
    \since 4.6

    Returns the current default precision policy for the database connection.

    \sa QSql::NumericalPrecisionPolicy, setNumericalPrecisionPolicy(),
    QSqlQuery::numericalPrecisionPolicy(), QSqlQuery::setNumericalPrecisionPolicy()
*/
QSql::NumericalPrecisionPolicy QSqlDriver::numericalPrecisionPolicy() const
{
   return d_func()->precisionPolicy;
}

QT_END_NAMESPACE
