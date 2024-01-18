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

#include "private/qdeclarativeobjectscriptclass_p.h"

#include "private/qdeclarativeengine_p.h"
#include "private/qdeclarativecontext_p.h"
#include "private/qdeclarativedata_p.h"
#include "private/qdeclarativetypenamescriptclass_p.h"
#include "private/qdeclarativelistscriptclass_p.h"
#include "private/qdeclarativebinding_p.h"
#include "private/qdeclarativeguard_p.h"
#include "private/qdeclarativevmemetaobject_p.h"

#include <QtCore/qtimer.h>
#include <QtCore/qvarlengtharray.h>
#include <QtScript/qscriptcontextinfo.h>

Q_DECLARE_METATYPE(QScriptValue)

#if defined(__GNUC__)
# if (__GNUC__ * 100 + __GNUC_MINOR__) >= 405
// The code in this file does not violate strict aliasing, but GCC thinks it does
// so turn off the warnings for us to have a clean build
#  pragma GCC diagnostic ignored "-Wstrict-aliasing"
# endif
#endif

QT_BEGIN_NAMESPACE

struct ObjectData : public QScriptDeclarativeClass::Object {
   ObjectData(QObject *o, int t) : object(o), type(t) {
      if (o) {
         QDeclarativeData *ddata = QDeclarativeData::get(object, true);
         if (ddata) {
            ddata->objectDataRefCount++;
         }
      }
   }

   virtual ~ObjectData() {
      if (object && !object->parent()) {
         QDeclarativeData *ddata = QDeclarativeData::get(object, false);
         if (ddata && !ddata->indestructible && 0 == --ddata->objectDataRefCount) {
            object->deleteLater();
         }
      }
   }

   QDeclarativeGuard<QObject> object;
   int type;
};

/*
    The QDeclarativeObjectScriptClass handles property access for QObjects
    via QtScript. It is also used to provide a more useful API in
    QtScript for QML.
 */
QDeclarativeObjectScriptClass::QDeclarativeObjectScriptClass(QDeclarativeEngine *bindEngine)
   : QScriptDeclarativeClass(QDeclarativeEnginePrivate::getScriptEngine(bindEngine)),
     methods(bindEngine), lastData(0), engine(bindEngine)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   m_destroy = scriptEngine->newFunction(destroy);
   m_destroyId = createPersistentIdentifier(QLatin1String("destroy"));
   m_toString = scriptEngine->newFunction(tostring);
   m_toStringId = createPersistentIdentifier(QLatin1String("toString"));
}

QDeclarativeObjectScriptClass::~QDeclarativeObjectScriptClass()
{
}

QScriptValue QDeclarativeObjectScriptClass::newQObject(QObject *object, int type)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   if (!object) {
      return scriptEngine->nullValue();
   }
   //        return newObject(scriptEngine, this, new ObjectData(object, type));

   if (QObjectPrivate::get(object)->wasDeleted) {
      return scriptEngine->undefinedValue();
   }

   QDeclarativeData *ddata = QDeclarativeData::get(object, true);

   if (!ddata) {
      return scriptEngine->undefinedValue();
   } else if (!ddata->indestructible && !object->parent()) {
      return newObject(scriptEngine, this, new ObjectData(object, type));
   } else if (!ddata->scriptValue) {
      ddata->scriptValue = new QScriptValue(newObject(scriptEngine, this, new ObjectData(object, type)));
      return *ddata->scriptValue;
   } else if (ddata->scriptValue->engine() == QDeclarativeEnginePrivate::getScriptEngine(engine)) {
      return *ddata->scriptValue;
   } else {
      return newObject(scriptEngine, this, new ObjectData(object, type));
   }
}

QObject *QDeclarativeObjectScriptClass::toQObject(const QScriptValue &value) const
{
   return value.toQObject();
}

int QDeclarativeObjectScriptClass::objectType(const QScriptValue &value) const
{
   if (scriptClass(value) != this) {
      return QVariant::Invalid;
   }

   Object *o = object(value);
   return ((ObjectData *)(o))->type;
}

QScriptClass::QueryFlags
QDeclarativeObjectScriptClass::queryProperty(Object *object, const Identifier &name,
      QScriptClass::QueryFlags flags)
{
   return queryProperty(toQObject(object), name, flags, 0);
}

QScriptClass::QueryFlags
QDeclarativeObjectScriptClass::queryProperty(QObject *obj, const Identifier &name,
      QScriptClass::QueryFlags flags, QDeclarativeContextData *evalContext,
      QueryHints hints)
{
   Q_UNUSED(flags);
   lastData = 0;
   lastTNData = 0;

   if (name == m_destroyId.identifier ||
         name == m_toStringId.identifier) {
      return QScriptClass::HandlesReadAccess;
   }

   if (!obj) {
      return 0;
   }

   QDeclarativeEnginePrivate *enginePrivate = QDeclarativeEnginePrivate::get(engine);
   lastData = QDeclarativePropertyCache::property(engine, obj, name, local);
   if ((hints & ImplicitObject) && lastData && lastData->revision != 0) {

      QDeclarativeData *ddata = QDeclarativeData::get(obj);
      if (ddata && ddata->propertyCache && !ddata->propertyCache->isAllowedInRevision(lastData)) {
         return 0;
      }
   }

   if (lastData) {
      return QScriptClass::HandlesReadAccess | QScriptClass::HandlesWriteAccess;
   }

   if (!(hints & SkipAttachedProperties)) {
      if (!evalContext && context()) {
         // Global object, QScriptContext activation object, QDeclarativeContext object
         QScriptValue scopeNode = scopeChainValue(context(), -3);
         if (scopeNode.isValid()) {
            Q_ASSERT(scriptClass(scopeNode) == enginePrivate->contextClass);

            evalContext = enginePrivate->contextClass->contextFromValue(scopeNode);
         }
      }

      if (evalContext && evalContext->imports) {
         QDeclarativeTypeNameCache::Data *data = evalContext->imports->data(name);
         if (data) {
            lastTNData = data;
            return QScriptClass::HandlesReadAccess;
         }
      }
   }

   if (!(hints & ImplicitObject)) {
      local.coreIndex = -1;
      lastData = &local;
      return QScriptClass::HandlesWriteAccess;
   }

   return 0;
}

QDeclarativeObjectScriptClass::Value
QDeclarativeObjectScriptClass::property(Object *object, const Identifier &name)
{
   return property(toQObject(object), name);
}

QDeclarativeObjectScriptClass::Value
QDeclarativeObjectScriptClass::property(QObject *obj, const Identifier &name)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   if (name == m_destroyId.identifier) {
      return Value(scriptEngine, m_destroy);
   } else if (name == m_toStringId.identifier) {
      return Value(scriptEngine, m_toString);
   }

   if (lastData && !lastData->isValid()) {
      return Value();
   }

   Q_ASSERT(obj);

   QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(engine);

   if (lastTNData) {

      if (lastTNData->type) {
         return Value(scriptEngine, enginePriv->typeNameClass->newObject(obj, lastTNData->type));
      } else {
         return Value(scriptEngine, enginePriv->typeNameClass->newObject(obj, lastTNData->typeNamespace));
      }

   } else if (lastData->flags & QDeclarativePropertyCache::Data::IsFunction) {
      if (lastData->flags & QDeclarativePropertyCache::Data::IsVMEFunction) {
         return Value(scriptEngine, ((QDeclarativeVMEMetaObject *)(obj->metaObject()))->vmeMethod(lastData->coreIndex));
      } else {
         // Uncomment to use QtScript method call logic
         // QScriptValue sobj = scriptEngine->newQObject(obj);
         // return Value(scriptEngine, sobj.property(toString(name)));
         return Value(scriptEngine, methods.newMethod(obj, lastData));
      }
   } else {
      if (enginePriv->captureProperties && !(lastData->flags & QDeclarativePropertyCache::Data::IsConstant)) {
         if (lastData->coreIndex == 0) {
            enginePriv->capturedProperties <<
                                           QDeclarativeEnginePrivate::CapturedProperty(QDeclarativeData::get(obj, true)->objectNameNotifier());
         } else {
            enginePriv->capturedProperties <<
                                           QDeclarativeEnginePrivate::CapturedProperty(obj, lastData->coreIndex, lastData->notifyIndex);
         }
      }

      if (QDeclarativeValueTypeFactory::isValueType((uint)lastData->propType)) {
         QDeclarativeValueType *valueType = enginePriv->valueTypes[lastData->propType];
         if (valueType) {
            return Value(scriptEngine, enginePriv->valueTypeClass->newObject(obj, lastData->coreIndex, valueType));
         }
      }

      if (lastData->flags & QDeclarativePropertyCache::Data::IsQList) {
         return Value(scriptEngine, enginePriv->listClass->newList(obj, lastData->coreIndex, lastData->propType));
      } else if (lastData->flags & QDeclarativePropertyCache::Data::IsQObjectDerived) {
         QObject *rv = 0;
         void *args[] = { &rv, 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
         return Value(scriptEngine, newQObject(rv, lastData->propType));
      } else if (lastData->flags & QDeclarativePropertyCache::Data::IsQScriptValue) {
         QScriptValue rv = scriptEngine->nullValue();
         void *args[] = { &rv, 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
         return Value(scriptEngine, rv);
      } else if (lastData->propType == QMetaType::QReal) {
         qreal rv = 0;
         void *args[] = { &rv, 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
         return Value(scriptEngine, rv);
      } else if (lastData->propType == QMetaType::Int || lastData->flags & QDeclarativePropertyCache::Data::IsEnumType) {
         int rv = 0;
         void *args[] = { &rv, 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
         return Value(scriptEngine, rv);
      } else if (lastData->propType == QMetaType::Bool) {
         bool rv = false;
         void *args[] = { &rv, 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
         return Value(scriptEngine, rv);
      } else if (lastData->propType == QMetaType::QString) {
         QString rv;
         void *args[] = { &rv, 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
         return Value(scriptEngine, rv);
      } else if (lastData->propType == QMetaType::UInt) {
         uint rv = 0;
         void *args[] = { &rv, 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
         return Value(scriptEngine, rv);
      } else if (lastData->propType == QMetaType::Float) {
         float rv = 0;
         void *args[] = { &rv, 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
         return Value(scriptEngine, rv);
      } else if (lastData->propType == QMetaType::Double) {
         double rv = 0;
         void *args[] = { &rv, 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, lastData->coreIndex, args);
         return Value(scriptEngine, rv);
      } else {
         QVariant var = obj->metaObject()->property(lastData->coreIndex).read(obj);
         return Value(scriptEngine, enginePriv->scriptValueFromVariant(var));
      }
   }
}

void QDeclarativeObjectScriptClass::setProperty(Object *object,
      const Identifier &name,
      const QScriptValue &value)
{
   return setProperty(toQObject(object), name, value, context());
}
namespace {
int qRoundDouble(double d)
{
   return d >= double(0.0) ? int(d + double(0.5)) : int(d - int(d - 1) + double(0.5)) + int(d - 1);
}
}
void QDeclarativeObjectScriptClass::setProperty(QObject *obj,
      const Identifier &name,
      const QScriptValue &value,
      QScriptContext *context,
      QDeclarativeContextData *evalContext)
{
   Q_UNUSED(name);

   Q_ASSERT(obj);
   Q_ASSERT(lastData);
   Q_ASSERT(context);

   if (!lastData->isValid()) {
      QString error = QLatin1String("Cannot assign to non-existent property \"") +
                      toString(name) + QLatin1Char('\"');
      context->throwError(error);
      return;
   }

   if (!(lastData->flags & QDeclarativePropertyCache::Data::IsWritable) &&
         !(lastData->flags & QDeclarativePropertyCache::Data::IsQList)) {
      QString error = QLatin1String("Cannot assign to read-only property \"") +
                      toString(name) + QLatin1Char('\"');
      context->throwError(error);
      return;
   }

   QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(engine);

   if (!evalContext) {
      // Global object, QScriptContext activation object, QDeclarativeContext object
      QScriptValue scopeNode = scopeChainValue(context, -3);
      if (scopeNode.isValid()) {
         Q_ASSERT(scriptClass(scopeNode) == enginePriv->contextClass);

         evalContext = enginePriv->contextClass->contextFromValue(scopeNode);
      }
   }

   QDeclarativeBinding *newBinding = 0;
   if (value.isFunction() && !value.isRegExp()) {
      QScriptContextInfo ctxtInfo(context);
      QDeclarativePropertyCache::ValueTypeData valueTypeData;

      newBinding = new QDeclarativeBinding(value, obj, evalContext);
      newBinding->setSourceLocation(ctxtInfo.fileName(), ctxtInfo.functionStartLineNumber());
      newBinding->setTarget(QDeclarativePropertyPrivate::restore(*lastData, valueTypeData, obj, evalContext));
      if (newBinding->expression().contains(QLatin1String("this"))) {
         newBinding->setEvaluateFlags(newBinding->evaluateFlags() | QDeclarativeBinding::RequiresThisObject);
      }
   }

   QDeclarativeAbstractBinding *delBinding =
      QDeclarativePropertyPrivate::setBinding(obj, lastData->coreIndex, -1, newBinding);
   if (delBinding) {
      delBinding->destroy();
   }

   if (value.isNull() && lastData->flags & QDeclarativePropertyCache::Data::IsQObjectDerived) {
      QObject *o = 0;
      int status = -1;
      int flags = 0;
      void *argv[] = { &o, 0, &status, &flags };
      QMetaObject::metacall(obj, QMetaObject::WriteProperty, lastData->coreIndex, argv);
   } else if (value.isUndefined() && lastData->flags & QDeclarativePropertyCache::Data::IsResettable) {
      void *a[] = { 0 };
      QMetaObject::metacall(obj, QMetaObject::ResetProperty, lastData->coreIndex, a);
   } else if (value.isUndefined() && lastData->propType == qMetaTypeId<QVariant>()) {
      QDeclarativePropertyPrivate::write(obj, *lastData, QVariant(), evalContext);
   } else if (value.isUndefined()) {
      QString error = QLatin1String("Cannot assign [undefined] to ") +
                      QLatin1String(QMetaType::typeName(lastData->propType));
      context->throwError(error);
   } else if (value.isFunction() && !value.isRegExp()) {
      // this is handled by the binding creation above
   } else {
      //### expand optimization for other known types
      if (lastData->propType == QMetaType::Int && value.isNumber()) {
         int rawValue = qRoundDouble(value.toNumber());
         int status = -1;
         int flags = 0;
         void *a[] = { (void *) &rawValue, 0, &status, &flags };
         QMetaObject::metacall(obj, QMetaObject::WriteProperty,
                               lastData->coreIndex, a);
         return;
      } else if (lastData->propType == QMetaType::QReal && value.isNumber()) {
         qreal rawValue = qreal(value.toNumber());
         int status = -1;
         int flags = 0;
         void *a[] = { (void *) &rawValue, 0, &status, &flags };
         QMetaObject::metacall(obj, QMetaObject::WriteProperty,
                               lastData->coreIndex, a);
         return;
      } else if (lastData->propType == QMetaType::QString && value.isString()) {
         const QString &rawValue = value.toString();
         int status = -1;
         int flags = 0;
         void *a[] = { (void *) &rawValue, 0, &status, &flags };
         QMetaObject::metacall(obj, QMetaObject::WriteProperty,
                               lastData->coreIndex, a);
         return;
      }

      QVariant v;
      if (lastData->flags & QDeclarativePropertyCache::Data::IsQList) {
         v = enginePriv->scriptValueToVariant(value, qMetaTypeId<QList<QObject *> >());
      } else {
         v = enginePriv->scriptValueToVariant(value, lastData->propType);
      }

      if (!QDeclarativePropertyPrivate::write(obj, *lastData, v, evalContext)) {
         const char *valueType = 0;
         if (v.userType() == QVariant::Invalid) {
            valueType = "null";
         } else {
            valueType = QMetaType::typeName(v.userType());
         }

         QString error = QLatin1String("Cannot assign ") +
                         QLatin1String(valueType) +
                         QLatin1String(" to ") +
                         QLatin1String(QMetaType::typeName(lastData->propType));
         context->throwError(error);
      }
   }
}

bool QDeclarativeObjectScriptClass::isQObject() const
{
   return true;
}

QObject *QDeclarativeObjectScriptClass::toQObject(Object *object, bool *ok)
{
   if (ok) {
      *ok = true;
   }

   ObjectData *data = (ObjectData *)object;
   return data->object.data();
}

QScriptValue QDeclarativeObjectScriptClass::tostring(QScriptContext *context, QScriptEngine *)
{
   QObject *obj = context->thisObject().toQObject();

   QString ret;
   if (obj) {
      QString objectName = obj->objectName();

      ret += QString::fromUtf8(obj->metaObject()->className());
      ret += QLatin1String("(0x");
      ret += QString::number((quintptr)obj, 16);

      if (!objectName.isEmpty()) {
         ret += QLatin1String(", \"");
         ret += objectName;
         ret += QLatin1Char('\"');
      }

      ret += QLatin1Char(')');
   } else {
      ret += QLatin1String("null");
   }
   return QScriptValue(ret);
}

QScriptValue QDeclarativeObjectScriptClass::destroy(QScriptContext *context, QScriptEngine *engine)
{
   QDeclarativeEnginePrivate *p = QDeclarativeEnginePrivate::get(engine);
   QScriptValue that = context->thisObject();

   if (scriptClass(that) != p->objectClass) {
      return engine->undefinedValue();
   }

   ObjectData *data = (ObjectData *)p->objectClass->object(that);
   if (!data->object) {
      return engine->undefinedValue();
   }

   QDeclarativeData *ddata = QDeclarativeData::get(data->object, false);
   if (!ddata || ddata->indestructible) {
      return engine->currentContext()->throwError(QLatin1String("Invalid attempt to destroy() an indestructible object"));
   }

   QObject *obj = data->object;
   int delay = 0;
   if (context->argumentCount() > 0) {
      delay = context->argument(0).toInt32();
   }
   if (delay > 0) {
      QTimer::singleShot(delay, obj, SLOT(deleteLater()));
   } else {
      obj->deleteLater();
   }

   return engine->undefinedValue();
}

QStringList QDeclarativeObjectScriptClass::propertyNames(Object *object)
{
   QObject *obj = toQObject(object);
   if (!obj) {
      return QStringList();
   }

   QDeclarativeEnginePrivate *enginePrivate = QDeclarativeEnginePrivate::get(engine);

   QDeclarativePropertyCache *cache = 0;
   QDeclarativeData *ddata = QDeclarativeData::get(obj);
   if (ddata) {
      cache = ddata->propertyCache;
   }
   if (!cache) {
      cache = enginePrivate->cache(obj);
      if (cache) {
         if (ddata) {
            cache->addref();
            ddata->propertyCache = cache;
         }
      } else {
         // Not cachable - fall back to QMetaObject (eg. dynamic meta object)
         // XXX QDeclarativeOpenMetaObject has a cache, so this is suboptimal.
         // XXX This is a workaround for QTBUG-9420.
         const QMetaObject *mo = obj->metaObject();
         QStringList r;
         int pc = mo->propertyCount();
         int po = mo->propertyOffset();
         for (int i = po; i < pc; ++i) {
            r += QString::fromUtf8(mo->property(i).name());
         }
         return r;
      }
   }
   return cache->propertyNames();
}

bool QDeclarativeObjectScriptClass::compare(Object *o1, Object *o2)
{
   ObjectData *d1 = (ObjectData *)o1;
   ObjectData *d2 = (ObjectData *)o2;

   return d1 == d2 || d1->object == d2->object;
}

struct MethodData : public QScriptDeclarativeClass::Object {
   MethodData(QObject *o, const QDeclarativePropertyCache::Data &d) : object(o), data(d) {}

   QDeclarativeGuard<QObject> object;
   QDeclarativePropertyCache::Data data;
};

QDeclarativeObjectMethodScriptClass::QDeclarativeObjectMethodScriptClass(QDeclarativeEngine *bindEngine)
   : QScriptDeclarativeClass(QDeclarativeEnginePrivate::getScriptEngine(bindEngine)),
     engine(bindEngine)
{
   qRegisterMetaType<QList<QObject *> >("QList<QObject *>");

   setSupportsCall(true);

   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   m_connect = scriptEngine->newFunction(connect);
   m_connectId = createPersistentIdentifier(QLatin1String("connect"));
   m_disconnect = scriptEngine->newFunction(disconnect);
   m_disconnectId = createPersistentIdentifier(QLatin1String("disconnect"));
}

QDeclarativeObjectMethodScriptClass::~QDeclarativeObjectMethodScriptClass()
{
}

QScriptValue QDeclarativeObjectMethodScriptClass::newMethod(QObject *object,
      const QDeclarativePropertyCache::Data *method)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   return newObject(scriptEngine, this, new MethodData(object, *method));
}

QScriptValue QDeclarativeObjectMethodScriptClass::connect(QScriptContext *context, QScriptEngine *engine)
{
   QDeclarativeEnginePrivate *p = QDeclarativeEnginePrivate::get(engine);

   QScriptValue that = context->thisObject();
   if (&p->objectClass->methods != scriptClass(that)) {
      return engine->undefinedValue();
   }

   MethodData *data = (MethodData *)object(that);

   if (!data->object || context->argumentCount() == 0) {
      return engine->undefinedValue();
   }

   QByteArray signal("2");
   signal.append(data->object->metaObject()->method(data->data.coreIndex).signature());

   if (context->argumentCount() == 1) {
      qScriptConnect(data->object, signal.constData(), QScriptValue(), context->argument(0));
   } else {
      qScriptConnect(data->object, signal.constData(), context->argument(0), context->argument(1));
   }

   return engine->undefinedValue();
}

QScriptValue QDeclarativeObjectMethodScriptClass::disconnect(QScriptContext *context, QScriptEngine *engine)
{
   QDeclarativeEnginePrivate *p = QDeclarativeEnginePrivate::get(engine);

   QScriptValue that = context->thisObject();
   if (&p->objectClass->methods != scriptClass(that)) {
      return engine->undefinedValue();
   }

   MethodData *data = (MethodData *)object(that);

   if (!data->object || context->argumentCount() == 0) {
      return engine->undefinedValue();
   }

   QByteArray signal("2");
   signal.append(data->object->metaObject()->method(data->data.coreIndex).signature());

   if (context->argumentCount() == 1) {
      qScriptDisconnect(data->object, signal.constData(), QScriptValue(), context->argument(0));
   } else {
      qScriptDisconnect(data->object, signal.constData(), context->argument(0), context->argument(1));
   }

   return engine->undefinedValue();
}

QScriptClass::QueryFlags
QDeclarativeObjectMethodScriptClass::queryProperty(Object *, const Identifier &name,
      QScriptClass::QueryFlags flags)
{
   Q_UNUSED(flags);
   if (name == m_connectId.identifier || name == m_disconnectId.identifier) {
      return QScriptClass::HandlesReadAccess;
   } else {
      return 0;
   }

}

QDeclarativeObjectMethodScriptClass::Value
QDeclarativeObjectMethodScriptClass::property(Object *, const Identifier &name)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   if (name == m_connectId.identifier) {
      return Value(scriptEngine, m_connect);
   } else if (name == m_disconnectId.identifier) {
      return Value(scriptEngine, m_disconnect);
   } else {
      return Value();
   }
}

namespace {
struct MetaCallArgument {
   inline MetaCallArgument();
   inline ~MetaCallArgument();
   inline void *dataPtr();

   inline void initAsType(int type, QDeclarativeEngine *);
   void fromScriptValue(int type, QDeclarativeEngine *, const QScriptValue &);
   inline QScriptDeclarativeClass::Value toValue(QDeclarativeEngine *);

 private:
   MetaCallArgument(const MetaCallArgument &);

   inline void cleanup();

   union {
      float floatValue;
      double doubleValue;
      quint32 intValue;
      bool boolValue;
      QObject *qobjectPtr;

      char allocData[sizeof(QVariant)];
   };

   // Pointers to allocData
   union {
      QString *qstringPtr;
      QVariant *qvariantPtr;
      QList<QObject *> *qlistPtr;
      QScriptValue *qscriptValuePtr;
   };

   int type;
};
}

MetaCallArgument::MetaCallArgument()
   : type(QVariant::Invalid)
{
}

MetaCallArgument::~MetaCallArgument()
{
   cleanup();
}

void MetaCallArgument::cleanup()
{
   if (type == QMetaType::QString) {
      qstringPtr->~QString();
   } else if (type == -1 || type == QMetaType::QVariant) {
      qvariantPtr->~QVariant();
   } else if (type == qMetaTypeId<QScriptValue>()) {
      qscriptValuePtr->~QScriptValue();
   } else if (type == qMetaTypeId<QList<QObject *> >()) {
      qlistPtr->~QList<QObject *>();
   }
}

void *MetaCallArgument::dataPtr()
{
   if (type == -1) {
      return qvariantPtr->data();
   } else {
      return (void *)&allocData;
   }
}

void MetaCallArgument::initAsType(int callType, QDeclarativeEngine *e)
{
   if (type != 0) {
      cleanup();
      type = 0;
   }
   if (callType == 0) {
      return;
   }

   QScriptEngine *engine = QDeclarativeEnginePrivate::getScriptEngine(e);

   if (callType == qMetaTypeId<QScriptValue>()) {
      qscriptValuePtr = new (&allocData) QScriptValue(engine->undefinedValue());
      type = callType;
   } else if (callType == QMetaType::Int ||
              callType == QMetaType::UInt ||
              callType == QMetaType::Bool ||
              callType == QMetaType::Double ||
              callType == QMetaType::Float) {
      type = callType;
   } else if (callType == QMetaType::QObjectStar) {
      qobjectPtr = 0;
      type = callType;
   } else if (callType == QMetaType::QString) {
      qstringPtr = new (&allocData) QString();
      type = callType;
   } else if (callType == qMetaTypeId<QVariant>()) {
      type = callType;
      qvariantPtr = new (&allocData) QVariant();
   } else if (callType == qMetaTypeId<QList<QObject *> >()) {
      type = callType;
      qlistPtr = new (&allocData) QList<QObject *>();
   } else {
      type = -1;
      qvariantPtr = new (&allocData) QVariant(callType, (void *)0);
   }
}

void MetaCallArgument::fromScriptValue(int callType, QDeclarativeEngine *engine, const QScriptValue &value)
{
   if (type != 0) {
      cleanup();
      type = 0;
   }

   if (callType == qMetaTypeId<QScriptValue>()) {
      qscriptValuePtr = new (&allocData) QScriptValue(value);
      type = qMetaTypeId<QScriptValue>();
   } else if (callType == QMetaType::Int) {
      intValue = quint32(value.toInt32());
      type = callType;
   } else if (callType == QMetaType::UInt) {
      intValue = quint32(value.toUInt32());
      type = callType;
   } else if (callType == QMetaType::Bool) {
      boolValue = value.toBool();
      type = callType;
   } else if (callType == QMetaType::Double) {
      doubleValue = double(value.toNumber());
      type = callType;
   } else if (callType == QMetaType::Float) {
      floatValue = float(value.toNumber());
      type = callType;
   } else if (callType == QMetaType::QString) {
      if (value.isNull() || value.isUndefined()) {
         qstringPtr = new (&allocData) QString();
      } else {
         qstringPtr = new (&allocData) QString(value.toString());
      }
      type = callType;
   } else if (callType == QMetaType::QObjectStar) {
      qobjectPtr = value.toQObject();
      type = callType;
   } else if (callType == qMetaTypeId<QVariant>()) {
      QVariant other = QDeclarativeEnginePrivate::get(engine)->scriptValueToVariant(value);
      qvariantPtr = new (&allocData) QVariant(other);
      type = callType;
   } else if (callType == qMetaTypeId<QList<QObject *> >()) {
      qlistPtr = new (&allocData) QList<QObject *>();
      if (value.isArray()) {
         int length = value.property(QLatin1String("length")).toInt32();
         for (int ii = 0; ii < length; ++ii) {
            QScriptValue arrayItem = value.property(ii);
            QObject *d = arrayItem.toQObject();
            qlistPtr->append(d);
         }
      } else if (QObject *d = value.toQObject()) {
         qlistPtr->append(d);
      }
      type = callType;
   } else {
      qvariantPtr = new (&allocData) QVariant();
      type = -1;

      QDeclarativeEnginePrivate *priv = QDeclarativeEnginePrivate::get(engine);
      QVariant v = priv->scriptValueToVariant(value);
      if (v.userType() == callType) {
         *qvariantPtr = v;
      } else if (v.canConvert((QVariant::Type)callType)) {
         *qvariantPtr = v;
         qvariantPtr->convert((QVariant::Type)callType);
      } else if (const QMetaObject *mo = priv->rawMetaObjectForType(callType)) {
         QObject *obj = priv->toQObject(v);

         if (obj) {
            const QMetaObject *objMo = obj->metaObject();
            while (objMo && objMo != mo) {
               objMo = objMo->superClass();
            }
            if (!objMo) {
               obj = 0;
            }
         }

         *qvariantPtr = QVariant(callType, &obj);
      } else {
         *qvariantPtr = QVariant(callType, (void *)0);
      }
   }
}

QScriptDeclarativeClass::Value MetaCallArgument::toValue(QDeclarativeEngine *e)
{
   QScriptEngine *engine = QDeclarativeEnginePrivate::getScriptEngine(e);

   if (type == qMetaTypeId<QScriptValue>()) {
      return QScriptDeclarativeClass::Value(engine, *qscriptValuePtr);
   } else if (type == QMetaType::Int) {
      return QScriptDeclarativeClass::Value(engine, int(intValue));
   } else if (type == QMetaType::UInt) {
      return QScriptDeclarativeClass::Value(engine, uint(intValue));
   } else if (type == QMetaType::Bool) {
      return QScriptDeclarativeClass::Value(engine, boolValue);
   } else if (type == QMetaType::Double) {
      return QScriptDeclarativeClass::Value(engine, doubleValue);
   } else if (type == QMetaType::Float) {
      return QScriptDeclarativeClass::Value(engine, floatValue);
   } else if (type == QMetaType::QString) {
      return QScriptDeclarativeClass::Value(engine, *qstringPtr);
   } else if (type == QMetaType::QObjectStar) {
      if (qobjectPtr) {
         QDeclarativeData::get(qobjectPtr, true)->setImplicitDestructible();
      }
      QDeclarativeEnginePrivate *priv = QDeclarativeEnginePrivate::get(e);
      return QScriptDeclarativeClass::Value(engine, priv->objectClass->newQObject(qobjectPtr));
   } else if (type == qMetaTypeId<QList<QObject *> >()) {
      QList<QObject *> &list = *qlistPtr;
      QScriptValue rv = engine->newArray(list.count());
      QDeclarativeEnginePrivate *priv = QDeclarativeEnginePrivate::get(e);
      for (int ii = 0; ii < list.count(); ++ii) {
         QObject *object = list.at(ii);
         QDeclarativeData::get(object, true)->setImplicitDestructible();
         rv.setProperty(ii, priv->objectClass->newQObject(object));
      }
      return QScriptDeclarativeClass::Value(engine, rv);
   } else if (type == -1 || type == qMetaTypeId<QVariant>()) {
      QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(e);
      QScriptValue rv = ep->scriptValueFromVariant(*qvariantPtr);
      if (rv.isQObject()) {
         QObject *object = rv.toQObject();
         if (object) {
            QDeclarativeData::get(object, true)->setImplicitDestructible();
         }
      }
      return QScriptDeclarativeClass::Value(engine, rv);
   } else {
      return QScriptDeclarativeClass::Value();
   }
}

int QDeclarativeObjectMethodScriptClass::enumType(const QMetaObject *meta, const QString &strname)
{
   QByteArray str = strname.toUtf8();
   QByteArray scope;
   QByteArray name;
   int scopeIdx = str.lastIndexOf("::");
   if (scopeIdx != -1) {
      scope = str.left(scopeIdx);
      name = str.mid(scopeIdx + 2);
   } else {
      name = str;
   }
   for (int i = meta->enumeratorCount() - 1; i >= 0; --i) {
      QMetaEnum m = meta->enumerator(i);
      if ((m.name() == name) && (scope.isEmpty() || (m.scope() == scope))) {
         return QVariant::Int;
      }
   }
   return QVariant::Invalid;
}

QDeclarativeObjectMethodScriptClass::Value QDeclarativeObjectMethodScriptClass::call(Object *o, QScriptContext *ctxt)
{
   MethodData *method = static_cast<MethodData *>(o);

   if (method->data.relatedIndex == -1) {
      return callPrecise(method->object, method->data, ctxt);
   } else {
      return callOverloaded(method, ctxt);
   }
}

QDeclarativeObjectMethodScriptClass::Value
QDeclarativeObjectMethodScriptClass::callPrecise(QObject *object, const QDeclarativePropertyCache::Data &data,
      QScriptContext *ctxt)
{
   if (data.flags & QDeclarativePropertyCache::Data::HasArguments) {

      QMetaMethod m = object->metaObject()->method(data.coreIndex);
      QList<QByteArray> argTypeNames = m.parameterTypes();
      QVarLengthArray<int, 9> argTypes(argTypeNames.count());

      // ### Cache
      for (int ii = 0; ii < argTypeNames.count(); ++ii) {
         argTypes[ii] = QMetaType::type(argTypeNames.at(ii));
         if (argTypes[ii] == QVariant::Invalid) {
            argTypes[ii] = enumType(object->metaObject(), QString::fromLatin1(argTypeNames.at(ii)));
         }
         if (argTypes[ii] == QVariant::Invalid) {
            return Value(ctxt, ctxt->throwError(QString::fromLatin1("Unknown method parameter type: %1").arg(QLatin1String(
                                                   argTypeNames.at(ii)))));
         }
      }

      if (argTypes.count() > ctxt->argumentCount()) {
         return Value(ctxt, ctxt->throwError(QLatin1String("Insufficient arguments")));
      }

      return callMethod(object, data.coreIndex, data.propType, argTypes.count(), argTypes.data(), ctxt);

   } else {

      return callMethod(object, data.coreIndex, data.propType, 0, 0, ctxt);

   }
}

QDeclarativeObjectMethodScriptClass::Value
QDeclarativeObjectMethodScriptClass::callMethod(QObject *object, int index,
      int returnType, int argCount, int *argTypes,
      QScriptContext *ctxt)
{
   if (argCount > 0) {

      QVarLengthArray<MetaCallArgument, 9> args(argCount + 1);
      args[0].initAsType(returnType, engine);

      for (int ii = 0; ii < argCount; ++ii) {
         args[ii + 1].fromScriptValue(argTypes[ii], engine, ctxt->argument(ii));
      }

      QVarLengthArray<void *, 9> argData(args.count());
      for (int ii = 0; ii < args.count(); ++ii) {
         argData[ii] = args[ii].dataPtr();
      }

      QMetaObject::metacall(object, QMetaObject::InvokeMetaMethod, index, argData.data());

      return args[0].toValue(engine);

   } else if (returnType != 0) {

      MetaCallArgument arg;
      arg.initAsType(returnType, engine);

      void *args[] = { arg.dataPtr() };

      QMetaObject::metacall(object, QMetaObject::InvokeMetaMethod, index, args);

      return arg.toValue(engine);

   } else {

      void *args[] = { 0 };
      QMetaObject::metacall(object, QMetaObject::InvokeMetaMethod, index, args);
      return Value();

   }
}

/*!
Resolve the overloaded method to call.  The algorithm works conceptually like this:
    1.  Resolve the set of overloads it is *possible* to call.
        Impossible overloads include those that have too many parameters or have parameters
        of unknown type.
    2.  Filter the set of overloads to only contain those with the closest number of
        parameters.
        For example, if we are called with 3 parameters and there are 2 overloads that
        take 2 parameters and one that takes 3, eliminate the 2 parameter overloads.
    3.  Find the best remaining overload based on its match score.
        If two or more overloads have the same match score, call the last one.  The match
        score is constructed by adding the matchScore() result for each of the parameters.
*/
QDeclarativeObjectMethodScriptClass::Value
QDeclarativeObjectMethodScriptClass::callOverloaded(MethodData *method, QScriptContext *ctxt)
{
   int argumentCount = ctxt->argumentCount();

   QDeclarativePropertyCache::Data *best = 0;
   int bestParameterScore = INT_MAX;
   int bestMatchScore = INT_MAX;

   QDeclarativePropertyCache::Data dummy;
   QDeclarativePropertyCache::Data *attempt = &method->data;

   do {
      QList<QByteArray> methodArgTypeNames;

      if (attempt->flags & QDeclarativePropertyCache::Data::HasArguments) {
         methodArgTypeNames = method->object->metaObject()->method(attempt->coreIndex).parameterTypes();
      }

      int methodArgumentCount = methodArgTypeNames.count();

      if (methodArgumentCount > argumentCount) {
         continue;   // We don't have sufficient arguments to call this method
      }

      int methodParameterScore = argumentCount - methodArgumentCount;
      if (methodParameterScore > bestParameterScore) {
         continue;   // We already have a better option
      }

      int methodMatchScore = 0;
      QVarLengthArray<int, 9> methodArgTypes(methodArgumentCount);

      bool unknownArgument = false;
      for (int ii = 0; ii < methodArgumentCount; ++ii) {
         methodArgTypes[ii] = QMetaType::type(methodArgTypeNames.at(ii));
         if (methodArgTypes[ii] == QVariant::Invalid)
            methodArgTypes[ii] = enumType(method->object->metaObject(),
                                          QString::fromLatin1(methodArgTypeNames.at(ii)));
         if (methodArgTypes[ii] == QVariant::Invalid) {
            unknownArgument = true;
            break;
         }
         methodMatchScore += matchScore(ctxt->argument(ii), methodArgTypes[ii], methodArgTypeNames.at(ii));
      }
      if (unknownArgument) {
         continue;   // We don't understand all the parameters
      }

      if (bestParameterScore > methodParameterScore || bestMatchScore > methodMatchScore) {
         best = attempt;
         bestParameterScore = methodParameterScore;
         bestMatchScore = methodMatchScore;
      }

      if (bestParameterScore == 0 && bestMatchScore == 0) {
         break;   // We can't get better than that
      }

   } while ((attempt = relatedMethod(method->object, attempt, dummy)) != 0);

   if (best) {
      return callPrecise(method->object, *best, ctxt);
   } else {
      QString error = QLatin1String("Unable to determine callable overload.  Candidates are:");
      QDeclarativePropertyCache::Data *candidate = &method->data;
      while (candidate) {
         error += QLatin1String("\n    ") + QString::fromUtf8(method->object->metaObject()->method(
                     candidate->coreIndex).signature());
         candidate = relatedMethod(method->object, candidate, dummy);
      }
      return Value(ctxt, ctxt->throwError(error));
   }
}

/*!
    Returns the match score for converting \a actual to be of type \a conversionType.  A
    zero score means "perfect match" whereas a higher score is worse.

    The conversion table is copied out of the QtScript callQtMethod() function.
*/
int QDeclarativeObjectMethodScriptClass::matchScore(const QScriptValue &actual, int conversionType,
      const QByteArray &conversionTypeName)
{
   if (actual.isNumber()) {
      switch (conversionType) {
         case QMetaType::Double:
            return 0;
         case QMetaType::Float:
            return 1;
         case QMetaType::LongLong:
         case QMetaType::ULongLong:
            return 2;
         case QMetaType::Long:
         case QMetaType::ULong:
            return 3;
         case QMetaType::Int:
         case QMetaType::UInt:
            return 4;
         case QMetaType::Short:
         case QMetaType::UShort:
            return 5;
            break;
         case QMetaType::Char:
         case QMetaType::UChar:
            return 6;
         default:
            return 10;
      }
   } else if (actual.isString()) {
      switch (conversionType) {
         case QMetaType::QString:
            return 0;
         default:
            return 10;
      }
   } else if (actual.isBoolean()) {
      switch (conversionType) {
         case QMetaType::Bool:
            return 0;
         default:
            return 10;
      }
   } else if (actual.isDate()) {
      switch (conversionType) {
         case QMetaType::QDateTime:
            return 0;
         case QMetaType::QDate:
            return 1;
         case QMetaType::QTime:
            return 2;
         default:
            return 10;
      }
   } else if (actual.isRegExp()) {
      switch (conversionType) {
         case QMetaType::QRegExp:
            return 0;
         default:
            return 10;
      }
   } else if (actual.isVariant()) {
      if (conversionType == qMetaTypeId<QVariant>()) {
         return 0;
      } else if (actual.toVariant().userType() == conversionType) {
         return 0;
      } else {
         return 10;
      }
   } else if (actual.isArray()) {
      switch (conversionType) {
         case QMetaType::QStringList:
         case QMetaType::QVariantList:
            return 5;
         default:
            return 10;
      }
   } else if (actual.isQObject()) {
      switch (conversionType) {
         case QMetaType::QObjectStar:
            return 0;
         default:
            return 10;
      }
   } else if (actual.isNull()) {
      switch (conversionType) {
         case QMetaType::VoidStar:
         case QMetaType::QObjectStar:
            return 0;
         default:
            if (!conversionTypeName.endsWith('*')) {
               return 10;
            } else {
               return 0;
            }
      }
   } else {
      return 10;
   }
}

static inline int QMetaObject_methods(const QMetaObject *metaObject)
{
   struct Private {
      int revision;
      int className;
      int classInfoCount, classInfoData;
      int methodCount, methodData;
   };

   return reinterpret_cast<const Private *>(metaObject->d.data)->methodCount;
}

static QByteArray QMetaMethod_name(const QMetaMethod &m)
{
   QByteArray sig = m.signature();
   int paren = sig.indexOf('(');
   if (paren == -1) {
      return sig;
   } else {
      return sig.left(paren);
   }
}

/*!
Returns the next related method, if one, or 0.
*/
QDeclarativePropertyCache::Data *
QDeclarativeObjectMethodScriptClass::relatedMethod(QObject *object, QDeclarativePropertyCache::Data *current,
      QDeclarativePropertyCache::Data &dummy)
{
   QDeclarativePropertyCache *cache = QDeclarativeData::get(object)->propertyCache;
   if (current->relatedIndex == -1) {
      return 0;
   }

   if (cache) {
      return cache->method(current->relatedIndex);
   } else {
      const QMetaObject *mo = object->metaObject();
      int methodOffset = mo->methodCount() - QMetaObject_methods(mo);

      while (methodOffset > current->relatedIndex) {
         mo = mo->superClass();
         methodOffset -= QMetaObject_methods(mo);
      }

      QMetaMethod method = mo->method(current->relatedIndex);
      dummy.load(method);

      // Look for overloaded methods
      QByteArray methodName = QMetaMethod_name(method);
      for (int ii = current->relatedIndex - 1; ii >= methodOffset; --ii) {
         if (methodName == QMetaMethod_name(mo->method(ii))) {
            dummy.relatedIndex = ii;
            return &dummy;
         }
      }

      return &dummy;
   }
}

QT_END_NAMESPACE

