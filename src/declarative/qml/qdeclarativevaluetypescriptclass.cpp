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

#include <qdeclarativevaluetypescriptclass_p.h>
#include <qdeclarativebinding_p.h>
#include <qdeclarativeproperty_p.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativeguard_p.h>
#include <QtScript/qscriptcontextinfo.h>

QT_BEGIN_NAMESPACE

struct QDeclarativeValueTypeObject : public QScriptDeclarativeClass::Object {
   enum Type { Reference, Copy };
   QDeclarativeValueTypeObject(Type t) : objectType(t) {}
   Type objectType;
   QDeclarativeValueType *type;
};

struct QDeclarativeValueTypeReference : public QDeclarativeValueTypeObject {
   QDeclarativeValueTypeReference() : QDeclarativeValueTypeObject(Reference) {}
   QDeclarativeGuard<QObject> object;
   int property;
};

struct QDeclarativeValueTypeCopy : public QDeclarativeValueTypeObject {
   QDeclarativeValueTypeCopy() : QDeclarativeValueTypeObject(Copy) {}
   QVariant value;
};

QDeclarativeValueTypeScriptClass::QDeclarativeValueTypeScriptClass(QDeclarativeEngine *bindEngine)
   : QScriptDeclarativeClass(QDeclarativeEnginePrivate::getScriptEngine(bindEngine)), engine(bindEngine)
{
}

QDeclarativeValueTypeScriptClass::~QDeclarativeValueTypeScriptClass()
{
}

QScriptValue QDeclarativeValueTypeScriptClass::newObject(QObject *object, int coreIndex, QDeclarativeValueType *type)
{
   QDeclarativeValueTypeReference *ref = new QDeclarativeValueTypeReference;
   ref->type = type;
   ref->object = object;
   ref->property = coreIndex;
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);
   return QScriptDeclarativeClass::newObject(scriptEngine, this, ref);
}

QScriptValue QDeclarativeValueTypeScriptClass::newObject(const QVariant &v, QDeclarativeValueType *type)
{
   QDeclarativeValueTypeCopy *copy = new QDeclarativeValueTypeCopy;
   copy->type = type;
   copy->value = v;
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);
   return QScriptDeclarativeClass::newObject(scriptEngine, this, copy);
}

QScriptClass::QueryFlags
QDeclarativeValueTypeScriptClass::queryProperty(Object *obj, const Identifier &name,
      QScriptClass::QueryFlags)
{
   QDeclarativeValueTypeObject *o = static_cast<QDeclarativeValueTypeObject *>(obj);

   m_lastIndex = -1;

   QByteArray propName = toString(name).toUtf8();

   m_lastIndex = o->type->metaObject()->indexOfProperty(propName.constData());
   if (m_lastIndex == -1) {
      return 0;
   }

   QScriptClass::QueryFlags rv = 0;

   if (o->objectType == QDeclarativeValueTypeObject::Reference) {
      QDeclarativeValueTypeReference *ref = static_cast<QDeclarativeValueTypeReference *>(o);

      if (!ref->object) {
         return 0;
      }

      QMetaProperty prop = ref->object->metaObject()->property(m_lastIndex);

      rv = QScriptClass::HandlesReadAccess;
      if (prop.isWritable()) {
         rv |= QScriptClass::HandlesWriteAccess;
      }
   } else {
      rv = QScriptClass::HandlesReadAccess | QScriptClass::HandlesWriteAccess;
   }

   return rv;
}

QDeclarativeValueTypeScriptClass::Value QDeclarativeValueTypeScriptClass::property(Object *obj, const Identifier &)
{
   QDeclarativeValueTypeObject *o = static_cast<QDeclarativeValueTypeObject *>(obj);

   QVariant rv;
   if (o->objectType == QDeclarativeValueTypeObject::Reference) {
      QDeclarativeValueTypeReference *ref = static_cast<QDeclarativeValueTypeReference *>(obj);

      QMetaProperty p = ref->type->metaObject()->property(m_lastIndex);
      ref->type->read(ref->object, ref->property);
      rv = p.read(ref->type);
   } else {
      QDeclarativeValueTypeCopy *copy = static_cast<QDeclarativeValueTypeCopy *>(obj);

      QMetaProperty p = copy->type->metaObject()->property(m_lastIndex);
      copy->type->setValue(copy->value);
      rv = p.read(copy->type);
   }

   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);
   return Value(scriptEngine, static_cast<QDeclarativeEnginePrivate *>(QObjectPrivate::get(
                   engine))->scriptValueFromVariant(rv));
}

void QDeclarativeValueTypeScriptClass::setProperty(Object *obj, const Identifier &,
      const QScriptValue &value)
{
   QDeclarativeValueTypeObject *o = static_cast<QDeclarativeValueTypeObject *>(obj);

   QVariant v = QDeclarativeEnginePrivate::get(engine)->scriptValueToVariant(value);

   if (o->objectType == QDeclarativeValueTypeObject::Reference) {
      QDeclarativeValueTypeReference *ref = static_cast<QDeclarativeValueTypeReference *>(obj);

      ref->type->read(ref->object, ref->property);
      QMetaProperty p = ref->type->metaObject()->property(m_lastIndex);

      QDeclarativeBinding *newBinding = 0;
      if (value.isFunction() && !value.isRegExp()) {
         QDeclarativeContextData *ctxt = QDeclarativeEnginePrivate::get(engine)->getContext(context());

         QDeclarativePropertyCache::Data cacheData;
         cacheData.flags = QDeclarativePropertyCache::Data::IsWritable;
         cacheData.propType = ref->object->metaObject()->property(ref->property).userType();
         cacheData.coreIndex = ref->property;

         QDeclarativePropertyCache::ValueTypeData valueTypeData;
         valueTypeData.valueTypeCoreIdx = m_lastIndex;
         valueTypeData.valueTypePropType = p.userType();

         newBinding = new QDeclarativeBinding(value, ref->object, ctxt);
         QScriptContextInfo ctxtInfo(context());
         newBinding->setSourceLocation(ctxtInfo.fileName(), ctxtInfo.functionStartLineNumber());
         QDeclarativeProperty prop = QDeclarativePropertyPrivate::restore(cacheData, valueTypeData, ref->object, ctxt);
         newBinding->setTarget(prop);
         if (newBinding->expression().contains(QLatin1String("this"))) {
            newBinding->setEvaluateFlags(newBinding->evaluateFlags() | QDeclarativeBinding::RequiresThisObject);
         }
      }

      QDeclarativeAbstractBinding *delBinding =
         QDeclarativePropertyPrivate::setBinding(ref->object, ref->property, m_lastIndex, newBinding);
      if (delBinding) {
         delBinding->destroy();
      }

      if (p.isEnumType() && (QMetaType::Type)v.type() == QMetaType::Double) {
         v = v.toInt();
      }
      p.write(ref->type, v);
      ref->type->write(ref->object, ref->property, 0);

   } else {
      QDeclarativeValueTypeCopy *copy = static_cast<QDeclarativeValueTypeCopy *>(obj);
      copy->type->setValue(copy->value);
      QMetaProperty p = copy->type->metaObject()->property(m_lastIndex);
      p.write(copy->type, v);
      copy->value = copy->type->value();
   }
}

QVariant QDeclarativeValueTypeScriptClass::toVariant(Object *obj, bool *ok)
{
   QDeclarativeValueTypeObject *o = static_cast<QDeclarativeValueTypeObject *>(obj);

   if (o->objectType == QDeclarativeValueTypeObject::Reference) {
      QDeclarativeValueTypeReference *ref = static_cast<QDeclarativeValueTypeReference *>(obj);

      if (ok) {
         *ok = true;
      }

      if (ref->object) {
         ref->type->read(ref->object, ref->property);
         return ref->type->value();
      }
   } else {
      QDeclarativeValueTypeCopy *copy = static_cast<QDeclarativeValueTypeCopy *>(obj);

      if (ok) {
         *ok = true;
      }

      return copy->value;
   }

   return QVariant();
}

QVariant QDeclarativeValueTypeScriptClass::toVariant(const QScriptValue &value)
{
   Q_ASSERT(scriptClass(value) == this);

   return toVariant(object(value), 0);
}

QT_END_NAMESPACE

