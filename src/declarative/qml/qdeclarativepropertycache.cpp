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

#include <qdeclarativepropertycache_p.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativebinding_p.h>
#include <QtCore/qdebug.h>

Q_DECLARE_METATYPE(QScriptValue)

QT_BEGIN_NAMESPACE

QDeclarativePropertyCache::Data::Flags QDeclarativePropertyCache::Data::flagsForProperty(const QMetaProperty &p,
      QDeclarativeEngine *engine)
{
   int propType = p.userType();

   Flags flags;

   if (p.isConstant()) {
      flags |= Data::IsConstant;
   }
   if (p.isWritable()) {
      flags |= Data::IsWritable;
   }
   if (p.isResettable()) {
      flags |= Data::IsResettable;
   }

   if (propType == qMetaTypeId<QDeclarativeBinding *>()) {
      flags |= Data::IsQmlBinding;
   } else if (propType == qMetaTypeId<QScriptValue>()) {
      flags |= Data::IsQScriptValue;
   } else if (p.isEnumType()) {
      flags |= Data::IsEnumType;
   } else {
      QDeclarativeMetaType::TypeCategory cat = engine ? QDeclarativeEnginePrivate::get(engine)->typeCategory(propType)
            : QDeclarativeMetaType::typeCategory(propType);
      if (cat == QDeclarativeMetaType::Object) {
         flags |= Data::IsQObjectDerived;
      } else if (cat == QDeclarativeMetaType::List) {
         flags |= Data::IsQList;
      }
   }

   return flags;
}

void QDeclarativePropertyCache::Data::load(const QMetaProperty &p, QDeclarativeEngine *engine)
{
   propType = p.userType();
   if (QVariant::Type(propType) == QVariant::LastType) {
      propType = qMetaTypeId<QVariant>();
   }
   coreIndex = p.propertyIndex();
   notifyIndex = p.notifySignalIndex();
   flags = flagsForProperty(p, engine);
   revision = p.revision();
}

void QDeclarativePropertyCache::Data::load(const QMetaMethod &m)
{
   coreIndex = m.methodIndex();
   relatedIndex = -1;
   flags |= Data::IsFunction;
   if (m.methodType() == QMetaMethod::Signal) {
      flags |= Data::IsSignal;
   }
   propType = QVariant::Invalid;

   const char *returnType = m.typeName();
   if (returnType) {
      propType = QMetaType::type(returnType);
   }

   QList<QByteArray> params = m.parameterTypes();
   if (!params.isEmpty()) {
      flags |= Data::HasArguments;
   }
   revision = m.revision();
}


/*!
Creates a new empty QDeclarativePropertyCache.
*/
QDeclarativePropertyCache::QDeclarativePropertyCache(QDeclarativeEngine *e)
   : QDeclarativeCleanup(e), engine(e)
{
   Q_ASSERT(engine);
}

/*!
Creates a new QDeclarativePropertyCache of \a metaObject.
*/
QDeclarativePropertyCache::QDeclarativePropertyCache(QDeclarativeEngine *e, const QMetaObject *metaObject)
   : QDeclarativeCleanup(e), engine(e)
{
   Q_ASSERT(engine);
   Q_ASSERT(metaObject);

   update(engine, metaObject);
}

QDeclarativePropertyCache::~QDeclarativePropertyCache()
{
   clear();
}

void QDeclarativePropertyCache::clear()
{
   for (int ii = 0; ii < indexCache.count(); ++ii) {
      if (indexCache.at(ii)) {
         indexCache.at(ii)->release();
      }
   }

   for (int ii = 0; ii < methodIndexCache.count(); ++ii) {
      RData *data = methodIndexCache.at(ii);
      if (data) {
         data->release();
      }
   }

   for (StringCache::ConstIterator iter = stringCache.begin();
         iter != stringCache.end(); ++iter) {
      RData *data = (*iter);
      data->release();
   }

   for (IdentifierCache::ConstIterator iter = identifierCache.begin();
         iter != identifierCache.end(); ++iter) {
      RData *data = (*iter);
      data->release();
   }

   indexCache.clear();
   methodIndexCache.clear();
   stringCache.clear();
   identifierCache.clear();
}

QDeclarativePropertyCache::Data QDeclarativePropertyCache::create(const QMetaObject *metaObject,
      const QString &property)
{
   Q_ASSERT(metaObject);

   QDeclarativePropertyCache::Data rv;
   {
      const QMetaObject *cmo = metaObject;
      while (cmo) {
         int idx = metaObject->indexOfProperty(property.toUtf8());
         if (idx != -1) {
            QMetaProperty p = metaObject->property(idx);
            if (p.isScriptable()) {
               rv.load(metaObject->property(idx));
               return rv;
            } else {
               while (cmo && cmo->propertyOffset() >= idx) {
                  cmo = cmo->superClass();
               }
            }
         } else {
            cmo = 0;
         }
      }
   }

   int methodCount = metaObject->methodCount();
   for (int ii = methodCount - 1; ii >= 3; --ii) { // >=3 to block the destroyed signal and deleteLater() slot
      QMetaMethod m = metaObject->method(ii);

      if (m.access() == QMetaMethod::Private) {
         continue;
      }

      QString methodName = QString::fromUtf8(m.signature());

      int parenIdx = methodName.indexOf(QLatin1Char('('));
      Q_ASSERT(parenIdx != -1);

      QStringView methodNameRef = methodName.leftView(parenIdx);

      if (methodNameRef == property) {
         rv.load(m);
         return rv;
      }
   }

   return rv;
}

QDeclarativePropertyCache *QDeclarativePropertyCache::copy() const
{
   QDeclarativePropertyCache *cache = new QDeclarativePropertyCache(engine);
   cache->indexCache = indexCache;
   cache->methodIndexCache = methodIndexCache;
   cache->stringCache = stringCache;
   cache->identifierCache = identifierCache;
   cache->allowedRevisionCache = allowedRevisionCache;

   for (int ii = 0; ii < indexCache.count(); ++ii) {
      if (indexCache.at(ii)) {
         indexCache.at(ii)->addref();
      }
   }
   for (int ii = 0; ii < methodIndexCache.count(); ++ii) {
      if (methodIndexCache.at(ii)) {
         methodIndexCache.at(ii)->addref();
      }
   }
   for (StringCache::ConstIterator iter = stringCache.begin(); iter != stringCache.end(); ++iter) {
      (*iter)->addref();
   }
   for (IdentifierCache::ConstIterator iter = identifierCache.begin(); iter != identifierCache.end(); ++iter) {
      (*iter)->addref();
   }

   return cache;
}

void QDeclarativePropertyCache::append(QDeclarativeEngine *engine, const QMetaObject *metaObject,
                                       Data::Flag propertyFlags, Data::Flag methodFlags, Data::Flag signalFlags)
{
   append(engine, metaObject, -1, propertyFlags, methodFlags, signalFlags);
}

void QDeclarativePropertyCache::append(QDeclarativeEngine *engine, const QMetaObject *metaObject,
                                       int revision,
                                       Data::Flag propertyFlags, Data::Flag methodFlags, Data::Flag signalFlags)
{
   Q_UNUSED(revision);

   allowedRevisionCache.append(0);

   QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(engine);
   int methodCount = metaObject->methodCount();
   // 3 to block the destroyed signal and the deleteLater() slot
   int methodOffset = qMax(3, metaObject->methodOffset());

   methodIndexCache.resize(methodCount);
   for (int ii = methodOffset; ii < methodCount; ++ii) {
      QMetaMethod m = metaObject->method(ii);
      if (m.access() == QMetaMethod::Private) {
         continue;
      }
      QString methodName = QString::fromUtf8(m.signature());

      int parenIdx = methodName.indexOf(QLatin1Char('('));
      Q_ASSERT(parenIdx != -1);
      methodName = methodName.left(parenIdx);

      RData *data = new RData;
      data->identifier = enginePriv->objectClass->createPersistentIdentifier(methodName);
      methodIndexCache[ii] = data;

      data->load(m);
      if (m.methodType() == QMetaMethod::Slot || m.methodType() == QMetaMethod::Method) {
         data->flags |= methodFlags;
      } else if (m.methodType() == QMetaMethod::Signal) {
         data->flags |= signalFlags;
      }

      data->metaObjectOffset = allowedRevisionCache.count() - 1;

      if (stringCache.contains(methodName)) {
         RData *old = stringCache[methodName];
         // We only overload methods in the same class, exactly like C++
         if (old->flags & Data::IsFunction && old->coreIndex >= methodOffset) {
            data->relatedIndex = old->coreIndex;
         }
         data->overrideIndexIsProperty = !bool(old->flags & Data::IsFunction);
         data->overrideIndex = old->coreIndex;
         stringCache[methodName]->release();
         identifierCache[data->identifier.identifier]->release();
      }

      stringCache.insert(methodName, data);
      identifierCache.insert(data->identifier.identifier, data);
      data->addref();
      data->addref();
   }

   int propCount = metaObject->propertyCount();
   int propOffset = metaObject->propertyOffset();

   indexCache.resize(propCount);
   for (int ii = propOffset; ii < propCount; ++ii) {
      QMetaProperty p = metaObject->property(ii);
      if (!p.isScriptable()) {
         continue;
      }

      QString propName = QString::fromUtf8(p.name());

      RData *data = new RData;
      data->identifier = enginePriv->objectClass->createPersistentIdentifier(propName);
      indexCache[ii] = data;

      data->load(p, engine);
      data->flags |= propertyFlags;

      data->metaObjectOffset = allowedRevisionCache.count() - 1;

      if (stringCache.contains(propName)) {
         RData *old = stringCache[propName];
         data->overrideIndexIsProperty = !bool(old->flags & Data::IsFunction);
         data->overrideIndex = old->coreIndex;
         stringCache[propName]->release();
         identifierCache[data->identifier.identifier]->release();
      }

      stringCache.insert(propName, data);
      identifierCache.insert(data->identifier.identifier, data);
      data->addref();
      data->addref();
   }
}

void QDeclarativePropertyCache::updateRecur(QDeclarativeEngine *engine, const QMetaObject *metaObject)
{
   if (!metaObject) {
      return;
   }

   updateRecur(engine, metaObject->superClass());

   append(engine, metaObject);
}

void QDeclarativePropertyCache::update(QDeclarativeEngine *engine, const QMetaObject *metaObject)
{
   Q_ASSERT(engine);
   Q_ASSERT(metaObject);

   clear();

   // Optimization to prevent unnecessary reallocation of lists
   indexCache.reserve(metaObject->propertyCount());
   methodIndexCache.reserve(metaObject->methodCount());

   updateRecur(engine, metaObject);
}

QDeclarativePropertyCache::Data *
QDeclarativePropertyCache::property(int index) const
{
   if (index < 0 || index >= indexCache.count()) {
      return 0;
   }

   return indexCache.at(index);
}

QDeclarativePropertyCache::Data *
QDeclarativePropertyCache::method(int index) const
{
   if (index < 0 || index >= methodIndexCache.count()) {
      return 0;
   }

   return methodIndexCache.at(index);
}

QDeclarativePropertyCache::Data *
QDeclarativePropertyCache::property(const QString &str) const
{
   return stringCache.value(str);
}

QString QDeclarativePropertyCache::Data::name(QObject *object)
{
   if (!object) {
      return QString();
   }

   return name(object->metaObject());
}

QString QDeclarativePropertyCache::Data::name(const QMetaObject *metaObject)
{
   if (!metaObject || coreIndex == -1) {
      return QString();
   }

   if (flags & IsFunction) {
      QMetaMethod m = metaObject->method(coreIndex);

      QString name = QString::fromUtf8(m.signature());
      int parenIdx = name.indexOf(QLatin1Char('('));
      if (parenIdx != -1) {
         name = name.left(parenIdx);
      }
      return name;
   } else {
      QMetaProperty p = metaObject->property(coreIndex);
      return QString::fromUtf8(p.name());
   }
}

QStringList QDeclarativePropertyCache::propertyNames() const
{
   return stringCache.keys();
}

QDeclarativePropertyCache::Data *QDeclarativePropertyCache::property(QDeclarativeEngine *engine, QObject *obj,
      const QScriptDeclarativeClass::Identifier &name, Data &local)
{
   QDeclarativePropertyCache::Data *rv = 0;

   QDeclarativeEnginePrivate *enginePrivate = QDeclarativeEnginePrivate::get(engine);

   QDeclarativePropertyCache *cache = 0;
   QDeclarativeData *ddata = QDeclarativeData::get(obj);
   if (ddata && ddata->propertyCache && ddata->propertyCache->qmlEngine() == engine) {
      cache = ddata->propertyCache;
   }
   if (!cache) {
      cache = enginePrivate->cache(obj);
      if (cache && ddata && !ddata->propertyCache) {
         cache->addref();
         ddata->propertyCache = cache;
      }
   }

   if (cache) {
      rv = cache->property(name);
   } else {
      local = QDeclarativePropertyCache::create(obj->metaObject(), enginePrivate->objectClass->toString(name));
      if (local.isValid()) {
         rv = &local;
      }
   }

   return rv;
}

QDeclarativePropertyCache::Data *QDeclarativePropertyCache::property(QDeclarativeEngine *engine, QObject *obj,
      const QString &name, Data &local)
{
   QDeclarativePropertyCache::Data *rv = 0;

   if (!engine) {
      local = QDeclarativePropertyCache::create(obj->metaObject(), name);
      if (local.isValid()) {
         rv = &local;
      }
   } else {
      QDeclarativeEnginePrivate *enginePrivate = QDeclarativeEnginePrivate::get(engine);

      QDeclarativePropertyCache *cache = 0;
      QDeclarativeData *ddata = QDeclarativeData::get(obj);
      if (ddata && ddata->propertyCache && ddata->propertyCache->qmlEngine() == engine) {
         cache = ddata->propertyCache;
      }
      if (!cache) {
         cache = enginePrivate->cache(obj);
         if (cache && ddata && !ddata->propertyCache) {
            cache->addref();
            ddata->propertyCache = cache;
         }
      }

      if (cache) {
         rv = cache->property(name);
      } else {
         local = QDeclarativePropertyCache::create(obj->metaObject(), name);
         if (local.isValid()) {
            rv = &local;
         }
      }
   }

   return rv;
}

QT_END_NAMESPACE
