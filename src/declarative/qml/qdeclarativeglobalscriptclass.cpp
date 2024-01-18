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

#include "private/qdeclarativeglobalscriptclass_p.h"

#include <QtCore/qstringlist.h>
#include <QtCore/qvector.h>
#include <QtScript/qscriptstring.h>
#include <QtScript/qscriptengine.h>
#include <QtScript/qscriptvalueiterator.h>

#include <private/qscriptdeclarativeclass_p.h>

QT_BEGIN_NAMESPACE

/*
    Used to prevent any writes to the global object.
*/
QDeclarativeGlobalScriptClass::QDeclarativeGlobalScriptClass(QScriptEngine *engine)
   : QScriptClass(engine)
{
   QString eval = QLatin1String("eval");
   QString version = QLatin1String("version");

   QScriptValue originalGlobalObject = engine->globalObject();

   QScriptValue newGlobalObject = engine->newObject();

   {
      QScriptValueIterator iter(originalGlobalObject);
      QVector<QString> names;
      QVector<QScriptValue> values;
      QVector<QScriptValue::PropertyFlags> flags;
      while (iter.hasNext()) {
         iter.next();

         QString name = iter.name();

         if (name == version) {
            continue;
         }

         if (name != eval) {
            names.append(name);
            values.append(iter.value());
            flags.append(iter.flags() | QScriptValue::Undeletable);
         }
         newGlobalObject.setProperty(iter.scriptName(), iter.value());

         m_illegalNames.insert(name);
      }
      m_staticGlobalObject = QScriptDeclarativeClass::newStaticScopeObject(
                                engine, names.size(), names.constData(), values.constData(), flags.constData());
   }

   newGlobalObject.setScriptClass(this);
   engine->setGlobalObject(newGlobalObject);
}

QScriptClass::QueryFlags
QDeclarativeGlobalScriptClass::queryProperty(const QScriptValue &object,
      const QScriptString &name,
      QueryFlags flags, uint *id)
{
   Q_UNUSED(object);
   Q_UNUSED(name);
   Q_UNUSED(flags);
   Q_UNUSED(id);
   return HandlesWriteAccess;
}

void QDeclarativeGlobalScriptClass::setProperty(QScriptValue &object,
      const QScriptString &name,
      uint id, const QScriptValue &value)
{
   Q_UNUSED(object);
   Q_UNUSED(id);
   Q_UNUSED(value);
   QString error = QLatin1String("Invalid write to global property \"") +
                   name.toString() + QLatin1Char('\"');
   engine()->currentContext()->throwError(error);
}

/* This method is for the use of tst_qdeclarativeecmascript::callQtInvokables() only */
void QDeclarativeGlobalScriptClass::explicitSetProperty(const QStringList &names, const QList<QScriptValue> &values)
{
   Q_ASSERT(names.count() == values.count());
   QScriptValue globalObject = engine()->globalObject();

   QScriptValue v = engine()->newObject();

   QScriptValueIterator iter(v);
   while (iter.hasNext()) {
      iter.next();
      v.setProperty(iter.scriptName(), iter.value());
   }

   for (int ii = 0; ii < names.count(); ++ii) {
      const QString &name = names.at(ii);
      const QScriptValue &value = values.at(ii);
      v.setProperty(name, value);
   }

   v.setScriptClass(this);

   engine()->setGlobalObject(v);
}

QT_END_NAMESPACE

