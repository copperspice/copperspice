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

#include "private/qdeclarativecontextscriptclass_p.h"

#include "private/qdeclarativeengine_p.h"
#include "private/qdeclarativecontext_p.h"
#include "private/qdeclarativetypenamescriptclass_p.h"
#include "private/qdeclarativelistscriptclass_p.h"
#include "private/qdeclarativeguard_p.h"

QT_BEGIN_NAMESPACE

struct ContextData : public QScriptDeclarativeClass::Object {
   ContextData() : overrideObject(0), isSharedContext(true) {}
   ContextData(QDeclarativeContextData *c, QObject *o)
      : context(c), scopeObject(o), overrideObject(0), isSharedContext(false), isUrlContext(false) {}
   QDeclarativeGuardedContextData context;
   QDeclarativeGuard<QObject> scopeObject;
   QObject *overrideObject;
   bool isSharedContext: 1;
   bool isUrlContext: 1;

   QDeclarativeContextData *getContext(QDeclarativeEngine *engine) {
      if (isSharedContext) {
         return QDeclarativeEnginePrivate::get(engine)->sharedContext;
      } else {
         return context.contextData();
      }
   }

   QObject *getScope(QDeclarativeEngine *engine) {
      if (isSharedContext) {
         return QDeclarativeEnginePrivate::get(engine)->sharedScope;
      } else {
         return scopeObject.data();
      }
   }
};

struct UrlContextData : public ContextData {
   UrlContextData(QDeclarativeContextData *c, QObject *o, const QString &u)
      : ContextData(c, o), url(u) {
      isUrlContext = true;
   }
   UrlContextData(const QString &u)
      : ContextData(0, 0), url(u) {
      isUrlContext = true;
   }
   QString url;
};

/*
    The QDeclarativeContextScriptClass handles property access for a QDeclarativeContext
    via QtScript.
 */
QDeclarativeContextScriptClass::QDeclarativeContextScriptClass(QDeclarativeEngine *bindEngine)
   : QScriptDeclarativeClass(QDeclarativeEnginePrivate::getScriptEngine(bindEngine)), engine(bindEngine),
     lastScopeObject(0), lastContext(0), lastData(0), lastPropertyIndex(-1)
{
}

QDeclarativeContextScriptClass::~QDeclarativeContextScriptClass()
{
}

QScriptValue QDeclarativeContextScriptClass::newContext(QDeclarativeContextData *context, QObject *scopeObject)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   return newObject(scriptEngine, this, new ContextData(context, scopeObject));
}

QScriptValue QDeclarativeContextScriptClass::newUrlContext(QDeclarativeContextData *context, QObject *scopeObject,
      const QString &url)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   return newObject(scriptEngine, this, new UrlContextData(context, scopeObject, url));
}

QScriptValue QDeclarativeContextScriptClass::newUrlContext(const QString &url)
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   return newObject(scriptEngine, this, new UrlContextData(url));
}

QScriptValue QDeclarativeContextScriptClass::newSharedContext()
{
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   return newObject(scriptEngine, this, new ContextData());
}

QDeclarativeContextData *QDeclarativeContextScriptClass::contextFromValue(const QScriptValue &v)
{
   if (scriptClass(v) != this) {
      return 0;
   }

   ContextData *data = (ContextData *)object(v);
   return data->getContext(engine);
}

QUrl QDeclarativeContextScriptClass::urlFromValue(const QScriptValue &v)
{
   if (scriptClass(v) != this) {
      return QUrl();
   }

   ContextData *data = (ContextData *)object(v);
   if (data->isUrlContext) {
      return QUrl(static_cast<UrlContextData *>(data)->url);
   } else {
      return QUrl();
   }
}

QObject *QDeclarativeContextScriptClass::setOverrideObject(QScriptValue &v, QObject *override)
{
   if (scriptClass(v) != this) {
      return 0;
   }

   ContextData *data = (ContextData *)object(v);
   QObject *rv = data->overrideObject;
   data->overrideObject = override;
   return rv;
}

QScriptClass::QueryFlags
QDeclarativeContextScriptClass::queryProperty(Object *object, const Identifier &name,
      QScriptClass::QueryFlags flags)
{
   Q_UNUSED(flags);

   lastScopeObject = 0;
   lastContext = 0;
   lastData = 0;
   lastPropertyIndex = -1;

   QDeclarativeContextData *bindContext = ((ContextData *)object)->getContext(engine);
   QObject *scopeObject = ((ContextData *)object)->getScope(engine);
   if (!bindContext) {
      return 0;
   }

   QObject *overrideObject = ((ContextData *)object)->overrideObject;
   if (overrideObject) {
      QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);
      QScriptClass::QueryFlags rv =
         ep->objectClass->queryProperty(overrideObject, name, flags, bindContext,
                                        QDeclarativeObjectScriptClass::ImplicitObject |
                                        QDeclarativeObjectScriptClass::SkipAttachedProperties);
      if (rv) {
         lastScopeObject = overrideObject;
         lastContext = bindContext;
         return rv;
      }
   }

   bool includeTypes = true;
   while (bindContext) {
      QScriptClass::QueryFlags rv =
         queryProperty(bindContext, scopeObject, name, flags, includeTypes);
      scopeObject = 0; // Only applies to the first context
      includeTypes = false; // Only applies to the first context
      if (rv) {
         return rv;
      }
      bindContext = bindContext->parent;
   }

   return 0;
}

QScriptClass::QueryFlags
QDeclarativeContextScriptClass::queryProperty(QDeclarativeContextData *bindContext, QObject *scopeObject,
      const Identifier &name,
      QScriptClass::QueryFlags flags,
      bool includeTypes)
{
   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);

   lastPropertyIndex = bindContext->propertyNames ? bindContext->propertyNames->value(name) : -1;
   if (lastPropertyIndex != -1) {
      lastContext = bindContext;
      return QScriptClass::HandlesReadAccess;
   }

   if (includeTypes && bindContext->imports) {
      QDeclarativeTypeNameCache::Data *data = bindContext->imports->data(name);

      if (data)  {
         lastData = data;
         lastContext = bindContext;
         lastScopeObject = scopeObject;
         return QScriptClass::HandlesReadAccess;
      }
   }

   if (scopeObject) {
      QScriptClass::QueryFlags rv =
         ep->objectClass->queryProperty(scopeObject, name, flags, bindContext,
                                        QDeclarativeObjectScriptClass::ImplicitObject | QDeclarativeObjectScriptClass::SkipAttachedProperties);
      if (rv) {
         lastScopeObject = scopeObject;
         lastContext = bindContext;
         return rv;
      }
   }

   if (bindContext->contextObject) {
      QScriptClass::QueryFlags rv =
         ep->objectClass->queryProperty(bindContext->contextObject, name, flags, bindContext,
                                        QDeclarativeObjectScriptClass::ImplicitObject | QDeclarativeObjectScriptClass::SkipAttachedProperties);

      if (rv) {
         lastScopeObject = bindContext->contextObject;
         lastContext = bindContext;
         return rv;
      }
   }

   return 0;
}

QDeclarativeContextScriptClass::Value
QDeclarativeContextScriptClass::property(Object *object, const Identifier &name)
{
   Q_UNUSED(object);

   QDeclarativeContextData *bindContext = lastContext;
   Q_ASSERT(bindContext);

   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   if (lastData) {

      if (lastData->type) {
         return Value(scriptEngine, ep->typeNameClass->newObject(lastScopeObject, lastData->type));
      } else if (lastData->typeNamespace) {
         return Value(scriptEngine, ep->typeNameClass->newObject(lastScopeObject, lastData->typeNamespace));
      } else {
         int index = lastData->importedScriptIndex;
         if (index < bindContext->importedScripts.count()) {
            return Value(scriptEngine, bindContext->importedScripts.at(index));
         } else {
            return Value();
         }
      }

   } else if (lastScopeObject) {

      return ep->objectClass->property(lastScopeObject, name);

   } else if (lastPropertyIndex != -1) {

      QScriptValue rv;
      if (lastPropertyIndex < bindContext->idValueCount) {
         rv =  ep->objectClass->newQObject(bindContext->idValues[lastPropertyIndex].data());

         if (ep->captureProperties) {
            ep->capturedProperties << QDeclarativeEnginePrivate::CapturedProperty(
                                      &bindContext->idValues[lastPropertyIndex].bindings);
         }
      } else {
         QDeclarativeContextPrivate *cp = bindContext->asQDeclarativeContextPrivate();
         const QVariant &value = cp->propertyValues.at(lastPropertyIndex);
         if (value.userType() == qMetaTypeId<QList<QObject *> >()) {
            rv = ep->listClass->newList(QDeclarativeListProperty<QObject>(bindContext->asQDeclarativeContext(),
                                        (void *)lastPropertyIndex, 0, QDeclarativeContextPrivate::context_count, QDeclarativeContextPrivate::context_at),
                                        qMetaTypeId<QDeclarativeListProperty<QObject> >());
         } else {
            rv = ep->scriptValueFromVariant(value);
         }

         if (ep->captureProperties) {
            ep->capturedProperties << QDeclarativeEnginePrivate::CapturedProperty(bindContext->asQDeclarativeContext(), -1,
                                   lastPropertyIndex + cp->notifyIndex);
         }
      }

      return Value(scriptEngine, rv);

   } else {

      return Value(scriptEngine, lastFunction);

   }
}

void QDeclarativeContextScriptClass::setProperty(Object *object, const Identifier &name,
      const QScriptValue &value)
{
   Q_UNUSED(object);
   Q_ASSERT(lastScopeObject);

   QDeclarativeContextData *bindContext = lastContext;
   Q_ASSERT(bindContext);

   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);

   ep->objectClass->setProperty(lastScopeObject, name, value, context(), bindContext);
}

QT_END_NAMESPACE
