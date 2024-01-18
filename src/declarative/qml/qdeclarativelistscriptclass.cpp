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

#include "private/qdeclarativelistscriptclass_p.h"

#include "private/qdeclarativeengine_p.h"
#include "private/qdeclarativeguard_p.h"
#include "private/qdeclarativelist_p.h"

QT_BEGIN_NAMESPACE

struct ListData : public QScriptDeclarativeClass::Object {
   QDeclarativeGuard<QObject> object;
   QDeclarativeListProperty<QObject> property;
   int propertyType;
};

QDeclarativeListScriptClass::QDeclarativeListScriptClass(QDeclarativeEngine *e)
   : QScriptDeclarativeClass(QDeclarativeEnginePrivate::getScriptEngine(e)), engine(e)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);
   Q_UNUSED(scriptEngine);

   m_lengthId = createPersistentIdentifier(QLatin1String("length"));
}

QDeclarativeListScriptClass::~QDeclarativeListScriptClass()
{
}

QScriptValue QDeclarativeListScriptClass::newList(QObject *object, int propId, int propType)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   if (!object || propId == -1) {
      return scriptEngine->nullValue();
   }

   ListData *data = new ListData;
   data->object = object;
   data->propertyType = propType;
   void *args[] = { &data->property, 0 };
   QMetaObject::metacall(object, QMetaObject::ReadProperty, propId, args);

   return newObject(scriptEngine, this, data);
}

QScriptValue QDeclarativeListScriptClass::newList(const QDeclarativeListProperty<QObject> &prop, int propType)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   ListData *data = new ListData;
   data->object = prop.object;
   data->property = prop;
   data->propertyType = propType;

   return newObject(scriptEngine, this, data);
}

QScriptClass::QueryFlags
QDeclarativeListScriptClass::queryProperty(Object *object, const Identifier &name,
      QScriptClass::QueryFlags flags)
{
   Q_UNUSED(object);
   Q_UNUSED(flags);
   if (name == m_lengthId.identifier) {
      return QScriptClass::HandlesReadAccess;
   }

   bool ok = false;
   quint32 idx = toArrayIndex(name, &ok);

   if (ok) {
      lastIndex = idx;
      return QScriptClass::HandlesReadAccess;
   } else {
      return 0;
   }
}

QDeclarativeListScriptClass::Value QDeclarativeListScriptClass::property(Object *obj, const Identifier &name)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);
   QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(engine);

   ListData *data = (ListData *)obj;
   if (!data->object) {
      return Value();
   }

   quint32 count = data->property.count ? data->property.count(&data->property) : 0;

   if (name == m_lengthId.identifier) {
      return Value(scriptEngine, count);
   } else if (lastIndex < count && data->property.at) {
      return Value(scriptEngine, enginePriv->objectClass->newQObject(data->property.at(&data->property, lastIndex)));
   } else {
      return Value();
   }
}

QVariant QDeclarativeListScriptClass::toVariant(Object *obj, bool *ok)
{
   ListData *data = (ListData *)obj;

   if (!data->object) {
      if (ok) {
         *ok = false;
      }
      return QVariant();
   }

   return QVariant::fromValue(QDeclarativeListReferencePrivate::init(data->property, data->propertyType, engine));
}

QT_END_NAMESPACE

