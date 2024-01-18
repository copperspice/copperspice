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

#include <qdeclarativetypenamescriptclass_p.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativetypenamecache_p.h>

QT_BEGIN_NAMESPACE

struct TypeNameData : public QScriptDeclarativeClass::Object {
   TypeNameData(QObject *o, QDeclarativeType *t, QDeclarativeTypeNameScriptClass::TypeNameMode m)
      : object(o), type(t), typeNamespace(0), mode(m) {}

   TypeNameData(QObject *o, QDeclarativeTypeNameCache *n, QDeclarativeTypeNameScriptClass::TypeNameMode m)
      : object(o), type(0), typeNamespace(n), mode(m) {
      if (typeNamespace) {
         typeNamespace->addref();
      }
   }

   ~TypeNameData() {
      if (typeNamespace) {
         typeNamespace->release();
      }
   }

   QObject *object;
   QDeclarativeType *type;
   QDeclarativeTypeNameCache *typeNamespace;
   QDeclarativeTypeNameScriptClass::TypeNameMode mode;
};

QDeclarativeTypeNameScriptClass::QDeclarativeTypeNameScriptClass(QDeclarativeEngine *bindEngine)
   : QScriptDeclarativeClass(QDeclarativeEnginePrivate::getScriptEngine(bindEngine)),
     engine(bindEngine), object(0), type(0)
{
}

QDeclarativeTypeNameScriptClass::~QDeclarativeTypeNameScriptClass()
{
}

QScriptValue QDeclarativeTypeNameScriptClass::newObject(QObject *object, QDeclarativeType *type, TypeNameMode mode)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   return QScriptDeclarativeClass::newObject(scriptEngine, this, new TypeNameData(object, type, mode));
}

QScriptValue QDeclarativeTypeNameScriptClass::newObject(QObject *object, QDeclarativeTypeNameCache *ns,
      TypeNameMode mode)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   return QScriptDeclarativeClass::newObject(scriptEngine, this, new TypeNameData(object, ns, mode));
}

QScriptClass::QueryFlags
QDeclarativeTypeNameScriptClass::queryProperty(Object *obj, const Identifier &name,
      QScriptClass::QueryFlags flags)
{
   Q_UNUSED(flags);

   TypeNameData *data = (TypeNameData *)obj;

   object = 0;
   type = 0;
   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);

   if (data->typeNamespace) {

      QDeclarativeTypeNameCache::Data *d = data->typeNamespace->data(name);
      if (d && d->type) {
         type = d->type;
         return QScriptClass::HandlesReadAccess;
      } else {
         return 0;
      }

   } else if (data->type) {

      if (startsWithUpper(name)) {
         QString strName = toString(name);
         // Must be an enum
         if (data->mode == IncludeEnums) {
            // ### Optimize
            QByteArray enumName = strName.toUtf8();
            const QMetaObject *metaObject = data->type->baseMetaObject();
            for (int ii = metaObject->enumeratorCount() - 1; ii >= 0; --ii) {
               QMetaEnum e = metaObject->enumerator(ii);
               int value = e.keyToValue(enumName.constData());
               if (value != -1) {
                  enumValue = value;
                  return QScriptClass::HandlesReadAccess;
               }
            }
         }
         return 0;
      } else if (data->object) {
         // Must be an attached property
         object = qmlAttachedPropertiesObjectById(data->type->attachedPropertiesId(), data->object);
         if (!object) {
            return 0;
         }
         return ep->objectClass->queryProperty(object, name, flags, 0);
      }

   }

   return 0;
}

QDeclarativeTypeNameScriptClass::Value
QDeclarativeTypeNameScriptClass::property(Object *obj, const Identifier &name)
{
   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);
   if (type) {
      return Value(scriptEngine, newObject(((TypeNameData *)obj)->object, type, ((TypeNameData *)obj)->mode));
   } else if (object) {
      return ep->objectClass->property(object, name);
   } else {
      return Value(scriptEngine, enumValue);
   }
}

void QDeclarativeTypeNameScriptClass::setProperty(Object *, const Identifier &n, const QScriptValue &v)
{
   Q_ASSERT(object);
   Q_ASSERT(!type);

   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);
   ep->objectClass->setProperty(object, n, v, context());
}

QT_END_NAMESPACE

