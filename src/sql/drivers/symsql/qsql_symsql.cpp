/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#include "qsql_symsql.h"


#define SYMBIAN_ENABLE_PUBLIC_PLATFORM_HEADER_SPLIT

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <qstringlist.h>
#include <qvector.h>
#include <qdebug.h>
#include "../../../corelib/kernel/qcore_symbian_p.h"

#if defined Q_OS_WIN
# include <qt_windows.h>
#else
# include <unistd.h>
#endif

#include <sqldb.h>
#include <e32capability.h>

const char* qCapabilityNames[ECapability_Limit] =
{
    "TCB",
    "CommDD",
    "PowerMgmt",
    "MultimediaDD",
    "ReadDeviceData",
    "WriteDeviceData",
    "DRM",
    "TrustedUI",
    "ProtServ",
    "DiskAdmin",
    "NetworkControl",
    "AllFiles",
    "SwEvent",
    "NetworkServices",
    "LocalServices",
    "ReadUserData",
    "WriteUserData",
    "Location",
    "SurroundingsDD",
    "UserEnvironment"
};

const char qCapabilityNone[] = "None";


Q_DECLARE_METATYPE(RSqlDatabase)
Q_DECLARE_METATYPE(RSqlStatement)

QT_BEGIN_NAMESPACE

const QString valueSeparator(QLatin1String("="));
const QString fieldSeparator(QLatin1String(","));

static QString _q_escapeIdentifier(const QString &identifier) 
{
    QString res = identifier;
    if (!identifier.isEmpty() 
            && identifier.left(1) != QString(QLatin1Char('"')) 
            && identifier.right(1) != QString(QLatin1Char('"'))) {
        res.replace(QLatin1Char('"'), QLatin1String("\"\""));
        res.prepend(QLatin1Char('"')).append(QLatin1Char('"'));
        res.replace(QLatin1Char('.'), QLatin1String("\".\""));
    }
    return res;
}

static QVariant::Type qGetColumnType(const TSqlColumnType coltype)
{
    switch(coltype){
    case ESqlInt:
    case ESqlInt64:
        return QVariant::Int;
    case ESqlReal:
        return QVariant::Double;
    case ESqlBinary:
        return QVariant::ByteArray;
    case ESqlText:
    case ESqlNull:
    default:
        return QVariant::String;
    }
}

static QVariant::Type qGetColumnType(const QString &tpName)
{
    const QString typeName = tpName.toLower();
    
    if (typeName == QLatin1String("integer")
        || typeName == QLatin1String("int"))
        return QVariant::Int;
    if (typeName == QLatin1String("double")
        || typeName == QLatin1String("float")
        || typeName == QLatin1String("real")
        || typeName.startsWith(QLatin1String("numeric")))
        return QVariant::Double;
    if (typeName == QLatin1String("blob"))
        return QVariant::ByteArray;
    return QVariant::String;
}

static QSqlError qMakeError(RSqlDatabase& access, 
    const QString &descr, 
    QSqlError::ErrorType type,
    int errorCode = -1)
{
    return QSqlError(descr,
                     QString::fromUtf16(static_cast<const ushort *>(access.LastErrorMessage().Ptr())),
                     type, 
                     errorCode);
}


static QSqlError gMakeErrorOpen(const QString &descr, 
    QSqlError::ErrorType type,
    TInt errorCode)
{
    return QSqlError(descr, QLatin1String(""), type, errorCode);
}

class QSymSQLDriverPrivate
{
public:
    inline QSymSQLDriverPrivate() {}
    RSqlDatabase access;
};

class QSymSQLResultPrivate
{
public:
    QSymSQLResultPrivate(QSymSQLResult *res);
    void cleanup();
    bool fetchNext(bool initialFetch);
    // initializes the recordInfo
    void initColumns(QSqlRecord& rec);
    void finalize();

    QSymSQLResult* q;
    RSqlDatabase access;
    RSqlStatement stmt;
    bool skipRow; // skip the next fetchNext()?
    bool skippedStatus; // the status of the fetchNext() that's skipped
    bool prepareCalled;
};

QSymSQLResultPrivate::QSymSQLResultPrivate(QSymSQLResult* res) : q(res),
    skipRow(false), 
    skippedStatus(false), 
    prepareCalled(false)      
{
}

void QSymSQLResultPrivate::cleanup()
{
    finalize();
    skippedStatus = false;
    skipRow = false;
    q->setAt(QSql::BeforeFirstRow);
    q->setActive(false);
}

void QSymSQLResultPrivate::finalize()
{
    prepareCalled = false;
    stmt.Close();
}

void QSymSQLResultPrivate::initColumns(QSqlRecord& rec)
{
    int nCols = stmt.ColumnCount();
    if (nCols <= 0) {
        q->setLastError(qMakeError(access, QCoreApplication::translate("QSymSQLResult",
                        "Error retrieving column count"), QSqlError::UnknownError, nCols));
        return;
    }
       
    for (int i = 0; i < nCols; ++i) {
        TPtrC cName;
        TInt err = stmt.ColumnName(i, cName);
        
        if (err != KErrNone) {
            q->setLastError(qMakeError(access, QCoreApplication::translate("QSymSQLResult",
                             "Error retrieving column name"), QSqlError::UnknownError, err));
             return;
        }
        
        QString colName = qt_TDesC2QString(cName);

        // must use typeName for resolving the type to match QSymSQLDriver::record
        TPtrC tName;
        TSqlColumnType decColType;
        err = stmt.DeclaredColumnType(i, decColType);
        
        if (err != KErrNone) {
            q->setLastError(qMakeError(access, QCoreApplication::translate("QSymSQLResult",
                             "Error retrieving column type"), QSqlError::UnknownError, err));
             return;
        }
        
        int dotIdx = colName.lastIndexOf(QLatin1Char('.'));
        QSqlField fld(colName.mid(dotIdx == -1 ? 0 : dotIdx + 1),  qGetColumnType(decColType));

        rec.append(fld);
    }
}

bool QSymSQLResultPrivate::fetchNext(bool initialFetch)
{
    int res;
    
    if (skipRow) {
        // already fetched
        Q_ASSERT(!initialFetch);
        skipRow = false;
        return skippedStatus;
    }
    
    skipRow = initialFetch; 
    res = stmt.Next();
    
    switch(res) {
    case KSqlAtRow: 
        return true;
    case KSqlAtEnd:
        stmt.Reset();
        return false;
    case KSqlErrGeneral:
        // KSqlErrGeneral is a generic error code and we must call stmt.Reset()
        // to get the specific error message.
        stmt.Reset();
        q->setLastError(qMakeError(access, QCoreApplication::translate("QSymSQLResult",
                        "Unable to fetch row"), QSqlError::ConnectionError, res));
        q->setAt(QSql::AfterLastRow);
        return false;
    case KSqlErrMisuse:
    case KSqlErrBusy:
    default:
        // something wrong, don't get col info, but still return false
        q->setLastError(qMakeError(access, QCoreApplication::translate("QSymSQLResult",
                        "Unable to fetch row"), QSqlError::ConnectionError, res));
        stmt.Reset();
        q->setAt(QSql::AfterLastRow);
        return false;
    }
    return false;
}

////////////////////////////////// QSymSQLResult /////////////////////////////////////////////////

QSymSQLResult::QSymSQLResult(const QSymSQLDriver* db)
    : QSqlResult(db)
{
    d = new QSymSQLResultPrivate(this);
    d->access = db->d->access;
}


QSymSQLResult::~QSymSQLResult()
{
    d->cleanup();
    delete d;
}

bool QSymSQLResult::reset(const QString &query)
{
    if (!prepare(query))
        return false;
    
    return exec();
}

bool QSymSQLResult::prepare(const QString &query)
{
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
        return false;

    d->cleanup();
    setSelect(false);

    TInt res = d->stmt.Prepare(d->access, qt_QString2TPtrC(query));
    
    if (res != KErrNone) {
        setLastError(qMakeError(d->access, QCoreApplication::translate("QSymSQLResult",
                     "Unable to execute statement"), QSqlError::StatementError, res));
        d->finalize();
        return false;
    }
    
    d->prepareCalled = true;
    
    return true;
}

bool QSymSQLResult::exec()
{
    if (d->prepareCalled == false) {
        setLastError(qMakeError(d->access, QCoreApplication::translate("QSymSQLResult",
                        "Statement is not prepared"), QSqlError::StatementError, KErrGeneral));
        return false;
    }
    
    const QVector<QVariant> values = boundValues();
    
    d->skippedStatus = false;
    d->skipRow = false;
    setAt(QSql::BeforeFirstRow);
    setLastError(QSqlError());
    int res = d->stmt.Reset();
    
    if (res != KErrNone) {
        setLastError(qMakeError(d->access, QCoreApplication::translate("QSymSQLResult",
                     "Unable to reset statement"), QSqlError::StatementError, res));
        d->finalize();
        return false;
    }
    TPtrC tmp;
    TInt paramCount = 0;
    while (d->stmt.ParamName(paramCount, tmp) == KErrNone) 
        paramCount++;

    if (paramCount == values.count()) {
        for (int i = 0; i < paramCount; ++i) {
            res = KErrNone;
            const QVariant value = values.at(i);

            if (value.isNull()) {
                res = d->stmt.BindNull(i); //replaced i + 1 with i
            } else {
                switch (value.type()) {
                case QVariant::ByteArray: {
                    const QByteArray *ba = static_cast<const QByteArray*>(value.constData());
                    TPtrC8 data(reinterpret_cast<const TUint8 *>(ba->constData()), ba->length());
                    res = d->stmt.BindBinary(i, data); //replaced i + 1 with i
                    break; }
                case QVariant::Int:
                    res = d->stmt.BindInt(i, value.toInt()); //replaced i + 1 with i
                    break;
                case QVariant::Double:
                    res = d->stmt.BindReal(i, value.toDouble()); //replaced i + 1 with i

                    break;
                case QVariant::UInt:
                case QVariant::LongLong:
                    res = d->stmt.BindReal(i, value.toLongLong()); //replaced i + 1 with i
                    break;
                
                case QVariant::String: {
                    // lifetime of string == lifetime of its qvariant
                    const QString *str = static_cast<const QString*>(value.constData());
                    res = d->stmt.BindText(i, qt_QString2TPtrC(*str)); // replaced i + 1 with i
                    break; }
                default: {
                    QString str = value.toString();
                    res = d->stmt.BindText(i, qt_QString2TPtrC(str)); //replaced i + 1 with i
                    break; }
                }
            }
            if (res != KErrNone) {
                setLastError(qMakeError(d->access, QCoreApplication::translate("QSymSQLResult",
                             "Unable to bind parameters"), QSqlError::StatementError, res));
                d->finalize();
                return false;
            }
        }
    } else {
        setLastError(QSqlError(QCoreApplication::translate("QSymSQLResult",
                        "Parameter count mismatch"), QString(), QSqlError::StatementError));
        return false;
    }
    
    d->skippedStatus = d->fetchNext(true);
    
    if (lastError().isValid()) {
        setSelect(false);
        setActive(false);
        return false;
    }
    
    if (d->stmt.ColumnCount() > 0) {
        //If there is something, it has to be select
        setSelect(true);
    } else {
        //If there isn't it might be just bad query, let's check manually whether we can find SELECT
       QString query = this->lastQuery();
       query = query.trimmed();
       query = query.toLower();
        
       //Just check whether there is one in the beginning, don't know if this is enough
       //Comments should be at the end of line if those are passed
       //For some reason, case insensitive indexOf didn't work for me
       if (query.indexOf(QLatin1String("select")) == 0) {
           setSelect(true);
       } else {
           setSelect(false);
       }
    }

    setActive(true);
    return true;
}


int QSymSQLResult::size()
{
    return -1;
}

int QSymSQLResult::numRowsAffected()
{   
    return -1;
}

QVariant QSymSQLResult::lastInsertId() const
{
    if (isActive()) {
        qint64 id = static_cast<qint64>(d->access.LastInsertedRowId());
        if (id)
            return id;
    }

    return QVariant();
}

QSqlRecord QSymSQLResult::record() const
{
    if (!isActive() || !isSelect())
        return QSqlRecord();
    
    QSqlRecord res;
    d->initColumns(res);
            
    return res;
}

QVariant QSymSQLResult::handle() const
{
    return qVariantFromValue(d->stmt);
}

    
void QSymSQLResult::virtual_hook(int id, void *data)
{
    switch (id) 
    {
        case QSqlResult::DetachFromResultSet:
            d->stmt.Reset();
            break;
        default:
            QSqlResult::virtual_hook(id, data);
    }  
}

QVariant QSymSQLResult::data(int idx)
{
    QVariant r;
    
    switch (d->stmt.ColumnType(idx)) {
    case ESqlBinary:
        {
            TPtrC8 data; 
            d->stmt.ColumnBinary(idx, data);
            return QByteArray(reinterpret_cast<const char *>(data.Ptr()), data.Length());
            break;
         }
    case ESqlInt:
        r = QVariant(d->stmt.ColumnInt(idx));
        break;
    case ESqlInt64:
        r = QVariant(d->stmt.ColumnInt64(idx));
        break;
    case ESqlReal:
        switch(numericalPrecisionPolicy()) {
        case QSql::LowPrecisionInt32:
            r = QVariant(d->stmt.ColumnInt(idx));
            break;
        case QSql::LowPrecisionInt64:
            r = QVariant(d->stmt.ColumnInt64(idx));
            break;
        case QSql::LowPrecisionDouble:
            r = QVariant(d->stmt.ColumnReal(idx));
            break;
        case QSql::HighPrecision:
        default:
            TPtrC res;
            d->stmt.ColumnText(idx, res);
            r = QVariant(qt_TDesC2QString(res));
            break;
        };
        break;
    case ESqlNull:
        r  = QVariant(QVariant::String);
        break;
    default:
        TPtrC res;
        d->stmt.ColumnText(idx, res);
        r = QVariant(qt_TDesC2QString(res));
        break;
    }

    return r;
}    
    
bool QSymSQLResult::isNull(int i)
{
    return d->stmt.IsNull(i);
}

bool QSymSQLResult::fetch(int i)
{
    //Single return point modified according to review
    bool retVal = true;
    
    if (i < 0 || !isActive()) {
        retVal = false;
    } else {
        if (at() <= -1 || i < at()) {
            d->stmt.Reset();
            setAt(-1);
            d->skipRow = false;
        }
    
        while (at() < i) {
            if (!d->fetchNext(false)) {
                retVal = false;
                break;
            }
        
            setAt(at() + 1);
        }
    }
    
    return retVal;
}

bool QSymSQLResult::fetchNext()
{   
    bool res = d->fetchNext(false);
    if (res) {
        setAt(at()+1);
    }
    
    return res;
}

bool QSymSQLResult::fetchPrevious()
{
    return QSqlResult::fetchPrevious();
}

bool QSymSQLResult::fetchFirst()
{
    return fetch(0);
}

bool QSymSQLResult::fetchLast()
{
    if (!isActive())
        return false;

    if (at() <= -1) {
        if (!fetchFirst())
            return false;
    }
    
    TInt res;
    
    do {
        res = d->stmt.Next();
        setAt(at()+1);
    } while (res == KSqlAtRow);
        
    if (res != KSqlAtEnd)
        return false;
    
    d->skippedStatus = false;
    d->skipRow = false;
    
    return fetchPrevious();
}
////////////////////////////////// QSymSQLDriver //////////////////////////////////////////

QSymSQLDriver::QSymSQLDriver(QObject * parent)
    : QSqlDriver(parent)
{
    d = new QSymSQLDriverPrivate();
}

QSymSQLDriver::QSymSQLDriver(RSqlDatabase& connection, QObject *parent)
    : QSqlDriver(parent)
{
    d = new QSymSQLDriverPrivate();
    d->access = connection;
    setOpen(true);
    setOpenError(false);
}


QSymSQLDriver::~QSymSQLDriver()
{
    d->access.Close();
    delete d;
}

bool QSymSQLDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case BLOB:
    case Transactions:
    case Unicode:
    case PreparedQueries:
    case PositionalPlaceholders:
    case SimpleLocking:
    case FinishQuery:
    case LowPrecisionNumbers:
    case LastInsertId:
    case NamedPlaceholders:
        return true;
    case QuerySize:
    case BatchOperations:
    case EventNotifications:
    case MultipleResultSets:
        return false;
    }
    return false;
}

/*!
    Converts capability string to TCapability
*/
TCapability qMatchCapStr(QString& str)
{
    TCapability cap = ECapability_HardLimit;
    
    for (int i = 0; i < static_cast<int>(ECapability_Limit); i++) {
        if (str.compare(QLatin1String(qCapabilityNames[i]), Qt::CaseInsensitive) == 0) {
            cap = static_cast<TCapability>(i);
            break;
        }
    }
    
    //Special case, we allow ECapability_None to be defined
    if (cap == ECapability_HardLimit
        && str.compare(QLatin1String(qCapabilityNone), Qt::CaseInsensitive) == 0) 
        cap = ECapability_None;
    
    return cap;
}

bool qExtractSecurityPolicyFromString(const QString &string, TSecurityPolicy &policy)
{
    int startPos = string.indexOf(QLatin1Char('='));
    QStringList values;
    bool ret = false;
    
    if (startPos == -1) {
        values = string.split(QLatin1Char(','), QString::SkipEmptyParts);
    } else {
        values = string.mid(startPos + 1).split(QLatin1Char(','), QString::SkipEmptyParts);
    }
    
    if (values.count() > 0) {
        const QString findVid(QLatin1String("vid["));
        const QString findSid(QLatin1String("sid["));
        const int MaxCapCount = 7;
        const int VidMaxCount = 3;
        const int SidMaxCount = 3;

        TCapability capList[MaxCapCount] = { ECapability_None,ECapability_None,ECapability_None,
                                    ECapability_None,ECapability_None,ECapability_None,ECapability_None };
        
        bool isVID = false;
        bool isSID = false;
        
        QString idString(QLatin1String(""));
        int maxAllowed = MaxCapCount;
        
        if (values[0].contains(findVid, Qt::CaseInsensitive)) {
            idString = values[0].remove(findVid, Qt::CaseInsensitive);
            idString = idString.remove(QLatin1Char(']'));
            values.removeAt(0);
            isVID = true;
            maxAllowed = VidMaxCount;
            
        } else if (values[0].contains(findSid, Qt::CaseInsensitive)) {
            idString = values[0].remove(findSid, Qt::CaseInsensitive);
            idString = idString.remove(QLatin1Char(']'));
            values.removeAt(0);
            isSID = true;
            maxAllowed = SidMaxCount;
        }
        
        if (values.count() <= maxAllowed) {
            bool wasSuccesful = true;
            
            for (int i = 0; i < values.count(); i++) {
                capList[i] = qMatchCapStr(values[i]);
                
                if (capList[i] == ECapability_HardLimit) {
                    wasSuccesful = false;
                    break;
                }
            }
            
            if (wasSuccesful) {
                if (isVID || isSID){
                    bool ok = true;
                    quint32 id = idString.toUInt(&ok, 16);
                    
                    if (ok) {
                        if (isVID) {
                            TVendorId vid(id);
                            policy = TSecurityPolicy(vid, capList[0], capList[1], capList[2]);
                        } else {
                            TSecureId sid(id);
                            policy = TSecurityPolicy(sid, capList[0], capList[1], capList[2]);
                        }
                        
                        ret = true; //Everything is fine
                    }   
                } else {
                    policy = TSecurityPolicy(capList[0], capList[1], capList[2], capList[3], 
                                        capList[4], capList[5], capList[6]);
                    
                    ret = true;  //Everything is fine
                }
            }
        }
    } 
    
    return ret;
}

/*!   
    Opens the database connection using the given connection options. Returns true on success; otherwise returns false.
    Error information can be retrieved using the lastError() function. Symbian SQL dbs have no \a user, \a password, \a host
    or \a port just file names.
    
    \a connOpts Connection options hold definition for security policies and all parameters that does not contain "POLICY_" will be
    passed to RSqlDatabase. Policy will be filled according to parsed values.

    Value in database wide parameters starts by definition which can be vendorId or secureId. These come directly from TSecurityPolicy class in Symbian. 
    
    POLICY_DB_DEFAULT
    Default security policy which will be used for the database and all database objects. POLICY_DB_DEFAULT must be 
    defined before any other policy definitions can be used.    
    POLICY_DB_READ
    Read database security policy. An application with read database security policy can read from database.    
    POLICY_DB_WRITE:
    Write database security policy. An application with write database security policy can write to database. 
    POLICY_DB_SCHEMA:
    Schema database security policy. An application with schema database security policy can modify
    the database schema, write to database, read from database.   
    
    Format:
    POLICY_DB_DEFAULT=cap1,cap2,cap3,cap4,cap5,cap6,cap7    (Up to 7 capabilities) 
    POLICY_DB_READ=cap1,cap2,cap3,cap4,cap5,cap6,cap7       (Up to 7 capabilities)
    POLICY_DB_WRITE=vendorid,cap1,cap2,cap3                 (Vendor ID and up to 3 capabilities)
    POLICY_DB_SCHEMA=secureid,cap1,cap2,cap3                (Secure ID and up to 3 capabilities)  
    
    Table policies does not support schema policy as database level does.
    
    Table specific parameters would be as:    
    POLICY_TABLE_WRITE=tablename,cap1,cap2,cap3,cap4,cap5,cap6,cap7
    POLICY_TABLE_READ=tablename,cap1,cap2,cap3,cap4,cap5,cap6,cap7    
    
    Vendor Id and Secure id format:    
    vid[0x12345678]                              (Hex)    
    sid[0x12345678]                              (Hex)                        
    
    Example:   
    \code   
    QSqlDatabase database = QSqlDatabase::addDatabase("QSYMSQL", "MyConnection");
    database.setConnectOptions("POLICY_DB_DEFAULT=ReadDeviceData");
    database.setDatabaseName("[12345678]myDatabase");
    bool ok = database.open();   
    \encode    
    
    \code   
    QSqlDatabase database = QSqlDatabase::addDatabase("QSYMSQL", "MyConnection");
    database.setConnectOptions("POLICY_DB_DEFAULT=None; POLICY_DB_WRITE=sid[0x12345678], WriteDeviceData");
    database.setDatabaseName("[12345678]myDatabase");
    bool ok = database.open();   
    \encode    
    
    FOREIGN KEY:  
    Enabling foreign key support from underlying SQLite
    add: "foreign_keys = ON" to your connection options string. This will be passes to SQLite.
   
    Foreign key Example:   
    \code
    QSqlDatabase database = QSqlDatabase::addDatabase("QSYMSQL", "MyConnection");
    database.setDatabaseName("[12345678]myDatabase");
    database.setConnectOptions("foreign_keys = ON");
    bool ok = database.open();
    \encode
    
   More information about Symbian Security Policy can be found from Symbian documentation.
    
*/
bool QSymSQLDriver::open(const QString & db, const QString &, const QString &, const QString &, int, const QString &conOpts)
{                                
    if (isOpen())
        close();
    if (db.isEmpty())
        return false;
    
    //Separating our parameters from Symbian ones and construct new connection options
    const QString itemSeparator(QLatin1String(";"));
    QRegExp isOurOption(QLatin1String("POLICY_*"), Qt::CaseInsensitive, QRegExp::Wildcard); 
    
    QStringList optionList = conOpts.split(itemSeparator, QString::SkipEmptyParts);
    QStringList symbianList;
    
    for (int i = optionList.count() - 1; i >= 0; i--) {
        if (!optionList[i].contains(isOurOption)) {
            symbianList.append(optionList[i]);
            optionList.removeAt(i);
        } else {
            //Removing whitespace
            QString formatted = optionList[i];
            formatted = formatted.remove(QLatin1Char(' '));
            formatted = formatted.remove(QLatin1Char('\t'));
            formatted = formatted.remove(QLatin1Char('\n'));
            formatted = formatted.remove(QLatin1Char('\r'));
            optionList[i] = formatted;
        }
    }
    
    QString symbianOpt;
    
    for (int i = 0; i < symbianList.count(); i++) {
        symbianOpt += symbianList[i];
        symbianOpt += itemSeparator;
    }

    TPtrC dbName(qt_QString2TPtrC(db));
    QByteArray conOpts8 = symbianOpt.toUtf8();    
    const TPtrC8 config(reinterpret_cast<const TUint8*>(conOpts8.constData()), (conOpts8.length()));
    
    TInt res = d->access.Open(dbName, &config);
    
    if (res == KErrNotFound) {

        QRegExp findDefault(QLatin1String("POLICY_DB_DEFAULT=*"), Qt::CaseInsensitive, QRegExp::Wildcard);
        QRegExp findRead(QLatin1String("POLICY_DB_READ=*"), Qt::CaseInsensitive, QRegExp::Wildcard);
        QRegExp findWrite(QLatin1String("POLICY_DB_WRITE=*"), Qt::CaseInsensitive, QRegExp::Wildcard);
        QRegExp findSchema(QLatin1String("POLICY_DB_SCHEMA=*"), Qt::CaseInsensitive, QRegExp::Wildcard);
        QRegExp findTableRead(QLatin1String("POLICY_TABLE_READ=*"), Qt::CaseInsensitive, QRegExp::Wildcard);
        QRegExp findTableWrite(QLatin1String("POLICY_TABLE_WRITE=*"), Qt::CaseInsensitive, QRegExp::Wildcard);
               
        int policyIndex = optionList.indexOf(findDefault);
        
        if (policyIndex != -1) {
            QString defaultPolicyString = optionList[policyIndex];
            optionList.removeAt(policyIndex);
            
            TSecurityPolicy policyItem;
            
            if (qExtractSecurityPolicyFromString(defaultPolicyString, policyItem)) {
                RSqlSecurityPolicy policy;
                res = policy.Create(policyItem);
                
                if (res == KErrNone) {
                    for (int i = 0; i < optionList.count(); i++) {
                        QString option = optionList[i];
                        
                        if (option.contains(findRead)) {
                            if (qExtractSecurityPolicyFromString(option, policyItem)) {
                                res = policy.SetDbPolicy(RSqlSecurityPolicy::EReadPolicy, policyItem);
                            } else {
                                res = KErrArgument;
                            }
                        } else if (option.contains(findWrite)) {
                            if (qExtractSecurityPolicyFromString(option, policyItem)) {
                                res = policy.SetDbPolicy(RSqlSecurityPolicy::EWritePolicy, policyItem);
                            } else {
                                res = KErrArgument;
                            }
                        } else if (option.contains(findSchema)) {
                            if (qExtractSecurityPolicyFromString(option, policyItem)) {
                                res = policy.SetDbPolicy(RSqlSecurityPolicy::ESchemaPolicy, policyItem);
                            } else {
                                res = KErrArgument;
                            }
                        } else if (option.contains(findTableWrite)) {
                            QString tableOption = option.mid(option.indexOf(QLatin1Char('=')) + 1);
                            int firstComma = tableOption.indexOf(QLatin1Char(','));
                            
                            if (firstComma != -1) {
                                QString tableName = tableOption.left(firstComma);
                                tableOption = tableOption.mid(firstComma + 1);
                                
                                if (qExtractSecurityPolicyFromString(tableOption, policyItem)) {
                                    TPtrC symTableName(qt_QString2TPtrC(tableName));
                                                                
                                    res = policy.SetPolicy(RSqlSecurityPolicy::ETable, symTableName,
                                                        RSqlSecurityPolicy::EWritePolicy, policyItem);
                                } else {
                                    res = KErrArgument;
                                }
                            } else {
                                res = KErrArgument;
                            }
                        } else if (option.contains(findTableRead)) {
                            QString tableOption = option.mid(option.indexOf(QLatin1Char('=')) + 1);
                            int firstComma = tableOption.indexOf(QLatin1Char(','));
                            
                            if (firstComma != -1) {
                                QString tableName = tableOption.left(firstComma);
                                tableOption = tableOption.mid(firstComma + 1);
                                
                                if (qExtractSecurityPolicyFromString(tableOption, policyItem)) {
                                    TPtrC symTableName(qt_QString2TPtrC(tableName));
                                                                
                                    res = policy.SetPolicy(RSqlSecurityPolicy::ETable, symTableName,
                                                        RSqlSecurityPolicy::EReadPolicy, policyItem);
                                } else {
                                    res = KErrArgument;
                                }
                            } else {
                                res = KErrArgument;
                            }
                        } else {
                            res = KErrArgument;
                        }
                        
                        if (res != KErrNone) {
                            setLastError(gMakeErrorOpen(tr("Invalid option: ") + option, QSqlError::ConnectionError, res));
                            break;
                        }
                    }
                    
                    if (res == KErrNone) {
                        res = d->access.Create(dbName, policy, &config);
                        policy.Close();
                        
                        if (res != KErrNone) 
                            setLastError(gMakeErrorOpen(tr("Error opening database"), QSqlError::ConnectionError, res));
                    }
                }
                
            } else {
                res = KErrArgument;
                setLastError(gMakeErrorOpen(tr("Invalid option: ") + defaultPolicyString, QSqlError::ConnectionError, res));
            }
            
        } else {
            //Check whether there is some of our options, fail if so.
            policyIndex = optionList.indexOf(isOurOption);
            
            if (policyIndex == -1) {
                res = d->access.Create(dbName, &config);
            
                if (res != KErrNone) 
                    setLastError(gMakeErrorOpen(tr("Error opening database"), QSqlError::ConnectionError, res));
            } else {
                res = KErrArgument;
                setLastError(gMakeErrorOpen(tr("POLICY_DB_DEFAULT must be defined before any other POLICY definitions can be used"), QSqlError::ConnectionError, res));
            }
        }
    }
    
    if (res == KErrNone) {
        setOpen(true);
        setOpenError(false);
        return true;
    } else {
        setOpenError(true);
        return false;
    }
}

void QSymSQLDriver::close()
{
    if (isOpen()) {
        d->access.Close();
        setOpen(false);
        setOpenError(false);
    }
}

QSqlResult *QSymSQLDriver::createResult() const
{
    return new QSymSQLResult(this);
}

bool QSymSQLDriver::beginTransaction()
{
    if (!isOpen() || isOpenError())
        return false;
    
    TInt err = d->access.Exec(_L("BEGIN"));
    if (err < KErrNone) {
        setLastError(QSqlError(tr("Unable to begin transaction"),
                qt_TDesC2QString(d->access.LastErrorMessage()), QSqlError::TransactionError, err));
        return false;
    }

    return true;
}

bool QSymSQLDriver::commitTransaction()
{
    if (!isOpen() || isOpenError())
        return false;
    
    TInt err = d->access.Exec(_L("COMMIT"));
    if (err < KErrNone) {
        setLastError(QSqlError(tr("Unable to commit transaction"),
                qt_TDesC2QString(d->access.LastErrorMessage()), QSqlError::TransactionError, err));
        return false;
    }    
    
    return true;
}

bool QSymSQLDriver::rollbackTransaction()
{
    if (!isOpen() || isOpenError())
        return false;

    TInt err = d->access.Exec(_L("ROLLBACK"));
    if (err < KErrNone) {
        setLastError(QSqlError(tr("Unable to rollback transaction"),
                qt_TDesC2QString(d->access.LastErrorMessage()), QSqlError::TransactionError, err));
        return false;
    }    
    
    return true;
}

QStringList QSymSQLDriver::tables(QSql::TableType type) const
{
    QStringList res;
    if (!isOpen())
        return res;

    QSqlQuery q(createResult());
    q.setForwardOnly(true);

    QString sql = QLatin1String("SELECT name FROM sqlite_master WHERE %1 "
                                "UNION ALL SELECT name FROM sqlite_temp_master WHERE %1");
    if ((type & QSql::Tables) && (type & QSql::Views))
        sql = sql.arg(QLatin1String("type='table' OR type='view'"));
    else if (type & QSql::Tables)
        sql = sql.arg(QLatin1String("type='table'"));
    else if (type & QSql::Views)
        sql = sql.arg(QLatin1String("type='view'"));
    else
        sql.clear();

    if (!sql.isEmpty() && q.exec(sql)) {
        while (q.next())
            res.append(q.value(0).toString());
    }

    if (type & QSql::SystemTables) 
        // there are no internal tables beside this one:
        res.append(QLatin1String("sqlite_master"));

    return res;
}

static QSqlIndex qGetTableInfo(QSqlQuery &q, QString &tableName, bool onlyPIndex = false)
{   
    QString dbName;
    QString table(tableName);
    int indexOfSeparator = tableName.indexOf(QLatin1Char('.'));
    if (indexOfSeparator > -1) {
        dbName = tableName.left(indexOfSeparator +1 );
        table = tableName.mid(indexOfSeparator + 1);
    }
    q.exec(QLatin1String("PRAGMA ") + dbName + QLatin1String("table_info (") + _q_escapeIdentifier(table) + QLatin1String(")"));

    const int NAME_IDX = 1;
    const int TYPE_IDX = 2;
    const int NOTNULL_IDX = 3;
    const int DFLT_VALUE_IDX = 4;
    const int PK_IDX = 5;    
    
    QSqlIndex ind;
    while (q.next()) {
        bool isPk = q.value(PK_IDX).toInt();
        if (onlyPIndex && !isPk)
            continue;
        QString typeName = q.value(TYPE_IDX).toString().toLower();
        QSqlField fld(q.value(NAME_IDX).toString(), qGetColumnType(typeName));
        if (isPk && (typeName == QLatin1String("integer")))
            // INTEGER PRIMARY KEY fields are auto-generated in sqlite
            // INT PRIMARY KEY is not the same as INTEGER PRIMARY KEY!
            fld.setAutoValue(true);
        fld.setRequired(q.value(NOTNULL_IDX).toInt() != 0);
        fld.setDefaultValue(q.value(DFLT_VALUE_IDX));
        ind.append(fld);
    }
    return ind;
}

QSqlIndex QSymSQLDriver::primaryIndex(const QString &tblname) const
{
    if (!isOpen())
        return QSqlIndex();

    QString table = tblname;
    if (isIdentifierEscaped(table, QSqlDriver::TableName))
        table = stripDelimiters(table, QSqlDriver::TableName);

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    return qGetTableInfo(q, table, true);
}

QSqlRecord QSymSQLDriver::record(const QString &tbl) const
{
    if (!isOpen())
        return QSqlRecord();

    QString table = tbl;
    if (isIdentifierEscaped(table, QSqlDriver::TableName))
        table = stripDelimiters(table, QSqlDriver::TableName);

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    return qGetTableInfo(q, table);
}

QVariant QSymSQLDriver::handle() const
{
    return qVariantFromValue(d->access);
}

QString QSymSQLDriver::escapeIdentifier(const QString &identifier, IdentifierType type) const
{
    Q_UNUSED(type);
    return _q_escapeIdentifier(identifier);
}

QT_END_NAMESPACE
