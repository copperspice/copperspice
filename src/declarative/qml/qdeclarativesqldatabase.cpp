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

#include <qdeclarativesqldatabase_p.h>
#include <qdeclarativeengine.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativerefcount_p.h>
#include <qdeclarativeengine_p.h>
#include <QtCore/qobject.h>
#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptvalueiterator.h>
#include <QtScript/qscriptcontext.h>
#include <QtScript/qscriptengine.h>
#include <QtScript/qscriptclasspropertyiterator.h>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlerror.h>
#include <QtSql/qsqlrecord.h>
#include <QtCore/qstack.h>
#include <QtCore/qcryptographichash.h>
#include <QtCore/qsettings.h>
#include <QtCore/qdir.h>
#include <QtCore/qdebug.h>

Q_DECLARE_METATYPE(QSqlDatabase)
Q_DECLARE_METATYPE(QSqlQuery)

QT_BEGIN_NAMESPACE

class QDeclarativeSqlQueryScriptClass: public QScriptClass
{
 public:
   QDeclarativeSqlQueryScriptClass(QScriptEngine *engine) : QScriptClass(engine) {
      str_length = engine->toStringHandle(QLatin1String("length"));
      str_forwardOnly = engine->toStringHandle(QLatin1String("forwardOnly")); // not in HTML5 (an optimization)
   }

   QueryFlags queryProperty(const QScriptValue &,
                            const QScriptString &name,
                            QueryFlags flags, uint *) {
      if (flags & HandlesReadAccess) {
         if (name == str_length) {
            return HandlesReadAccess;
         } else if (name == str_forwardOnly) {
            return flags;
         }
      }
      if (flags & HandlesWriteAccess)
         if (name == str_forwardOnly) {
            return flags;
         }
      return 0;
   }

   QScriptValue property(const QScriptValue &object,
                         const QScriptString &name, uint) {
      QSqlQuery query = qscriptvalue_cast<QSqlQuery>(object.data());
      if (name == str_length) {
         int s = query.size();
         if (s < 0) {
            // Inefficient.
            if (query.last()) {
               return query.at() + 1;
            } else {
               return 0;
            }
         } else {
            return s;
         }
      } else if (name == str_forwardOnly) {
         return query.isForwardOnly();
      }
      return engine()->undefinedValue();
   }

   void setProperty(QScriptValue &object,
                    const QScriptString &name, uint, const QScriptValue &value) {
      if (name == str_forwardOnly) {
         QSqlQuery query = qscriptvalue_cast<QSqlQuery>(object.data());
         query.setForwardOnly(value.toBool());
      }
   }

   QScriptValue::PropertyFlags propertyFlags(const QScriptValue &/*object*/, const QScriptString &name, uint /*id*/) {
      if (name == str_length) {
         return QScriptValue::Undeletable
                | QScriptValue::SkipInEnumeration;
      }
      return QScriptValue::Undeletable;
   }

 private:
   QScriptString str_length;
   QScriptString str_forwardOnly;
};

// If the spec changes to allow iteration, check git history...
// class QDeclarativeSqlQueryScriptClassPropertyIterator : public QScriptClassPropertyIterator



enum SqlException {
   UNKNOWN_ERR,
   DATABASE_ERR,
   VERSION_ERR,
   TOO_LARGE_ERR,
   QUOTA_ERR,
   SYNTAX_ERR,
   CONSTRAINT_ERR,
   TIMEOUT_ERR
};

static const char *sqlerror[] = {
   "UNKNOWN_ERR",
   "DATABASE_ERR",
   "VERSION_ERR",
   "TOO_LARGE_ERR",
   "QUOTA_ERR",
   "SYNTAX_ERR",
   "CONSTRAINT_ERR",
   "TIMEOUT_ERR",
   0
};

#define THROW_SQL(error, desc) \
{ \
    QScriptValue errorValue = context->throwError(desc); \
    errorValue.setProperty(QLatin1String("code"), error); \
    return errorValue; \
}

static QString qmlsqldatabase_databasesPath(QScriptEngine *engine)
{
   QDeclarativeScriptEngine *qmlengine = static_cast<QDeclarativeScriptEngine *>(engine);
   return QDir::toNativeSeparators(qmlengine->offlineStoragePath)
          + QDir::separator() + QLatin1String("Databases");
}

static void qmlsqldatabase_initDatabasesPath(QScriptEngine *engine)
{
   QDir().mkpath(qmlsqldatabase_databasesPath(engine));
}

static QString qmlsqldatabase_databaseFile(const QString &connectionName, QScriptEngine *engine)
{
   return qmlsqldatabase_databasesPath(engine) + QDir::separator()
          + connectionName;
}


static QScriptValue qmlsqldatabase_item(QScriptContext *context, QScriptEngine *engine)
{
   QSqlQuery query = qscriptvalue_cast<QSqlQuery>(context->thisObject().data());
   int i = context->argument(0).toNumber();
   if (query.at() == i || query.seek(i)) { // Qt 4.6 doesn't optimize seek(at())
      QSqlRecord r = query.record();
      QScriptValue row = engine->newObject();
      for (int j = 0; j < r.count(); ++j) {
         row.setProperty(r.fieldName(j), QScriptValue(engine, r.value(j).toString()));
      }
      return row;
   }
   return engine->undefinedValue();
}

static QScriptValue qmlsqldatabase_executeSql_outsidetransaction(QScriptContext *context, QScriptEngine * /*engine*/)
{
   THROW_SQL(DATABASE_ERR, QDeclarativeEngine::tr("executeSql called outside transaction()"));
}

static QScriptValue qmlsqldatabase_executeSql(QScriptContext *context, QScriptEngine *engine)
{
   QSqlDatabase db = qscriptvalue_cast<QSqlDatabase>(context->thisObject());
   QString sql = context->argument(0).toString();
   QSqlQuery query(db);
   bool err = false;

   QScriptValue result;

   if (query.prepare(sql)) {
      if (context->argumentCount() > 1) {
         QScriptValue values = context->argument(1);
         if (values.isObject()) {
            if (values.isArray()) {
               int size = values.property(QLatin1String("length")).toInt32();
               for (int i = 0; i < size; ++i) {
                  query.bindValue(i, values.property(i).toVariant());
               }
            } else {
               for (QScriptValueIterator it(values); it.hasNext();) {
                  it.next();
                  query.bindValue(it.name(), it.value().toVariant());
               }
            }
         } else {
            query.bindValue(0, values.toVariant());
         }
      }
      if (query.exec()) {
         result = engine->newObject();
         QDeclarativeScriptEngine *qmlengine = static_cast<QDeclarativeScriptEngine *>(engine);
         if (!qmlengine->sqlQueryClass) {
            qmlengine->sqlQueryClass = new QDeclarativeSqlQueryScriptClass(engine);
         }
         QScriptValue rows = engine->newObject(qmlengine->sqlQueryClass);
         rows.setData(engine->newVariant(QVariant::fromValue(query)));
         rows.setProperty(QLatin1String("item"), engine->newFunction(qmlsqldatabase_item, 1), QScriptValue::SkipInEnumeration);
         result.setProperty(QLatin1String("rows"), rows);
         result.setProperty(QLatin1String("rowsAffected"), query.numRowsAffected());
         result.setProperty(QLatin1String("insertId"), query.lastInsertId().toString());
      } else {
         err = true;
      }
   } else {
      err = true;
   }
   if (err) {
      THROW_SQL(DATABASE_ERR, query.lastError().text());
   }
   return result;
}

static QScriptValue qmlsqldatabase_executeSql_readonly(QScriptContext *context, QScriptEngine *engine)
{
   QString sql = context->argument(0).toString();
   if (sql.startsWith(QLatin1String("SELECT"), Qt::CaseInsensitive)) {
      return qmlsqldatabase_executeSql(context, engine);
   } else {
      THROW_SQL(SYNTAX_ERR, QDeclarativeEngine::tr("Read-only Transaction"))
   }
}

static QScriptValue qmlsqldatabase_change_version(QScriptContext *context, QScriptEngine *engine)
{
   if (context->argumentCount() < 2) {
      return engine->undefinedValue();
   }

   QSqlDatabase db = qscriptvalue_cast<QSqlDatabase>(context->thisObject());
   QString from_version = context->argument(0).toString();
   QString to_version = context->argument(1).toString();
   QScriptValue callback = context->argument(2);

   QScriptValue instance = engine->newObject();
   instance.setProperty(QLatin1String("executeSql"), engine->newFunction(qmlsqldatabase_executeSql, 1));
   QScriptValue tx = engine->newVariant(instance, QVariant::fromValue(db));

   QString foundvers = context->thisObject().property(QLatin1String("version")).toString();
   if (from_version != foundvers) {
      THROW_SQL(VERSION_ERR, QDeclarativeEngine::tr("Version mismatch: expected %1, found %2").arg(from_version).arg(
                   foundvers));
      return engine->undefinedValue();
   }

   bool ok = true;
   if (callback.isFunction()) {
      ok = false;
      db.transaction();
      callback.call(QScriptValue(), QScriptValueList() << tx);
      if (engine->hasUncaughtException()) {
         db.rollback();
      } else {
         if (!db.commit()) {
            db.rollback();
            THROW_SQL(UNKNOWN_ERR, QDeclarativeEngine::tr("SQL transaction failed"));
         } else {
            ok = true;
         }
      }
   }

   if (ok) {
      context->thisObject().setProperty(QLatin1String("version"), to_version, QScriptValue::ReadOnly);
#ifndef QT_NO_SETTINGS
      QSettings ini(qmlsqldatabase_databaseFile(db.connectionName(), engine) + QLatin1String(".ini"), QSettings::IniFormat);
      ini.setValue(QLatin1String("Version"), to_version);
#endif
   }

   return engine->undefinedValue();
}

static QScriptValue qmlsqldatabase_transaction_shared(QScriptContext *context, QScriptEngine *engine, bool readOnly)
{
   QSqlDatabase db = qscriptvalue_cast<QSqlDatabase>(context->thisObject());
   QScriptValue callback = context->argument(0);
   if (!callback.isFunction()) {
      THROW_SQL(UNKNOWN_ERR, QDeclarativeEngine::tr("transaction: missing callback"));
   }

   QScriptValue instance = engine->newObject();
   instance.setProperty(QLatin1String("executeSql"),
                        engine->newFunction(readOnly ? qmlsqldatabase_executeSql_readonly : qmlsqldatabase_executeSql, 1));
   QScriptValue tx = engine->newVariant(instance, QVariant::fromValue(db));

   db.transaction();
   callback.call(QScriptValue(), QScriptValueList() << tx);
   instance.setProperty(QLatin1String("executeSql"),
                        engine->newFunction(qmlsqldatabase_executeSql_outsidetransaction));
   if (engine->hasUncaughtException()) {
      db.rollback();
   } else {
      if (!db.commit()) {
         db.rollback();
      }
   }
   return engine->undefinedValue();
}

static QScriptValue qmlsqldatabase_transaction(QScriptContext *context, QScriptEngine *engine)
{
   return qmlsqldatabase_transaction_shared(context, engine, false);
}

static QScriptValue qmlsqldatabase_read_transaction(QScriptContext *context, QScriptEngine *engine)
{
   return qmlsqldatabase_transaction_shared(context, engine, true);
}

static QScriptValue qmlsqldatabase_open_sync(QScriptContext *context, QScriptEngine *engine)
{
#ifndef QT_NO_SETTINGS
   qmlsqldatabase_initDatabasesPath(engine);

   QSqlDatabase database;

   QString dbname = context->argument(0).toString();
   QString dbversion = context->argument(1).toString();
   QString dbdescription = context->argument(2).toString();
   int dbestimatedsize = context->argument(3).toNumber();
   QScriptValue dbcreationCallback = context->argument(4);

   QCryptographicHash md5(QCryptographicHash::Md5);
   md5.addData(dbname.toUtf8());
   QString dbid(QLatin1String(md5.result().toHex()));

   QString basename = qmlsqldatabase_databaseFile(dbid, engine);
   bool created = false;
   QString version = dbversion;

   {
      QSettings ini(basename + QLatin1String(".ini"), QSettings::IniFormat);

      if (QSqlDatabase::connectionNames().contains(dbid)) {
         database = QSqlDatabase::database(dbid);
         version = ini.value(QLatin1String("Version")).toString();
         if (version != dbversion && !dbversion.isEmpty() && !version.isEmpty()) {
            THROW_SQL(VERSION_ERR, QDeclarativeEngine::tr("SQL: database version mismatch"));
         }
      } else {
         created = !QFile::exists(basename + QLatin1String(".sqlite"));
         database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), dbid);
         if (created) {
            ini.setValue(QLatin1String("Name"), dbname);
            if (dbcreationCallback.isFunction()) {
               version = QString();
            }
            ini.setValue(QLatin1String("Version"), version);
            ini.setValue(QLatin1String("Description"), dbdescription);
            ini.setValue(QLatin1String("EstimatedSize"), dbestimatedsize);
            ini.setValue(QLatin1String("Driver"), QLatin1String("QSQLITE"));
         } else {
            if (!dbversion.isEmpty() && ini.value(QLatin1String("Version")) != dbversion) {
               // Incompatible
               THROW_SQL(VERSION_ERR, QDeclarativeEngine::tr("SQL: database version mismatch"));
            }
            version = ini.value(QLatin1String("Version")).toString();
         }
         database.setDatabaseName(basename + QLatin1String(".sqlite"));
      }
      if (!database.isOpen()) {
         database.open();
      }
   }

   QScriptValue instance = engine->newObject();
   instance.setProperty(QLatin1String("transaction"), engine->newFunction(qmlsqldatabase_transaction, 1));
   instance.setProperty(QLatin1String("readTransaction"), engine->newFunction(qmlsqldatabase_read_transaction, 1));
   instance.setProperty(QLatin1String("version"), version, QScriptValue::ReadOnly);
   instance.setProperty(QLatin1String("changeVersion"), engine->newFunction(qmlsqldatabase_change_version, 3));

   QScriptValue result = engine->newVariant(instance, QVariant::fromValue(database));

   if (created && dbcreationCallback.isFunction()) {
      dbcreationCallback.call(QScriptValue(), QScriptValueList() << result);
   }

   return result;
#else
   return engine->undefinedValue();
#endif // QT_NO_SETTINGS
}

void qt_add_qmlsqldatabase(QScriptEngine *engine)
{
   QScriptValue openDatabase = engine->newFunction(qmlsqldatabase_open_sync, 4);
   engine->globalObject().setProperty(QLatin1String("openDatabaseSync"), openDatabase);

   QScriptValue sqlExceptionPrototype = engine->newObject();
   for (int i = 0; sqlerror[i]; ++i)
      sqlExceptionPrototype.setProperty(QLatin1String(sqlerror[i]),
                                        i, QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);

   engine->globalObject().setProperty(QLatin1String("SQLException"), sqlExceptionPrototype);
}

/*
HTML5 "spec" says "rs.rows[n]", but WebKit only impelments "rs.rows.item(n)". We do both (and property iterator).
We add a "forwardOnly" property that stops Qt caching results (code promises to only go forward
through the data.
*/

QT_END_NAMESPACE
