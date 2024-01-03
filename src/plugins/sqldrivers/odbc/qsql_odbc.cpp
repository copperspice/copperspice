/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qsql_odbc.h>
#include <qsqlrecord.h>

#if defined (Q_OS_WIN)
#include <qt_windows.h>
#endif

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qstringlist.h>
#include <qvarlengtharray.h>
#include <qvector.h>
#include <qmath.h>
#include <qdebug.h>
#include <qsqlquery.h>

#include <qsqldriver_p.h>
#include <qsqlresult_p.h>

// undefine this to prevent initial check of the ODBC driver
#define ODBC_CHECK_DRIVER

static const int COLNAMESIZE = 256;
static const SQLSMALLINT qParamType[4] = {
   SQL_PARAM_INPUT,
   SQL_PARAM_INPUT,
   SQL_PARAM_OUTPUT,
   SQL_PARAM_INPUT_OUTPUT
};

static inline QString fromSQLTCHAR(const QVarLengthArray<SQLTCHAR> &input, int size = -1)
{
   QString result;

   int realsize = qMin(size, input.size());

   if (realsize > 0 && input[realsize - 1] == 0) {
      --realsize;
   }

   switch (sizeof(SQLTCHAR)) {
      case 1:
         result = QString::fromUtf8((const char *)input.constData(), realsize);
         break;

      case 2:
         result = QString::fromUtf16((const char16_t *)input.constData(), realsize);
         break;

      case 4:
         // ucs-4
         for (SQLTCHAR ch : input) {
            result.append(QChar32(ch));
         }

         break;

      default:
         qCritical() << "sizeof(SQLTCHAR) is " << sizeof(SQLTCHAR) << "Unable to handle this data type";
   }

   return result;
}

static inline QVarLengthArray<SQLTCHAR> toSQLTCHAR(const QString &input)
{
   QVarLengthArray<SQLTCHAR> result;

   switch (sizeof(SQLTCHAR)) {
      case 1:
         // utf-8
         result.resize(input.size_storage());

         memcpy(result.data(), input.constData(), input.size_storage());
         break;

      case 2: {
         // utf-16
         QString16 tmp = input.toUtf16();
         result.resize(tmp.size_storage() * 2);

         memcpy(result.data(), tmp.constData(), tmp.size_storage() * 2);
         break;
      }

      case 4: {
         // ucs-4
         QVector<char32_t> tmp;

         for (QChar32 ch : input) {
            tmp.append(ch.unicode());
         }

         result.resize(tmp.size() * 4);

         memcpy(result.data(), tmp.constData(), tmp.size() * 4);
         break;
      }

      default:
         qCritical() << "sizeof(SQLTCHAR) is " << sizeof(SQLTCHAR) << "Unable to handle this data type";
   }

   result.append(0);    // make sure it is null terminated

   return result;
}

class QODBCDriverPrivate : public QSqlDriverPrivate
{
 public:
   enum DefaultCase {Lower, Mixed, Upper, Sensitive};

   QODBCDriverPrivate()
      : hEnv(nullptr), hDbc(nullptr), unicode(false), useSchema(false),
        isFreeTDSDriver(false), hasSQLFetchScroll(true), hasMultiResultSets(false),
        disconnectCount(0), datetime_precision(19), isQuoteInitialized(false), m_quote('"')
   {
   }

   SQLHANDLE hEnv;
   SQLHANDLE hDbc;

   bool unicode;
   bool useSchema;
   bool isFreeTDSDriver;
   bool hasSQLFetchScroll;
   bool hasMultiResultSets;

   int disconnectCount;
   int datetime_precision;

   bool checkDriver() const;
   void checkUnicode();
   void checkDBMS();
   void checkHasSQLFetchScroll();
   void checkHasMultiResults();
   void checkSchemaUsage();
   void checkDateTimePrecision();
   bool setConnectionOptions(const QString &connOpts);
   void splitTableQualifier(const QString &qualifier, QString &catalog, QString &schema, QString &table);

   DefaultCase defaultCase() const;

   QString adjustCase(const QString &) const;
   QChar quoteChar() const;

 private:
   mutable bool isQuoteInitialized;
   mutable QChar m_quote;
};

class QODBCResultPrivate : public QSqlResultPrivate
{
 public:
   QODBCResultPrivate(QODBCDriverPrivate *dpp)
      : hStmt(nullptr), useSchema(false), hasSQLFetchScroll(true), userForwardOnly(false), driverPrivate(dpp)
   {
      unicode           = dpp->unicode;
      useSchema         = dpp->useSchema;
      disconnectCount   = dpp->disconnectCount;
      hasSQLFetchScroll = dpp->hasSQLFetchScroll;
   }

   void clearValues() {
      fieldCache.fill(QVariant());
      fieldCacheIdx = 0;
   }

   SQLHANDLE dpEnv() const {
      return driverPrivate ? driverPrivate->hEnv : nullptr;
   }

   SQLHANDLE dpDbc() const {
      return driverPrivate ? driverPrivate->hDbc : nullptr;
   }

   SQLHANDLE hStmt;

   bool unicode;
   bool useSchema;
   bool hasSQLFetchScroll;
   bool userForwardOnly;

   int fieldCacheIdx;
   int disconnectCount;

   QSqlRecord rInf;
   QVector<QVariant> fieldCache;
   QODBCDriverPrivate *driverPrivate;

   bool isStmtHandleValid(const QSqlDriver *driver);
   void updateStmtHandleState(const QSqlDriver *driver);
};

bool QODBCResultPrivate::isStmtHandleValid(const QSqlDriver *driver)
{
   const QODBCDriver *odbcdriver = static_cast<const QODBCDriver *> (driver);
   const QODBCDriverPrivate *obj = odbcdriver->d_func();

   return disconnectCount == obj->disconnectCount;
}

void QODBCResultPrivate::updateStmtHandleState(const QSqlDriver *driver)
{
   const QODBCDriver *odbcdriver = static_cast<const QODBCDriver *> (driver);
   const QODBCDriverPrivate *obj = odbcdriver->d_func();

   disconnectCount = obj->disconnectCount;
}

static QString cs_warnHandle(int handleType, SQLHANDLE handle, int *nativeCode = nullptr)
{
   SQLINTEGER nativeCode_ = 0;
   SQLSMALLINT msgLen     = 0;

   SQLRETURN r = SQL_NO_DATA;

   SQLTCHAR state_[SQL_SQLSTATE_SIZE + 1];
   QVarLengthArray<SQLTCHAR> description_(SQL_MAX_MESSAGE_LENGTH);
   QString result;
   int i = 1;

   description_[0] = 0;
   do {
      r = SQLGetDiagRec(handleType, handle, i, state_, &nativeCode_, nullptr, 0, &msgLen);

      if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && msgLen > 0) {
         description_.resize(msgLen + 1);
      }

      r = SQLGetDiagRec(handleType, handle, i, state_, &nativeCode_, description_.data(),
            description_.size(), &msgLen);

      if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
         if (nativeCode) {
            *nativeCode = nativeCode_;
         }

         const QString tmpstore = fromSQLTCHAR(description_, msgLen);

         if (result != tmpstore) {
            if (! result.isEmpty()) {
               result += ' ';
            }
            result += tmpstore;
         }

      } else if (r == SQL_ERROR || r == SQL_INVALID_HANDLE) {
         return result;
      }

      ++i;

   } while (r != SQL_NO_DATA);

   return result;
}

static QString cs_warnODBC(const SQLHANDLE hStmt, const SQLHANDLE envHandle = nullptr,
      const SQLHANDLE pDbC = nullptr, int *nativeCode = nullptr)
{
   QString result;

   if (envHandle != nullptr) {
      result += cs_warnHandle(SQL_HANDLE_ENV, envHandle, nativeCode);
   }

   if (pDbC != nullptr) {
     const QString dMessage = cs_warnHandle(SQL_HANDLE_DBC, pDbC, nativeCode);

     if (! dMessage.isEmpty()) {
         if (! result.isEmpty()) {
            result += ' ';
         }

         result += dMessage;
      }
   }

   if (hStmt != nullptr) {
      const QString hMessage = cs_warnHandle(SQL_HANDLE_STMT, hStmt, nativeCode);

      if (! hMessage.isEmpty()) {
         if (! result.isEmpty()) {
            result += ' ';
         }

         result += hMessage;
      }
   }

   return result;
}

static QString cs_warnODBC(const QODBCResultPrivate *odbc, int *nativeCode = nullptr)
{
   return cs_warnODBC(odbc->hStmt, odbc->dpEnv(), odbc->dpDbc(), nativeCode);
}

static QString cs_warnODBC(const QODBCDriverPrivate *odbc, int *nativeCode = nullptr)
{
   return cs_warnODBC(nullptr, odbc->hEnv, odbc->hDbc, nativeCode);
}

static void qSqlWarning(const QString &message, const QODBCResultPrivate *odbc)
{
   qWarning() << message << " Error:" << cs_warnODBC(odbc);
}

static void qSqlWarning(const QString &message, const QODBCDriverPrivate *odbc)
{
   qWarning() << message << " Error:" << cs_warnODBC(odbc);
}

static void qSqlWarning(const QString &message, const SQLHANDLE hStmt)
{
   qWarning() << message << " Error:" << cs_warnODBC(hStmt);
}

static QSqlError qMakeError(const QString &err, QSqlError::ErrorType type, const QODBCResultPrivate *p)
{
   int nativeCode  = -1;
   QString message = cs_warnODBC(p, &nativeCode);

   return QSqlError("QODBC: " + err, message, type, QString::number(nativeCode));
}

static QSqlError qMakeError(const QString& err, QSqlError::ErrorType type, const QODBCDriverPrivate *p)
{
   int nativeCode = -1;
   QString message = cs_warnODBC(p, &nativeCode);

   return QSqlError("QODBC: " + err, cs_warnODBC(p), type, QString::number(nativeCode));
}

static QVariant::Type qDecodeODBCType(SQLSMALLINT sqltype, bool isSigned = true)
{
   QVariant::Type type = QVariant::Invalid;

   switch (sqltype) {
      case SQL_DECIMAL:
      case SQL_NUMERIC:
      case SQL_REAL:
      case SQL_FLOAT:
      case SQL_DOUBLE:
         type = QVariant::Double;
         break;

      case SQL_SMALLINT:
      case SQL_INTEGER:
      case SQL_BIT:
         type = isSigned ? QVariant::Int : QVariant::UInt;
         break;

      case SQL_TINYINT:
         type = QVariant::UInt;
         break;

      case SQL_BIGINT:
         type = isSigned ? QVariant::LongLong : QVariant::ULongLong;
         break;

      case SQL_BINARY:
      case SQL_VARBINARY:
      case SQL_LONGVARBINARY:
         type = QVariant::ByteArray;
         break;

      case SQL_DATE:
      case SQL_TYPE_DATE:
         type = QVariant::Date;
         break;

      case SQL_TIME:
      case SQL_TYPE_TIME:
         type = QVariant::Time;
         break;

      case SQL_TIMESTAMP:
      case SQL_TYPE_TIMESTAMP:
         type = QVariant::DateTime;
         break;

      case SQL_WCHAR:
      case SQL_WVARCHAR:
      case SQL_WLONGVARCHAR:
         type = QVariant::String;
         break;

      case SQL_CHAR:
      case SQL_VARCHAR:

#if (ODBCVER >= 0x0350)
      case SQL_GUID:
         [[fallthrough]];
#endif

      case SQL_LONGVARCHAR:
         type = QVariant::String;
         break;

      default:
         type = QVariant::ByteArray;
         break;
   }

   return type;
}

static QString qGetStringData(SQLHANDLE hStmt, int column, int colSize, bool unicode = false)
{
   QString fieldVal;
   SQLRETURN r = SQL_ERROR;
   SQLLEN lengthIndicator = 0;

   // NB colSize must be a multiple of 2 for unicode enabled DBs
   if (colSize <= 0) {
      colSize = 256;

   } else if (colSize > 65536) {
      // limit buffer size to 64 KB
      colSize = 65536;

   } else {
      // make sure there is room for more than the 0 termination
      ++colSize;
   }

   if (unicode) {
      r = SQLGetData(hStmt, column + 1, SQL_C_TCHAR, nullptr, 0, &lengthIndicator);

      if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && lengthIndicator > 0) {
         colSize = lengthIndicator / sizeof(SQLTCHAR) + 1;
      }

      QVarLengthArray<SQLTCHAR> buf(colSize);
      memset(buf.data(), 0, colSize * sizeof(SQLTCHAR));
      while (true) {
         r = SQLGetData(hStmt, column + 1, SQL_C_TCHAR, (SQLPOINTER)buf.data(),
               colSize * sizeof(SQLTCHAR), &lengthIndicator);

         if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
            if (lengthIndicator == SQL_NULL_DATA || lengthIndicator == SQL_NO_TOTAL) {
               fieldVal.clear();
               break;
            }

            // if SQL_SUCCESS_WITH_INFO is returned, indicating that
            // more data can be fetched, the length indicator does NOT
            // contain the number of bytes returned - it contains the
            // total number of bytes that CAN be fetched
            // colSize-1: remove 0 termination when there is more data to fetch

            int rSize = (r == SQL_SUCCESS_WITH_INFO) ? colSize : lengthIndicator / sizeof(SQLTCHAR);
            fieldVal += fromSQLTCHAR(buf, rSize);

            if (lengthIndicator < SQLLEN(colSize * sizeof(SQLTCHAR))) {
               // workaround for Drivermanagers that don't return SQL_NO_DATA
               break;
            }

         } else if (r == SQL_NO_DATA) {
            break;

         } else {
            qWarning() << "qGetStringData: Error while fetching data (" << cs_warnHandle(SQL_HANDLE_STMT, hStmt) << ')';
            fieldVal.clear();
            break;
         }
      }

   } else {
      r = SQLGetData(hStmt, column + 1, SQL_C_CHAR, nullptr, 0, &lengthIndicator);

      if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && lengthIndicator > 0) {
         colSize = lengthIndicator + 1;
      }

      QVarLengthArray<SQLCHAR> buf(colSize);

      while (true) {
         r = SQLGetData(hStmt, column + 1, SQL_C_CHAR, (SQLPOINTER)buf.data(), colSize, &lengthIndicator);

         if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
            if (lengthIndicator == SQL_NULL_DATA || lengthIndicator == SQL_NO_TOTAL) {
               fieldVal.clear();
               break;
            }

            // if SQL_SUCCESS_WITH_INFO is returned, indicating that
            // more data can be fetched, the length indicator does NOT
            // contain the number of bytes returned - it contains the
            // total number of bytes that CAN be fetched
            // colSize-1: remove 0 termination when there is more data to fetch

            int rSize = (r == SQL_SUCCESS_WITH_INFO) ? colSize : lengthIndicator;
            fieldVal += QString::fromUtf8((const char *)buf.constData(), rSize);

            if (lengthIndicator < SQLLEN(colSize)) {
               // workaround for Drivermanagers that do not return SQL_NO_DATA
               break;
            }

         } else if (r == SQL_NO_DATA) {
            break;

         } else {
            qWarning() << "qGetStringData: Error while fetching data (" << cs_warnHandle(SQL_HANDLE_STMT, hStmt) << ')';
            fieldVal.clear();
            break;
         }
      }
   }

   return fieldVal;
}

static QVariant qGetBinaryData(SQLHANDLE hStmt, int column)
{
   QByteArray fieldVal;
   SQLSMALLINT colNameLen;
   SQLSMALLINT colType;
   SQLULEN colSize;
   SQLSMALLINT colScale;
   SQLSMALLINT nullable;
   SQLLEN lengthIndicator = 0;
   SQLRETURN r = SQL_ERROR;

   QVarLengthArray<SQLTCHAR> colName(COLNAMESIZE);

   r = SQLDescribeCol(hStmt, column + 1, colName.data(), COLNAMESIZE, &colNameLen, &colType,
         &colSize, &colScale, &nullable);

   if (r != SQL_SUCCESS) {
      qWarning() << "qGetBinaryData: Unable to describe column" << column;
   }

   // SQLDescribeCol may return 0 if size cannot be determined
   if (! colSize) {
      colSize = 255;
   } else if (colSize > 65536) { // read the field in 64 KB chunks
      colSize = 65536;
   }

   fieldVal.resize(colSize);
   ulong read = 0;

   while (true) {
      r = SQLGetData(hStmt, column + 1, SQL_C_BINARY, (SQLPOINTER)(fieldVal.constData() + read),
            colSize, &lengthIndicator);

      if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
         break;
      }

      if (lengthIndicator == SQL_NULL_DATA) {
         return QVariant(QVariant::ByteArray);
      }

      if (lengthIndicator > SQLLEN(colSize) || lengthIndicator == SQL_NO_TOTAL) {
         read += colSize;
         colSize = 65536;
      } else {
         read += lengthIndicator;
      }

      if (r == SQL_SUCCESS) {
         // the whole field was read in one chunk
         fieldVal.resize(read);
         break;
      }

      fieldVal.resize(fieldVal.size() + colSize);
   }

   return fieldVal;
}

static QVariant qGetIntData(SQLHANDLE hStmt, int column, bool isSigned = true)
{
   SQLINTEGER intbuf = 0;
   SQLLEN lengthIndicator = 0;

   SQLRETURN r = SQLGetData(hStmt, column + 1, isSigned ? SQL_C_SLONG : SQL_C_ULONG,
         (SQLPOINTER)&intbuf, sizeof(intbuf), &lengthIndicator);

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
      return QVariant(QVariant::Invalid);
   }

   if (lengthIndicator == SQL_NULL_DATA) {
      return QVariant(QVariant::Int);
   }

   if (isSigned) {
      return int(intbuf);
   } else {
      return uint(intbuf);
   }
}

static QVariant qGetDoubleData(SQLHANDLE hStmt, int column)
{
   SQLDOUBLE dblbuf;
   SQLLEN lengthIndicator = 0;

   SQLRETURN r = SQLGetData(hStmt, column + 1, SQL_C_DOUBLE, (SQLPOINTER) &dblbuf, 0, &lengthIndicator);

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
      return QVariant(QVariant::Invalid);
   }

   if (lengthIndicator == SQL_NULL_DATA) {
      return QVariant(QVariant::Double);
   }

   return (double) dblbuf;
}

static QVariant qGetBigIntData(SQLHANDLE hStmt, int column, bool isSigned = true)
{
   SQLBIGINT lngbuf = 0;
   SQLLEN lengthIndicator = 0;

   SQLRETURN r = SQLGetData(hStmt, column + 1, isSigned ? SQL_C_SBIGINT : SQL_C_UBIGINT,
         (SQLPOINTER) &lngbuf, sizeof(lngbuf), &lengthIndicator);

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
      return QVariant(QVariant::Invalid);
   }

   if (lengthIndicator == SQL_NULL_DATA) {
      return QVariant(QVariant::LongLong);
   }

   if (isSigned) {
      return qint64(lngbuf);
   } else {
      return quint64(lngbuf);
   }
}

static bool isAutoValue(const SQLHANDLE hStmt, int column)
{
   SQLLEN nNumericAttribute = 0;    // check for auto-increment

   const SQLRETURN r = ::SQLColAttribute(hStmt, column + 1, SQL_DESC_AUTO_UNIQUE_VALUE,
         nullptr, 0, nullptr, &nNumericAttribute);

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
      qSqlWarning("qMakeField: Unable to obtain autovalue attribute for column #"
            + QString::number(column), hStmt);

      return false;
   }

   return nNumericAttribute != SQL_FALSE;
}

// creates a QSqlField from a valid hStmt generated by SQLColumns
// hStmt has to point to a valid position
static QSqlField qMakeFieldInfo(const SQLHANDLE hStmt, const QODBCDriverPrivate *p)
{
   QString fname = qGetStringData(hStmt, 3, -1, p->unicode);
   int type      = qGetIntData(hStmt, 4).toInt();         // column type

   QSqlField f(fname, qDecodeODBCType(type, p));

   QVariant var = qGetIntData(hStmt, 6);
   f.setLength(! var.isValid() ? -1 : var.toInt());       // column size

   var = qGetIntData(hStmt, 8).toInt();

   f.setPrecision(! var.isValid() ? -1 : var.toInt());    // precision
   f.setSqlType(type);
   int required = qGetIntData(hStmt, 10).toInt();

   // required can be SQL_NO_NULLS, SQL_NULLABLE, or SQL_NULLABLE_UNKNOWN
   if (required == SQL_NO_NULLS) {
      f.setRequired(true);
   } else if (required == SQL_NULLABLE) {
      f.setRequired(false);
   }

   // else we do not know

   return f;
}

static QSqlField qMakeFieldInfo(const SQLHANDLE hStmt, int i, QString *errorMessage)
{
   SQLSMALLINT colNameLen;
   SQLSMALLINT colType;
   SQLULEN colSize;
   SQLSMALLINT colScale;
   SQLSMALLINT nullable;

   SQLRETURN r = SQL_ERROR;

   QVarLengthArray<SQLTCHAR> colName(COLNAMESIZE);
   errorMessage->clear();

   r = SQLDescribeCol(hStmt, i + 1, colName.data(), (SQLSMALLINT)COLNAMESIZE, &colNameLen,
         &colType, &colSize, &colScale, &nullable);

   if (r != SQL_SUCCESS) {
      *errorMessage = "qMakeField: Unable to describe column " + QString::number(i);

      return QSqlField();
   }

   SQLLEN unsignedFlag = SQL_FALSE;
   r = SQLColAttribute (hStmt, i + 1, SQL_DESC_UNSIGNED, nullptr, 0, nullptr, &unsignedFlag);

   if (r != SQL_SUCCESS) {
      qSqlWarning(QString("qMakeField: Unable to get column attributes for column %1").formatArg(i), hStmt);
   }

   QString qColName(fromSQLTCHAR(colName, colNameLen));

   // nullable can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
   QVariant::Type type = qDecodeODBCType(colType, unsignedFlag == SQL_FALSE);

   QSqlField f(qColName, type);
   f.setSqlType(colType);
   f.setLength(colSize == 0 ? -1 : int(colSize));
   f.setPrecision(colScale == 0 ? -1 : int(colScale));

   if (nullable == SQL_NO_NULLS) {
      f.setRequired(true);
   } else if (nullable == SQL_NULLABLE) {
      f.setRequired(false);
   }

   // else we do not know
   f.setAutoValue(isAutoValue(hStmt, i));

   return f;
}

static QSqlField qMakeFieldInfo(const QODBCResultPrivate *p, int i)
{
   QString errorMessage;
   const QSqlField result = qMakeFieldInfo(p->hStmt, i, &errorMessage);

   if (! errorMessage.isEmpty()) {
      qSqlWarning(errorMessage, p);
   }

   return result;
}

static size_t qGetODBCVersion(const QString &connOpts)
{
   if (connOpts.contains("SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3", Qt::CaseInsensitive)) {
      return SQL_OV_ODBC3;
   }

   return SQL_OV_ODBC2;
}

QChar QODBCDriverPrivate::quoteChar() const
{
   if (! isQuoteInitialized) {
      SQLTCHAR driverResponse[4];
      SQLSMALLINT length;

      int r = SQLGetInfo(hDbc, SQL_IDENTIFIER_QUOTE_CHAR, &driverResponse, sizeof(driverResponse), &length);

      if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
         m_quote = QChar(driverResponse[0]);

      } else {
         m_quote = '"';
      }

      isQuoteInitialized = true;
   }

   return m_quote;
}

bool QODBCDriverPrivate::setConnectionOptions(const QString &connOpts)
{
   // Set any connection attributes
   const QStringList opts(connOpts.split(';', QStringParser::SkipEmptyParts));
   SQLRETURN r = SQL_SUCCESS;

   for (int i = 0; i < opts.count(); ++i) {
      const QString tmp(opts.at(i));
      int idx;

      if ((idx = tmp.indexOf('=')) == -1) {
         qWarning() << "QODBCDriver::open: Illegal connect option value '" << tmp << '\'';
         continue;
      }

      const QString opt(tmp.left(idx));
      const QString val(tmp.mid(idx + 1).simplified());
      SQLUINTEGER v = 0;

      r = SQL_SUCCESS;

      if (opt.toUpper() == "SQL_ATTR_ACCESS_MODE") {
         if (val.toUpper() == "SQL_MODE_READ_ONLY") {
            v = SQL_MODE_READ_ONLY;

         } else if (val.toUpper() == "SQL_MODE_READ_WRITE") {
            v = SQL_MODE_READ_WRITE;

         } else {
            qWarning() << "QODBCDriver::open: Unknown option value '" << val << '\'';
            continue;
         }

         r = SQLSetConnectAttr(hDbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER) static_cast<std::uintptr_t>(v), 0);

      } else if (opt.toUpper() == "SQL_ATTR_CONNECTION_TIMEOUT") {
         v = val.toInteger<uint>();
         r = SQLSetConnectAttr(hDbc, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER) static_cast<std::uintptr_t>(v), 0);

      } else if (opt.toUpper() == "SQL_ATTR_LOGIN_TIMEOUT") {
         v = val.toInteger<uint>();
         r = SQLSetConnectAttr(hDbc, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER) static_cast<std::uintptr_t>(v), 0);

      } else if (opt.toUpper() == "SQL_ATTR_CURRENT_CATALOG") {
         QVarLengthArray<SQLTCHAR> tmp = toSQLTCHAR(val);
         r = SQLSetConnectAttr(hDbc, SQL_ATTR_CURRENT_CATALOG, tmp.data(), tmp.length() * sizeof(SQLTCHAR));

      } else if (opt.toUpper() == "SQL_ATTR_METADATA_ID") {
         if (val.toUpper() == "SQL_TRUE") {
            v = SQL_TRUE;

         } else if (val.toUpper() == "SQL_FALSE") {
            v = SQL_FALSE;

         } else {
            qWarning() << "QODBCDriver::open: Unknown option value '" << val << '\'';
            continue;
         }

         r = SQLSetConnectAttr(hDbc, SQL_ATTR_METADATA_ID, (SQLPOINTER) static_cast<std::uintptr_t>(v), 0);

      } else if (opt.toUpper() == "SQL_ATTR_PACKET_SIZE") {
         v = val.toInteger<uint>();
         r = SQLSetConnectAttr(hDbc, SQL_ATTR_PACKET_SIZE, (SQLPOINTER) static_cast<std::uintptr_t>(v), 0);

      } else if (opt.toUpper() == "SQL_ATTR_TRACEFILE") {
         QVarLengthArray<SQLTCHAR> tmp = toSQLTCHAR(val);
         r = SQLSetConnectAttr(hDbc, SQL_ATTR_TRACEFILE, tmp.data(), tmp.length() * sizeof(SQLTCHAR));

      } else if (opt.toUpper() == "SQL_ATTR_TRACE") {
         if (val.toUpper() == "SQL_OPT_TRACE_OFF") {
            v = SQL_OPT_TRACE_OFF;

         } else if (val.toUpper() == "SQL_OPT_TRACE_ON") {
            v = SQL_OPT_TRACE_ON;

         } else {
            qWarning() << "QODBCDriver::open: Unknown option value '" << val << '\'';
            continue;
         }

         r = SQLSetConnectAttr(hDbc, SQL_ATTR_TRACE, (SQLPOINTER) static_cast<std::uintptr_t>(v), 0);

      } else if (opt.toUpper() == "SQL_ATTR_CONNECTION_POOLING") {
         if (val == "SQL_CP_OFF") {
            v = SQL_CP_OFF;

         } else if (val.toUpper() == "SQL_CP_ONE_PER_DRIVER") {
            v = SQL_CP_ONE_PER_DRIVER;

         } else if (val.toUpper() == "SQL_CP_ONE_PER_HENV") {
            v = SQL_CP_ONE_PER_HENV;

         } else if (val.toUpper() == "SQL_CP_DEFAULT") {
            v = SQL_CP_DEFAULT;

         } else {
            qWarning() << "QODBCDriver::open: Unknown option value '" << val << '\'';
            continue;
         }

         r = SQLSetConnectAttr(hDbc, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER) static_cast<std::uintptr_t>(v), 0);

      } else if (opt.toUpper() == "SQL_ATTR_CP_MATCH") {
         if (val.toUpper() == "SQL_CP_STRICT_MATCH") {
            v = SQL_CP_STRICT_MATCH;

         } else if (val.toUpper() == "SQL_CP_RELAXED_MATCH") {
            v = SQL_CP_RELAXED_MATCH;

         } else if (val.toUpper() == "SQL_CP_MATCH_DEFAULT") {
            v = SQL_CP_MATCH_DEFAULT;

         } else {
            qWarning() << "QODBCDriver::open: Unknown option value '" << val << '\'';
            continue;
         }

         r = SQLSetConnectAttr(hDbc, SQL_ATTR_CP_MATCH, (SQLPOINTER) static_cast<std::uintptr_t>(v), 0);

      } else if (opt.toUpper() == "SQL_ATTR_ODBC_VERSION") {
         // Already handled in QODBCDriver::open()
         continue;
      } else {
         qWarning() << "QODBCDriver::open: Unknown connection attribute '" << opt << '\'';
      }

      if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
         qSqlWarning(QString::fromLatin1("QODBCDriver::open: Unable to set connection attribute'%1'").formatArg(opt), this);
      }
   }
   return true;
}

void QODBCDriverPrivate::splitTableQualifier(const QString &qualifier, QString &catalog, QString &schema, QString &table)
{
   if (! useSchema) {
      table = qualifier;
      return;
   }

   QStringList list = qualifier.split(QChar('.'));
   int count = list.count();

   if (count > 3) {
      return;   // can not be a valid table qualifier
   }

   int index = 0;

   if (count == 1) {
      table = qualifier;

   } else {
      for (const auto &item : list) {
         if (count == 3) {
            if (index == 0) {
               catalog = item;

            } else if (index == 1) {
               schema = item;

            } else if (index == 2) {
               table = item;
            }

         } else if (count == 2) {
            if (index == 0) {
               schema = item;

            } else if (index == 1) {
               table = item;
            }
         }

         ++index;
      }
   }
}

QODBCDriverPrivate::DefaultCase QODBCDriverPrivate::defaultCase() const
{
   DefaultCase ret;
   SQLUSMALLINT casing;

   int r = SQLGetInfo(hDbc, SQL_IDENTIFIER_CASE, &casing, sizeof(casing), nullptr);

   if ( r != SQL_SUCCESS) {
      ret = Mixed;   // arbitrary case if driver cannot be queried

   } else {
      switch (casing) {
         case (SQL_IC_UPPER):
            ret = Upper;
            break;

         case (SQL_IC_LOWER):
            ret = Lower;
            break;

         case (SQL_IC_SENSITIVE):
            ret = Sensitive;
            break;

         case (SQL_IC_MIXED):
         default:
            ret = Mixed;
            break;
      }
   }

   return ret;
}


// adjust the casing of an identifier to match what the database engine would have done to it.
QString QODBCDriverPrivate::adjustCase(const QString &identifier) const
{
   QString ret = identifier;

   switch (defaultCase()) {
      case (Lower):
         ret = identifier.toLower();
         break;

      case (Upper):
         ret = identifier.toUpper();
         break;

      case (Mixed):
      case (Sensitive):
      default:
         ret = identifier;
   }

   return ret;
}

QODBCResult::QODBCResult(const QODBCDriver *db, QODBCDriverPrivate *p)
   : QSqlResult(*new QODBCResultPrivate(p), db)
{
}

QODBCResult::~QODBCResult()
{
   Q_D(QODBCResult);

   if (d->hStmt && d->isStmtHandleValid(driver()) && driver()->isOpen()) {
      SQLRETURN r = SQLFreeHandle(SQL_HANDLE_STMT, d->hStmt);

      if (r != SQL_SUCCESS) {
         qSqlWarning(QString("QODBCDriver: Unable to free statement handle ") + QString::number(r), d);
      }
   }
}

bool QODBCResult::reset (const QString &query)
{
   Q_D(QODBCResult);

   setActive(false);
   setAt(QSql::BeforeFirstRow);
   d->rInf.clear();
   d->fieldCache.clear();
   d->fieldCacheIdx = 0;

   // Always reallocate the statement handle - the statement attributes
   // are not reset if SQLFreeStmt() is called which causes some problems.
   SQLRETURN r;

   if (d->hStmt && d->isStmtHandleValid(driver())) {
      r = SQLFreeHandle(SQL_HANDLE_STMT, d->hStmt);
      if (r != SQL_SUCCESS) {
         qSqlWarning(QString("QODBCResult::reset: Unable to free statement handle"), d);
         return false;
      }
   }
   r  = SQLAllocHandle(SQL_HANDLE_STMT, d->dpDbc(), &d->hStmt);
   if (r != SQL_SUCCESS) {
      qSqlWarning(QString("QODBCResult::reset: Unable to allocate statement handle"), d);
      return false;
   }

   d->updateStmtHandleState(driver());

   if (d->userForwardOnly) {
      r = SQLSetStmtAttr(d->hStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, SQL_IS_UINTEGER);

   } else {
      r = SQLSetStmtAttr(d->hStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, SQL_IS_UINTEGER);
   }

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
      setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
               "QODBCResult::reset: Unable to set 'SQL_CURSOR_STATIC' as statement attribute. "
               "Please check your ODBC driver configuration"), QSqlError::StatementError, d));
      return false;
   }
   r = SQLExecDirect(d->hStmt, toSQLTCHAR(query).data(), (SQLINTEGER) query.length());

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO && r != SQL_NO_DATA) {
      setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
               "Unable to execute statement"), QSqlError::StatementError, d));
      return false;
   }

   SQLULEN isScrollable = 0;
   r = SQLGetStmtAttr(d->hStmt, SQL_ATTR_CURSOR_SCROLLABLE, &isScrollable, SQL_IS_INTEGER, nullptr);

   if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
      QSqlResult::setForwardOnly(isScrollable == SQL_NONSCROLLABLE);
   }

   SQLSMALLINT count;
   SQLNumResultCols(d->hStmt, &count);

   if (count) {
      setSelect(true);
      for (int i = 0; i < count; ++i) {
         d->rInf.append(qMakeFieldInfo(d, i));
      }

      d->fieldCache.resize(count);

   } else {
      setSelect(false);
   }

   setActive(true);

   return true;
}

bool QODBCResult::fetch(int i)
{
   Q_D(QODBCResult);

   if (! driver()->isOpen()) {
      return false;
   }

   if (isForwardOnly() && i < at()) {
      return false;
   }

   if (i == at()) {
      return true;
   }

   d->clearValues();
   int actualIdx = i + 1;
   if (actualIdx <= 0) {
      setAt(QSql::BeforeFirstRow);
      return false;
   }

   SQLRETURN r;
   if (isForwardOnly()) {
      bool ok = true;
      while (ok && i > at()) {
         ok = fetchNext();
      }

      return ok;

   } else {
      r = SQLFetchScroll(d->hStmt,
            SQL_FETCH_ABSOLUTE,
            actualIdx);
   }

   if (r != SQL_SUCCESS) {
      if (r != SQL_NO_DATA)
         setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                  "Unable to fetch"), QSqlError::ConnectionError, d));
      return false;
   }

   setAt(i);

   return true;
}

bool QODBCResult::fetchNext()
{
   Q_D(QODBCResult);

   SQLRETURN r;
   d->clearValues();

   if (d->hasSQLFetchScroll)
      r = SQLFetchScroll(d->hStmt, SQL_FETCH_NEXT, 0);
   else {
      r = SQLFetch(d->hStmt);
   }

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
      if (r != SQL_NO_DATA)
         setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                  "Unable to fetch next"), QSqlError::ConnectionError, d));
      return false;
   }

   setAt(at() + 1);

   return true;
}

bool QODBCResult::fetchFirst()
{
   Q_D(QODBCResult);

   if (isForwardOnly() && at() != QSql::BeforeFirstRow) {
      return false;
   }

   SQLRETURN r;
   d->clearValues();
   if (isForwardOnly()) {
      return fetchNext();
   }

   r = SQLFetchScroll(d->hStmt, SQL_FETCH_FIRST, 0);

   if (r != SQL_SUCCESS) {
      if (r != SQL_NO_DATA)
         setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                  "Unable to fetch first"), QSqlError::ConnectionError, d));
      return false;
   }

   setAt(0);
   return true;
}

bool QODBCResult::fetchPrevious()
{
   Q_D(QODBCResult);

   if (isForwardOnly()) {
      return false;
   }

   SQLRETURN r;
   d->clearValues();
   r = SQLFetchScroll(d->hStmt, SQL_FETCH_PRIOR, 0);

   if (r != SQL_SUCCESS) {
      if (r != SQL_NO_DATA)
         setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                  "Unable to fetch previous"), QSqlError::ConnectionError, d));
      return false;
   }

   setAt(at() - 1);

   return true;
}

bool QODBCResult::fetchLast()
{
   Q_D(QODBCResult);

   SQLRETURN r;
   d->clearValues();

   if (isForwardOnly()) {
      // can not seek to last row in forwardOnly mode, so we have to use brute force
      int i = at();
      if (i == QSql::AfterLastRow) {
         return false;
      }

      if (i == QSql::BeforeFirstRow) {
         i = 0;
      }

      while (fetchNext()) {
         ++i;
      }

      setAt(i);

      return true;
   }

   r = SQLFetchScroll(d->hStmt, SQL_FETCH_LAST, 0);
   if (r != SQL_SUCCESS) {
      if (r != SQL_NO_DATA)
         setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                  "Unable to fetch last"), QSqlError::ConnectionError, d));
      return false;
   }

   SQLULEN currRow = 0;
   r = SQLGetStmtAttr(d->hStmt, SQL_ROW_NUMBER, &currRow, SQL_IS_INTEGER, nullptr);

   if (r != SQL_SUCCESS) {
      return false;
   }

   setAt(currRow - 1);

   return true;
}

QVariant QODBCResult::data(int field)
{
   Q_D(QODBCResult);

   if (field >= d->rInf.count() || field < 0) {
      qWarning() << "QODBCResult::data: column" << field << "out of range";
      return QVariant();
   }
   if (field < d->fieldCacheIdx) {
      return d->fieldCache.at(field);
   }

   SQLRETURN r(0);
   SQLLEN lengthIndicator = 0;

   for (int i = d->fieldCacheIdx; i <= field; ++i) {
      // some servers do not support fetching column n after we already
      // fetched column n+1, cache all previous columns here
      const QSqlField info = d->rInf.field(i);

      switch (info.type()) {
         case QVariant::LongLong:
            d->fieldCache[i] = qGetBigIntData(d->hStmt, i);
            break;

         case QVariant::ULongLong:
            d->fieldCache[i] = qGetBigIntData(d->hStmt, i, false);
            break;

         case QVariant::Int:
            d->fieldCache[i] = qGetIntData(d->hStmt, i);
            break;

         case QVariant::UInt:
            d->fieldCache[i] = qGetIntData(d->hStmt, i, false);
            break;

         case QVariant::Date:
            DATE_STRUCT dbuf;
            r = SQLGetData(d->hStmt, i + 1, SQL_C_DATE, (SQLPOINTER)&dbuf, 0, &lengthIndicator);

            if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (lengthIndicator != SQL_NULL_DATA)) {
               d->fieldCache[i] = QVariant(QDate(dbuf.year, dbuf.month, dbuf.day));
            } else {
               d->fieldCache[i] = QVariant(QVariant::Date);
            }
            break;

         case QVariant::Time:
            TIME_STRUCT tbuf;
            r = SQLGetData(d->hStmt, i + 1, SQL_C_TIME, (SQLPOINTER)&tbuf, 0, &lengthIndicator);
            if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (lengthIndicator != SQL_NULL_DATA)) {
               d->fieldCache[i] = QVariant(QTime(tbuf.hour, tbuf.minute, tbuf.second));
            } else {
               d->fieldCache[i] = QVariant(QVariant::Time);
            }
            break;

         case QVariant::DateTime:
            TIMESTAMP_STRUCT dtbuf;
            r = SQLGetData(d->hStmt, i + 1, SQL_C_TIMESTAMP, (SQLPOINTER)&dtbuf, 0, &lengthIndicator);

            if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (lengthIndicator != SQL_NULL_DATA))
               d->fieldCache[i] = QVariant(QDateTime(QDate(dtbuf.year, dtbuf.month, dtbuf.day),
                     QTime(dtbuf.hour, dtbuf.minute, dtbuf.second, dtbuf.fraction / 1000000)));

            else {
               d->fieldCache[i] = QVariant(QVariant::DateTime);
            }
            break;

         case QVariant::ByteArray:
            d->fieldCache[i] = qGetBinaryData(d->hStmt, i);
            break;

         case QVariant::String:
            d->fieldCache[i] = qGetStringData(d->hStmt, i, info.length(), d->unicode);
            break;

         case QVariant::Double:
            switch (numericalPrecisionPolicy()) {
               case QSql::LowPrecisionInt32:
                  d->fieldCache[i] = qGetIntData(d->hStmt, i);
                  break;

               case QSql::LowPrecisionInt64:
                  d->fieldCache[i] = qGetBigIntData(d->hStmt, i);
                  break;

               case QSql::LowPrecisionDouble:
                  d->fieldCache[i] = qGetDoubleData(d->hStmt, i);
                  break;

               case QSql::HighPrecision:
                  d->fieldCache[i] = qGetStringData(d->hStmt, i, info.length(), false);
                  break;
            }
            break;

         default:
            d->fieldCache[i] = QVariant(qGetStringData(d->hStmt, i, info.length(), false));
            break;
      }

      d->fieldCacheIdx = field + 1;
   }

   return d->fieldCache[field];
}

bool QODBCResult::isNull(int field)
{
   Q_D(QODBCResult);

   if (field < 0 || field > d->fieldCache.size()) {
      return true;
   }

   if (field <= d->fieldCacheIdx) {
      // no good way to find out whether the value is NULL without fetching the field
      data(field);
   }

   return ! d->fieldCache.at(field).isValid();
}

int QODBCResult::size()
{
   return -1;
}

int QODBCResult::numRowsAffected()
{
   Q_D(QODBCResult);

   SQLLEN affectedRowCount = 0;
   SQLRETURN r = SQLRowCount(d->hStmt, &affectedRowCount);

   if (r == SQL_SUCCESS) {
      return affectedRowCount;
   } else {
      qSqlWarning(QString("QODBCResult::numRowsAffected: Unable to count affected rows"), d);
   }

   return -1;
}

bool QODBCResult::prepare(const QString &query)
{
   Q_D(QODBCResult);

   setActive(false);
   setAt(QSql::BeforeFirstRow);
   SQLRETURN r;

   d->rInf.clear();
   if (d->hStmt && d->isStmtHandleValid(driver())) {
      r = SQLFreeHandle(SQL_HANDLE_STMT, d->hStmt);

      if (r != SQL_SUCCESS) {
         qSqlWarning(QString("QODBCResult::prepare: Unable to close statement"), d);
         return false;
      }
   }
   r  = SQLAllocHandle(SQL_HANDLE_STMT, d->dpDbc(), &d->hStmt);

   if (r != SQL_SUCCESS) {
      qSqlWarning(QString("QODBCResult::prepare: Unable to allocate statement handle"), d);
      return false;
   }

   d->updateStmtHandleState(driver());

   if (d->userForwardOnly) {
      r = SQLSetStmtAttr(d->hStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, SQL_IS_UINTEGER);
   } else {
      r = SQLSetStmtAttr(d->hStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, SQL_IS_UINTEGER);
   }

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
      setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
               "QODBCResult::reset: Unable to set 'SQL_CURSOR_STATIC' as statement attribute. "
               "Please check your ODBC driver configuration"), QSqlError::StatementError, d));
      return false;
   }

   r = SQLPrepare(d->hStmt, toSQLTCHAR(query).data(), (SQLINTEGER) query.length());


   if (r != SQL_SUCCESS) {
      setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
               "Unable to prepare statement"), QSqlError::StatementError, d));
      return false;
   }
   return true;
}

bool QODBCResult::exec()
{
   Q_D(QODBCResult);

   setActive(false);
   setAt(QSql::BeforeFirstRow);
   d->rInf.clear();
   d->fieldCache.clear();
   d->fieldCacheIdx = 0;

   if (! d->hStmt) {
      qSqlWarning(QString("QODBCResult::exec: No statement handle available"), d);
      return false;
   }

   if (isSelect()) {
      SQLCloseCursor(d->hStmt);
   }

   QVector<QVariant> &values = boundValues();
   QVector<QByteArray> tmpStorage(values.count(), QByteArray()); // holds temporary buffers
   QVarLengthArray<SQLLEN, 32> indicators(values.count());
   memset(indicators.data(), 0, indicators.size() * sizeof(SQLLEN));

   // bind parameters - only positional binding allowed
   int i;

   SQLRETURN r;
   for (i = 0; i < values.count(); ++i) {
      const QVariant &val = values.at(i);

      SQLLEN *ind = &indicators[i];
      if (! val.isValid()) {
         *ind = SQL_NULL_DATA;
      }

      switch (val.type()) {
         case QVariant::Date: {
            QByteArray &ba = tmpStorage[i];
            ba.resize(sizeof(DATE_STRUCT));

            DATE_STRUCT *dt = (DATE_STRUCT *)const_cast<char *>(ba.constData());

            QDate qdt = val.toDate();
            dt->year  = qdt.year();
            dt->month = qdt.month();
            dt->day   = qdt.day();

            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                  SQL_C_DATE, SQL_DATE, 0, 0, (void *) dt, 0, *ind == SQL_NULL_DATA ? ind : NULL);

            break;
         }

         case QVariant::Time: {
            QByteArray &ba = tmpStorage[i];
            ba.resize(sizeof(TIME_STRUCT));

            TIME_STRUCT *dt = (TIME_STRUCT *)const_cast<char *>(ba.constData());

            QTime qdt = val.toTime();
            dt->hour = qdt.hour();
            dt->minute = qdt.minute();
            dt->second = qdt.second();

            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                  SQL_C_TIME, SQL_TIME, 0, 0, (void *) dt, 0, *ind == SQL_NULL_DATA ? ind : NULL);

            break;
         }

         case QVariant::DateTime: {
            QByteArray &ba = tmpStorage[i];
            ba.resize(sizeof(TIMESTAMP_STRUCT));

            TIMESTAMP_STRUCT * dt = (TIMESTAMP_STRUCT *)const_cast<char *>(ba.constData());

            QDateTime qdt = val.toDateTime();
            dt->year = qdt.date().year();
            dt->month = qdt.date().month();
            dt->day = qdt.date().day();
            dt->hour = qdt.time().hour();
            dt->minute = qdt.time().minute();
            dt->second = qdt.time().second();

            int precision = d->driverPrivate->datetime_precision - 20; // (20 includes a separating period)

            if (precision <= 0) {
               dt->fraction = 0;

            } else {
               dt->fraction = qdt.time().msec() * 1000000;

               // How many leading digits do we want to keep? With SQL Server 2005, this should be 3: 123000000
               int keep = (int)qPow(10.0, 9 - qMin(9, precision));
               dt->fraction = (dt->fraction / keep) * keep;
            }

            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                  SQL_C_TIMESTAMP, SQL_TIMESTAMP, d->driverPrivate->datetime_precision, precision,
                  (void *) dt, 0, *ind == SQL_NULL_DATA ? ind : NULL);

            break;
         }

         case QVariant::Int: {
            int tmp = val.toInt();

            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                  SQL_C_SLONG, SQL_INTEGER, 0, 0, &tmp, 0,
                  *ind == SQL_NULL_DATA ? ind : NULL);

            break;
         }

         case QVariant::UInt: {
            uint tmp = val.toUInt();
            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                  SQL_C_ULONG, SQL_NUMERIC, 15, 0, &tmp, 0,
                  *ind == SQL_NULL_DATA ? ind : NULL);
            break;
         }

         case QVariant::Double: {
            double tmp = val.toDouble();
            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                  SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &tmp, 0,
                  *ind == SQL_NULL_DATA ? ind : NULL);
            break;
         }

         case QVariant::LongLong: {
            qint64 tmp = val.toLongLong();
            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                  SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &tmp, 0,
                  *ind == SQL_NULL_DATA ? ind : NULL);
            break;
         }

         case QVariant::ULongLong:  {
            quint64 tmp = val.toULongLong();
            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                  SQL_C_UBIGINT, SQL_BIGINT, 0, 0, &tmp, 0,
                  *ind == SQL_NULL_DATA ? ind : NULL);
            break;
         }

         case QVariant::ByteArray:
            if (*ind != SQL_NULL_DATA) {
               *ind = val.toByteArray().size();
            }

            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                  SQL_C_BINARY, SQL_LONGVARBINARY, val.toByteArray().size(), 0,
                  (void *) val.toByteArray().constData(), val.toByteArray().size(), ind);
            break;

         case QVariant::Bool: {
            bool tmp = val.toBool();

            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut], SQL_C_BIT,
                  SQL_BIT, 0, 0, &tmp, 0, *ind == SQL_NULL_DATA ? ind : NULL);
            break;
         }

         case QVariant::String:
            if (d->unicode) {
               QByteArray &ba = tmpStorage[i];
               QString str    = val.toString();

               if (*ind != SQL_NULL_DATA) {
                  *ind = str.length() * sizeof(SQLTCHAR);
               }

               int strSize = str.length() * sizeof(SQLTCHAR);

               if (bindValueType(i) & QSql::Out) {
                  const QVarLengthArray<SQLTCHAR> a(toSQLTCHAR(str));
                  ba = QByteArray((const char *)a.constData(), a.size() * sizeof(SQLTCHAR));

                  r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                        SQL_C_TCHAR, strSize > 254 ? SQL_WLONGVARCHAR : SQL_WVARCHAR,
                        0,    // do not change this value
                        0, (void *)a.constData(), a.size(), ind);
                  break;
               }

               ba = QByteArray((const char *)toSQLTCHAR(str).constData(), str.size() * sizeof(SQLTCHAR));
               r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                     SQL_C_TCHAR, strSize > 254 ? SQL_WLONGVARCHAR : SQL_WVARCHAR,
                     strSize, 0, (SQLPOINTER)ba.constData(), ba.size(), ind);
               break;

            } else {
               QByteArray &str = tmpStorage[i];
               str = val.toString().toUtf8();

               if (*ind != SQL_NULL_DATA) {
                  *ind = str.length();
               }
               int strSize = str.length();

               r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                     SQL_C_CHAR, strSize > 254 ? SQL_LONGVARCHAR : SQL_VARCHAR,
                     strSize, 0, (void *)str.constData(), strSize, ind);
               break;
            }
            [[fallthrough]];

         default: {
            QByteArray &ba = tmpStorage[i];

            if (*ind != SQL_NULL_DATA) {
               *ind = ba.size();
            }

            r = SQLBindParameter(d->hStmt, i + 1, qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                  SQL_C_BINARY, SQL_VARBINARY, ba.length() + 1, 0,
                  (void *) ba.constData(), ba.length() + 1, ind);
            break;
         }
      }

      if (r != SQL_SUCCESS) {
         qWarning() << "QODBCResult::exec: unable to bind variable:" << cs_warnODBC(d);
         setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                  "Unable to bind variable"), QSqlError::StatementError, d));
         return false;
      }
   }

   r = SQLExecute(d->hStmt);
   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO && r != SQL_NO_DATA) {
      qWarning() << "QODBCResult::exec: Unable to execute statement:" << cs_warnODBC(d);
      setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
               "Unable to execute statement"), QSqlError::StatementError, d));
      return false;
   }

   SQLULEN isScrollable = 0;
   r = SQLGetStmtAttr(d->hStmt, SQL_ATTR_CURSOR_SCROLLABLE, &isScrollable, SQL_IS_INTEGER, nullptr);
   if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
      QSqlResult::setForwardOnly(isScrollable == SQL_NONSCROLLABLE);
   }

   SQLSMALLINT count = 0;
   SQLNumResultCols(d->hStmt, &count);

   if (count) {
      setSelect(true);
      for (int i = 0; i < count; ++i) {
         d->rInf.append(qMakeFieldInfo(d, i));
      }

      d->fieldCache.resize(count);

   } else {
      setSelect(false);
   }
   setActive(true);

   // get out parameters
   if (! hasOutValues()) {
      return true;
   }

   for (i = 0; i < values.count(); ++i) {
      switch (values.at(i).type()) {
         case QVariant::Date: {
            DATE_STRUCT ds = *((DATE_STRUCT *)tmpStorage.at(i).constData());
            values[i] = QVariant(QDate(ds.year, ds.month, ds.day));
            break;
         }

         case QVariant::Time: {
            TIME_STRUCT dt = *((TIME_STRUCT *)tmpStorage.at(i).constData());
            values[i] = QVariant(QTime(dt.hour, dt.minute, dt.second));
            break;
         }

         case QVariant::DateTime: {
            TIMESTAMP_STRUCT dt = *((TIMESTAMP_STRUCT *)tmpStorage.at(i).constData());

            values[i] = QVariant(QDateTime(QDate(dt.year, dt.month, dt.day),
                     QTime(dt.hour, dt.minute, dt.second, dt.fraction / 1000000)));
            break;
         }

         case QVariant::Bool:
         case QVariant::Int:
         case QVariant::UInt:
         case QVariant::Double:
         case QVariant::ByteArray:
         case QVariant::LongLong:
         case QVariant::ULongLong:
            // nothing to do
            break;

         case QVariant::String:
            if (d->unicode) {
               if (bindValueType(i) & QSql::Out) {
                  const QByteArray &first = tmpStorage.at(i);
                  QVarLengthArray<SQLTCHAR> array;
                  array.append((const SQLTCHAR *)first.constData(), first.size());
                  values[i] = fromSQLTCHAR(array, first.size() / sizeof(SQLTCHAR *));
               }
               break;
            }
            [[fallthrough]];

         default: {
            if (bindValueType(i) & QSql::Out) {
               values[i] = tmpStorage.at(i);
            }
            break;
         }
      }

      if (indicators[i] == SQL_NULL_DATA) {
         values[i] = QVariant(values[i].type());
      }
   }

   return true;
}

QSqlRecord QODBCResult::record() const
{
   Q_D(const QODBCResult);

   if (! isActive() || ! isSelect()) {
      return QSqlRecord();
   }

   return d->rInf;
}

QVariant QODBCResult::lastInsertId() const
{
   Q_D(const QODBCResult);

   QString sql;

   switch (d->driverPrivate->dbmsType) {
      case QSqlDriver::MSSqlServer:
      case QSqlDriver::Sybase:
         sql = "SELECT @@IDENTITY;";
         break;

      case QSqlDriver::MySqlServer:
         sql = "SELECT LAST_INSERT_ID();";
         break;

      case QSqlDriver::PostgreSQL:
         sql = "SELECT lastval();";
         break;

      default:
         break;
   }

   if (!sql.isEmpty()) {
      QSqlQuery qry(driver()->createResult());

      if (qry.exec(sql) && qry.next()) {
         return qry.value(0);
      }

      qSqlWarning("QODBCResult::lastInsertId: Unable to get lastInsertId", d);

   } else {
      qSqlWarning("QODBCResult::lastInsertId: not implemented for this DBMS", d);
   }

   return QVariant();
}

QVariant QODBCResult::handle() const
{
   Q_D(const QODBCResult);

   return QVariant::fromValue<SQLHANDLE>(d->hStmt);
}

bool QODBCResult::nextResult()
{
   Q_D(QODBCResult);

   setActive(false);
   setAt(QSql::BeforeFirstRow);
   d->rInf.clear();
   d->fieldCache.clear();
   d->fieldCacheIdx = 0;
   setSelect(false);

   SQLRETURN r = SQLMoreResults(d->hStmt);

   if (r != SQL_SUCCESS) {
      if (r == SQL_SUCCESS_WITH_INFO) {
         int nativeCode  = -1;
         QString message = cs_warnODBC(d, &nativeCode);
         qWarning() << "QODBCResult::nextResult():" << message;

      } else {
         if (r != SQL_NO_DATA) {
            setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                  "Unable to fetch last"), QSqlError::ConnectionError, d));
         }

         return false;
      }
   }

   SQLSMALLINT count = 0;
   SQLNumResultCols(d->hStmt, &count);

   if (count) {
      setSelect(true);

      for (int i = 0; i < count; ++i) {
         d->rInf.append(qMakeFieldInfo(d, i));
      }

      d->fieldCache.resize(count);

   } else {
      setSelect(false);
   }

   setActive(true);

   return true;
}

void QODBCResult::virtual_hook(int id, void *data)
{
   QSqlResult::virtual_hook(id, data);
}

void QODBCResult::detachFromResultSet()
{
   Q_D(QODBCResult);

   if (d->hStmt) {
      SQLCloseCursor(d->hStmt);
   }
}

void QODBCResult::setForwardOnly(bool forward)
{
   Q_D(QODBCResult);

   d->userForwardOnly = forward;
   QSqlResult::setForwardOnly(forward);
}

QODBCDriver::QODBCDriver(QObject *parent)
   : QSqlDriver(*new QODBCDriverPrivate(), parent)
{
}

QODBCDriver::QODBCDriver(SQLHANDLE env, SQLHANDLE con, QObject *parent)
   : QSqlDriver(*new QODBCDriverPrivate(), parent)
{
   Q_D(QODBCDriver);

   d->hEnv = env;
   d->hDbc = con;

   if (env && con) {
      setOpen(true);
      setOpenError(false);
   }
}

QODBCDriver::~QODBCDriver()
{
   cleanup();
}

bool QODBCDriver::hasFeature(DriverFeature f) const
{
   Q_D(const QODBCDriver);

   switch (f) {
      case Transactions: {
         if (! d->hDbc) {
            return false;
         }

         SQLUSMALLINT txn;
         SQLSMALLINT t;

         int r = SQLGetInfo(d->hDbc, (SQLUSMALLINT)SQL_TXN_CAPABLE, &txn, sizeof(txn), &t);

         if (r != SQL_SUCCESS || txn == SQL_TC_NONE) {
            return false;
         } else {
            return true;
         }
      }

      case Unicode:
         return d->unicode;

      case PreparedQueries:
      case PositionalPlaceholders:
      case FinishQuery:
      case LowPrecisionNumbers:
         return true;

      case QuerySize:
      case NamedPlaceholders:
      case BatchOperations:
      case SimpleLocking:
      case EventNotifications:
      case CancelQuery:
         return false;

      case LastInsertId:
         return (d->dbmsType == MSSqlServer)
               || (d->dbmsType == Sybase)
               || (d->dbmsType == MySqlServer)
               || (d->dbmsType == PostgreSQL);

      case MultipleResultSets:
         return d->hasMultiResultSets;

      case BLOB: {
         if (d->dbmsType == MySqlServer) {
            return true;
         } else {
            return false;
         }
      }
   }

   return false;
}

bool QODBCDriver::open(const QString &db, const QString &user, const QString &password,
      const QString &, int, const QString &connOpts)
{
   Q_D(QODBCDriver);

   if (isOpen()) {
      close();
   }

   SQLRETURN r;
   r = SQLAllocHandle(SQL_HANDLE_ENV, nullptr, &d->hEnv);

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
      qSqlWarning(QString("QODBCDriver::open: Unable to allocate environment"), d);
      setOpenError(true);
      return false;
   }

   r = SQLSetEnvAttr(d->hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)qGetODBCVersion(connOpts), SQL_IS_UINTEGER);
   r = SQLAllocHandle(SQL_HANDLE_DBC, d->hEnv, &d->hDbc);

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
      qSqlWarning(QString("QODBCDriver::open: Unable to allocate connection"), d);
      setOpenError(true);
      cleanup();

      return false;
   }

   if (! d->setConnectionOptions(connOpts)) {
      cleanup();

      return false;
   }

   // Create the connection string
   QString connQStr;

   // support the "DRIVER={SQL SERVER};SERVER=blah" syntax
   if (db.contains(".dsn", Qt::CaseInsensitive)) {
      connQStr = "FILEDSN=" + db;

   } else if (db.contains("DRIVER=", Qt::CaseInsensitive) || db.contains("SERVER=", Qt::CaseInsensitive)) {
      connQStr = db;

   } else {
      connQStr = "DSN=" + db;
   }

   if (! user.isEmpty()) {
      connQStr += ";UID=" + user;
   }

   if (! password.isEmpty()) {
      connQStr += ";PWD=" + password;
   }

   SQLSMALLINT cb;
   QVarLengthArray<SQLTCHAR> connOut(1024);
   memset(connOut.data(), 0, connOut.size() * sizeof(SQLTCHAR));

   r = SQLDriverConnect(d->hDbc, nullptr, toSQLTCHAR(connQStr).data(),
         (SQLSMALLINT)connQStr.length(), connOut.data(), 1024, &cb, 0);

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
      setLastError(qMakeError(tr("Unable to connect"), QSqlError::ConnectionError, d));
      setOpenError(true);
      cleanup();

      return false;
   }

   if (! d->checkDriver()) {
      setLastError(qMakeError(tr("Unable to connect - Driver does not support all "
               "functionality required"), QSqlError::ConnectionError, d));
      setOpenError(true);
      cleanup();

      return false;
   }

   d->checkUnicode();
   d->checkSchemaUsage();
   d->checkDBMS();
   d->checkHasSQLFetchScroll();
   d->checkHasMultiResults();
   d->checkDateTimePrecision();

   setOpen(true);
   setOpenError(false);

   if (d->dbmsType == MSSqlServer) {
      QSqlQuery i(createResult());
      i.exec("SET QUOTED_IDENTIFIER ON");
   }

   return true;
}

void QODBCDriver::close()
{
   cleanup();
   setOpen(false);
   setOpenError(false);
}

void QODBCDriver::cleanup()
{
   Q_D(QODBCDriver);

   SQLRETURN r;

   if (d->hDbc) {
      // Open statements/descriptors handles are automatically cleaned up by SQLDisconnect
      if (isOpen()) {
         r = SQLDisconnect(d->hDbc);

         if (r != SQL_SUCCESS) {
            qSqlWarning("QODBCDriver::disconnect: Unable to disconnect datasource", d);
         } else {
            ++d->disconnectCount;
         }
      }

      r = SQLFreeHandle(SQL_HANDLE_DBC, d->hDbc);
      if (r != SQL_SUCCESS) {
         qSqlWarning("QODBCDriver::cleanup: Unable to free connection handle", d);
      }

      d->hDbc = nullptr;
   }

   if (d->hEnv) {
      r = SQLFreeHandle(SQL_HANDLE_ENV, d->hEnv);
      if (r != SQL_SUCCESS) {
         qSqlWarning("QODBCDriver::cleanup: Unable to free environment handle", d);
      }

      d->hEnv = nullptr;
   }
}

// checks whether the server can return char, varchar and longvarchar
// as two byte unicode characters
void QODBCDriverPrivate::checkUnicode()
{
   SQLRETURN   r;
   SQLUINTEGER fFunc;

   unicode = false;
   r = SQLGetInfo(hDbc, SQL_CONVERT_CHAR, (SQLPOINTER)&fFunc, sizeof(fFunc), nullptr);

   if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (fFunc & SQL_CVT_WCHAR)) {
      unicode = true;
      return;
   }

   r = SQLGetInfo(hDbc, SQL_CONVERT_VARCHAR, (SQLPOINTER)&fFunc, sizeof(fFunc), nullptr);

   if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (fFunc & SQL_CVT_WVARCHAR)) {
      unicode = true;
      return;
   }

   r = SQLGetInfo(hDbc, SQL_CONVERT_LONGVARCHAR, (SQLPOINTER)&fFunc, sizeof(fFunc), nullptr);
   if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (fFunc & SQL_CVT_WLONGVARCHAR)) {
      unicode = true;
      return;
   }

   SQLHANDLE hStmt;
   r = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
   r = SQLExecDirect(hStmt, toSQLTCHAR(QString("select 'test'")).data(), SQL_NTS);

   if (r == SQL_SUCCESS) {
      r = SQLFetch(hStmt);

      if (r == SQL_SUCCESS) {
         QVarLengthArray<SQLWCHAR> buffer(10);

         r = SQLGetData(hStmt, 1, SQL_C_WCHAR, buffer.data(), buffer.size() * sizeof(SQLWCHAR), nullptr);

         if (r == SQL_SUCCESS && fromSQLTCHAR(buffer) == "test") {
            unicode = true;
         }
      }
   }

   r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

bool QODBCDriverPrivate::checkDriver() const
{
#ifdef ODBC_CHECK_DRIVER
   static const SQLUSMALLINT reqFunc[] = {
      SQL_API_SQLDESCRIBECOL, SQL_API_SQLGETDATA, SQL_API_SQLCOLUMNS,
      SQL_API_SQLGETSTMTATTR, SQL_API_SQLGETDIAGREC, SQL_API_SQLEXECDIRECT,
      SQL_API_SQLGETINFO, SQL_API_SQLTABLES, 0
   };

   // these functions are optional
   static const SQLUSMALLINT optFunc[] = {
      SQL_API_SQLNUMRESULTCOLS, SQL_API_SQLROWCOUNT, 0
   };

   SQLRETURN r;
   SQLUSMALLINT sup;

   int i;
   // check the required functions
   for (i = 0; reqFunc[i] != 0; ++i) {

      r = SQLGetFunctions(hDbc, reqFunc[i], &sup);

      if (r != SQL_SUCCESS) {
         qSqlWarning("QODBCDriver::checkDriver: Unable to retrieve the list of supported functions", this);
         return false;
      }

      if (sup == SQL_FALSE) {
         qWarning () << "QODBCDriver::open: Warning - Driver does not support all needed functionality ("
                     << reqFunc[i]
                     << ").\nRefer to the SQL Driver documentation for more information.";
         return false;
      }
   }

   // these functions are optional and just generate a warning
   for (i = 0; optFunc[i] != 0; ++i) {

      r = SQLGetFunctions(hDbc, optFunc[i], &sup);

      if (r != SQL_SUCCESS) {
         qSqlWarning("QODBCDriver::checkDriver: Unable to retrieve list of supported functions", this);
         return false;

      }
      if (sup == SQL_FALSE) {
         qWarning() << "QODBCDriver::checkDriver: Driver is missing some optional functions ("
                    << optFunc[i]
                    << ')';

         return true;
      }
   }
#endif

   return true;
}

void QODBCDriverPrivate::checkSchemaUsage()
{
   SQLRETURN   r;
   SQLUINTEGER val;

   r = SQLGetInfo(hDbc, SQL_SCHEMA_USAGE, (SQLPOINTER) &val, sizeof(val), nullptr);

   if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
      useSchema = (val != 0);
   }
}

void QODBCDriverPrivate::checkDBMS()
{
   SQLRETURN r;
   QVarLengthArray<SQLTCHAR> serverString(200);

   SQLSMALLINT t;
   memset(serverString.data(), 0, serverString.size() * sizeof(SQLTCHAR));

   r = SQLGetInfo(hDbc, SQL_DBMS_NAME, serverString.data(), serverString.size() * sizeof(SQLTCHAR), &t);

   if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
      const QString serverType = fromSQLTCHAR(serverString, t / sizeof(SQLTCHAR));

      if (serverType.contains("PostgreSQL", Qt::CaseInsensitive)) {
         dbmsType = QSqlDriver::PostgreSQL;

      } else if (serverType.contains("Oracle", Qt::CaseInsensitive)) {
         dbmsType = QSqlDriver::Oracle;

      } else if (serverType.contains("MySql", Qt::CaseInsensitive)) {
         dbmsType = QSqlDriver::MySqlServer;

      } else if (serverType.contains("Microsoft SQL Server", Qt::CaseInsensitive))  {
         dbmsType = QSqlDriver::MSSqlServer;

      } else if (serverType.contains("Sybase", Qt::CaseInsensitive))  {
         dbmsType = QSqlDriver::Sybase;
      }
   }

   r = SQLGetInfo(hDbc, SQL_DRIVER_NAME, serverString.data(), SQLSMALLINT(serverString.size() * sizeof(SQLTCHAR)), &t);

   if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
      const QString serverType = fromSQLTCHAR(serverString, t / sizeof(SQLTCHAR));

      isFreeTDSDriver = serverType.contains("tdsodbc", Qt::CaseInsensitive);
      unicode = unicode && !isFreeTDSDriver;
   }
}

void QODBCDriverPrivate::checkHasSQLFetchScroll()
{
   SQLUSMALLINT sup;
   SQLRETURN r = SQLGetFunctions(hDbc, SQL_API_SQLFETCHSCROLL, &sup);

   if ((r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) || sup != SQL_TRUE) {
      hasSQLFetchScroll = false;
      qWarning() << "QODBCDriver::checkHasSQLFetchScroll: Warning - Driver does not support scrollable "
         "result sets, use forward only mode for queries";
   }
}

void QODBCDriverPrivate::checkHasMultiResults()
{
   QVarLengthArray<SQLTCHAR> driverResponse(2);
   SQLSMALLINT length;

   SQLRETURN r = SQLGetInfo(hDbc, SQL_MULT_RESULT_SETS, driverResponse.data(),
         driverResponse.size() * sizeof(SQLTCHAR), &length);

   if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
      hasMultiResultSets = fromSQLTCHAR(driverResponse, length / sizeof(SQLTCHAR)).startsWith('Y');
   }
}

void QODBCDriverPrivate::checkDateTimePrecision()
{
   SQLINTEGER columnSize;
   SQLHANDLE hStmt;

   SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
   if (r != SQL_SUCCESS) {
      return;
   }

   r = SQLGetTypeInfo(hStmt, SQL_TIMESTAMP);
   if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
      r = SQLFetch(hStmt);

      if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
         if (SQLGetData(hStmt, 3, SQL_INTEGER, &columnSize, sizeof(columnSize), nullptr) == SQL_SUCCESS) {
            datetime_precision = (int)columnSize;
         }
      }
   }

   SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

QSqlResult *QODBCDriver::createResult() const
{
   Q_D(const QODBCDriver);
   return new QODBCResult(this, const_cast<QODBCDriverPrivate *>(d));
}

bool QODBCDriver::beginTransaction()
{
   Q_D(QODBCDriver);

   if (! isOpen()) {
      qWarning() << "QODBCDriver::beginTransaction: Database not open";
      return false;
   }

   SQLUINTEGER ac(SQL_AUTOCOMMIT_OFF);
   SQLRETURN r  = SQLSetConnectAttr(d->hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) static_cast<std::uintptr_t>(ac), sizeof(ac));

   if (r != SQL_SUCCESS) {
      setLastError(qMakeError(tr("Unable to disable autocommit"), QSqlError::TransactionError, d));
      return false;
   }

   return true;
}

bool QODBCDriver::commitTransaction()
{
   Q_D(QODBCDriver);

   if (! isOpen()) {
      qWarning() << "QODBCDriver::commitTransaction: Database not open";
      return false;
   }

   SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC, d->hDbc, SQL_COMMIT);

   if (r != SQL_SUCCESS) {
      setLastError(qMakeError(tr("Unable to commit transaction"), QSqlError::TransactionError, d));
      return false;
   }

   return endTrans();
}

bool QODBCDriver::rollbackTransaction()
{
   Q_D(QODBCDriver);

   if (! isOpen()) {
      qWarning() << "QODBCDriver::rollbackTransaction: Database not open";
      return false;
   }

   SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC, d->hDbc, SQL_ROLLBACK);

   if (r != SQL_SUCCESS) {
      setLastError(qMakeError(tr("Unable to rollback transaction"), QSqlError::TransactionError, d));
      return false;
   }

   return endTrans();
}

bool QODBCDriver::endTrans()
{
   Q_D(QODBCDriver);

   SQLUINTEGER ac(SQL_AUTOCOMMIT_ON);
   SQLRETURN r = SQLSetConnectAttr(d->hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) static_cast<std::uintptr_t>(ac), sizeof(ac));

   if (r != SQL_SUCCESS) {
      setLastError(qMakeError(tr("Unable to enable autocommit"), QSqlError::TransactionError, d));
      return false;
   }

   return true;
}

QStringList QODBCDriver::tables(QSql::TableType type) const
{
   Q_D(const QODBCDriver);

   QStringList list;
   if (! isOpen()) {
      return list;
   }

   SQLHANDLE hStmt;

   SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT, d->hDbc, &hStmt);
   if (r != SQL_SUCCESS) {
      qSqlWarning("QODBCDriver::tables: Unable to allocate handle", d);
      return list;
   }

   r = SQLSetStmtAttr(hStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, SQL_IS_UINTEGER);

   QStringList tableType;
   if (type & QSql::Tables) {
      tableType += "TABLE";
   }

   if (type & QSql::Views) {
      tableType += "VIEW";
   }

   if (type & QSql::SystemTables) {
      tableType += "SYSTEM TABLE";
   }

   if (tableType.isEmpty()) {
      return list;
   }

   QString joinedTableTypeString = tableType.join(",");

   r = SQLTables(hStmt, nullptr, 0, nullptr, 0, nullptr, 0,
         toSQLTCHAR(joinedTableTypeString).data(),
         joinedTableTypeString.length());          // characters, not bytes

   if (r != SQL_SUCCESS) {
      qSqlWarning("QODBCDriver::tables Unable to execute table list", d);
   }

   if (d->hasSQLFetchScroll)
      r = SQLFetchScroll(hStmt, SQL_FETCH_NEXT, 0);
   else {
      r = SQLFetch(hStmt);
   }

   if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO && r != SQL_NO_DATA) {
      qWarning() << "QODBCDriver::tables failed to retrieve table/view list: (" << r << ","
                 << cs_warnHandle(SQL_HANDLE_STMT, hStmt) << ")";
      return QStringList();
   }

   while (r == SQL_SUCCESS) {
      QString fieldVal = qGetStringData(hStmt, 2, -1, false);
      list.append(fieldVal);

      if (d->hasSQLFetchScroll)
         r = SQLFetchScroll(hStmt, SQL_FETCH_NEXT, 0);
      else {
         r = SQLFetch(hStmt);
      }
   }

   r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
   if (r != SQL_SUCCESS) {
      qSqlWarning("QODBCDriver: Unable to free statement handle" + QString::number(r), d);
   }

   return list;
}

QSqlIndex QODBCDriver::primaryIndex(const QString &tablename) const
{
   Q_D(const QODBCDriver);

   QSqlIndex index(tablename);
   if (! isOpen()) {
      return index;
   }

   bool usingSpecialColumns = false;
   QSqlRecord rec = record(tablename);

   SQLHANDLE hStmt;
   SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT, d->hDbc, &hStmt);

   if (r != SQL_SUCCESS) {
      qSqlWarning("QODBCDriver::primaryIndex: Unable to list primary key", d);
      return index;
   }

   QString catalog;
   QString schema;
   QString table;

   const_cast<QODBCDriverPrivate*>(d)->splitTableQualifier(tablename, catalog, schema, table);

   if (isIdentifierEscaped(catalog, QSqlDriver::TableName)) {
      catalog = stripDelimiters(catalog, QSqlDriver::TableName);
   } else {
      catalog = d->adjustCase(catalog);
   }

   if (isIdentifierEscaped(schema, QSqlDriver::TableName)) {
      schema = stripDelimiters(schema, QSqlDriver::TableName);
   } else {
      schema = d->adjustCase(schema);
   }

   if (isIdentifierEscaped(table, QSqlDriver::TableName)) {
      table = stripDelimiters(table, QSqlDriver::TableName);
   } else {
      table = d->adjustCase(table);
   }

   r = SQLSetStmtAttr(hStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, SQL_IS_UINTEGER);

   r = SQLPrimaryKeys(hStmt,
         catalog.length() == 0 ? nullptr : toSQLTCHAR(catalog).data(),
         catalog.length(),
         schema.length() == 0 ? nullptr : toSQLTCHAR(schema).data(),
         schema.length(), toSQLTCHAR(table).data(), table.length() );

   // if the SQLPrimaryKeys() call does not succeed (e.g the driver
   // does not support it) - try an alternative method to get hold of
   // the primary index (e.g MS Access and FoxPro)

   if (r != SQL_SUCCESS) {
      r = SQLSpecialColumns(hStmt, SQL_BEST_ROWID,
            catalog.length() == 0 ? nullptr : toSQLTCHAR(catalog).data(),
            catalog.length(),
            schema.length() == 0 ? nullptr : toSQLTCHAR(schema).data(),
            schema.length(), toSQLTCHAR(table).data(), table.length(), SQL_SCOPE_CURROW, SQL_NULLABLE);

      if (r != SQL_SUCCESS) {
         qSqlWarning("QODBCDriver::primaryIndex: Unable to execute primary key list", d);
      } else {
         usingSpecialColumns = true;
      }
   }

   if (d->hasSQLFetchScroll) {
      r = SQLFetchScroll(hStmt, SQL_FETCH_NEXT, 0);
   } else {
      r = SQLFetch(hStmt);
   }

   int fakeId = 0;
   QString cName;
   QString idxName;

   // Store all fields in a StringList because some drivers can not detail fields in this FETCH loop
   while (r == SQL_SUCCESS) {
      if (usingSpecialColumns) {
         cName   = qGetStringData(hStmt, 1, -1, d->unicode);   // column name
         idxName = QString::number(fakeId++);                  // invent a fake index name
      } else {
         cName   = qGetStringData(hStmt, 3, -1, d->unicode);   // column name
         idxName = qGetStringData(hStmt, 5, -1, d->unicode);   // pk index name
      }

      index.append(rec.field(cName));
      index.setName(idxName);

      if (d->hasSQLFetchScroll) {
         r = SQLFetchScroll(hStmt, SQL_FETCH_NEXT, 0);
      } else {
         r = SQLFetch(hStmt);
      }

   }

   r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
   if (r != SQL_SUCCESS) {
      qSqlWarning("QODBCDriver: Unable to free statement handle" + QString::number(r), d);
   }

   return index;
}

QSqlRecord QODBCDriver::record(const QString &tablename) const
{
   Q_D(const QODBCDriver);

   QSqlRecord retval;

   if (! isOpen()) {
      return retval;
   }

   SQLHANDLE hStmt;

   QString catalog;
   QString schema;
   QString table;

   const_cast<QODBCDriverPrivate*>(d)->splitTableQualifier(tablename, catalog, schema, table);

   if (isIdentifierEscaped(catalog, QSqlDriver::TableName)) {
      catalog = stripDelimiters(catalog, QSqlDriver::TableName);
   } else {
      catalog = d->adjustCase(catalog);
   }

   if (isIdentifierEscaped(schema, QSqlDriver::TableName)) {
      schema = stripDelimiters(schema, QSqlDriver::TableName);
   } else {
      schema = d->adjustCase(schema);
   }

   if (isIdentifierEscaped(table, QSqlDriver::TableName)) {
      table = stripDelimiters(table, QSqlDriver::TableName);
   } else {
      table = d->adjustCase(table);
   }

   SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT, d->hDbc, &hStmt);
   if (r != SQL_SUCCESS) {
      qSqlWarning("QODBCDriver::record: Unable to allocate handle", d);
      return retval;
   }

   r = SQLSetStmtAttr(hStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, SQL_IS_UINTEGER);

   r =  SQLColumns(hStmt,
         catalog.length() == 0 ? nullptr : toSQLTCHAR(catalog).data(),
         catalog.length(),
         schema.length() == 0 ? nullptr : toSQLTCHAR(schema).data(),
         schema.length(), toSQLTCHAR(table).data(), table.length(), nullptr, 0);

   if (r != SQL_SUCCESS) {
      qSqlWarning("QODBCDriver::record: Unable to execute column list", d);
   }

   if (d->hasSQLFetchScroll) {
      r = SQLFetchScroll(hStmt, SQL_FETCH_NEXT, 0);
   } else {
      r = SQLFetch(hStmt);
   }

   // Store all fields in a StringList because some drivers can not detail fields in this FETCH loop
   while (r == SQL_SUCCESS) {

      retval.append(qMakeFieldInfo(hStmt, d));

      if (d->hasSQLFetchScroll) {
         r = SQLFetchScroll(hStmt, SQL_FETCH_NEXT, 0);
      } else {
         r = SQLFetch(hStmt);
      }
   }

   r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
   if (r != SQL_SUCCESS) {
      qSqlWarning("QODBCDriver: Unable to free statement handle " + QString::number(r), d);
   }

   return retval;
}

QString QODBCDriver::formatValue(const QSqlField &field, bool trimStrings) const
{
   QString retval;

   if (field.isNull()) {
      retval = "NULL";

   } else if (field.type() == QVariant::DateTime) {
      // Use an escape sequence for the datetime fields
      if (field.value().toDateTime().isValid()) {
         QDate dt = field.value().toDateTime().date();
         QTime tm = field.value().toDateTime().time();

         // Dateformat has to be "yyyy-MM-dd hh:mm:ss", with leading zeroes if month or day < 10
         retval = "{ ts '" +
            QString::number(dt.year()) + '-' +
            QString::number(dt.month()).rightJustified(2, '0', true) + '-' +
            QString::number(dt.day()).rightJustified(2, '0', true)   + ' ' + tm.toString() + "' }";

      } else {
         retval = "NULL";
      }

   } else if (field.type() == QVariant::ByteArray) {
      QByteArray ba = field.value().toByteArray();
      QString res;

      static const char hexchars[] = "0123456789abcdef";

      for (int i = 0; i < ba.size(); ++i) {
         uchar s = (uchar) ba[i];
         res += QChar(hexchars[s >> 4]);
         res += QChar(hexchars[s & 0x0f]);
      }

      retval = "0x" + res;

   } else {
      retval = QSqlDriver::formatValue(field, trimStrings);

   }

   return retval;
}

QVariant QODBCDriver::handle() const
{
   Q_D(const QODBCDriver);

   return QVariant::fromValue<SQLHANDLE>(d->hDbc);
}

QString QODBCDriver::escapeIdentifier(const QString &identifier, IdentifierType) const
{
   Q_D(const QODBCDriver);

   QChar quote    = d->quoteChar();
   QString retval = identifier;

   if (! identifier.isEmpty() && ! identifier.startsWith(quote) && ! identifier.endsWith(quote) ) {
      retval.replace(quote, QString(quote) + quote);
      retval.prepend(quote).append(quote);
      retval.replace('.', QString(quote) + '.' + quote);
   }

   return retval;
}

bool QODBCDriver::isIdentifierEscapedImplementation(const QString &identifier, IdentifierType) const
{
   Q_D(const QODBCDriver);

   QChar quote = d->quoteChar();

   return identifier.size() > 2
      && identifier.startsWith(quote)   // left delimited
      && identifier.endsWith(quote);    // right delimited
}
