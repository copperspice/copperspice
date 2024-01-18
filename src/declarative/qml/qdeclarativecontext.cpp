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

#include "qdeclarativecontext.h"
#include "private/qdeclarativecontext_p.h"

#include "private/qdeclarativecomponent_p.h"
#include "private/qdeclarativeexpression_p.h"
#include "private/qdeclarativeengine_p.h"
#include "qdeclarativeengine.h"
#include "private/qdeclarativecompiledbindings_p.h"
#include "qdeclarativeinfo.h"
#include "private/qdeclarativeglobalscriptclass_p.h"

#include <qscriptengine.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qdebug.h>

#include <private/qscriptdeclarativeclass_p.h>

QT_BEGIN_NAMESPACE

QDeclarativeContextPrivate::QDeclarativeContextPrivate()
   : data(0), notifyIndex(-1)
{
}

/*!
    \class QDeclarativeContext
    \since 4.7
    \brief The QDeclarativeContext class defines a context within a QML engine.
    \mainclass

    Contexts allow data to be exposed to the QML components instantiated by the
    QML engine.

    Each QDeclarativeContext contains a set of properties, distinct from its QObject
    properties, that allow data to be explicitly bound to a context by name.  The
    context properties are defined and updated by calling
    QDeclarativeContext::setContextProperty().  The following example shows a Qt model
    being bound to a context and then accessed from a QML file.

    \code
    QDeclarativeEngine engine;
    QStringListModel modelData;
    QDeclarativeContext *context = new QDeclarativeContext(engine.rootContext());
    context->setContextProperty("myModel", &modelData);

    QDeclarativeComponent component(&engine);
    component.setData("import QtQuick 1.0\nListView { model: myModel }", QUrl());
    QObject *window = component.create(context);
    \endcode

    Note it is the responsibility of the creator to delete any QDeclarativeContext it
    constructs. If the \c context object in the example is no longer needed when the
    \c window component instance is destroyed, the \c context must be destroyed explicitly.
    The simplest way to ensure this is to set \c window as the parent of \c context.

    To simplify binding and maintaining larger data sets, a context object can be set
    on a QDeclarativeContext.  All the properties of the context object are available
    by name in the context, as though they were all individually added through calls
    to QDeclarativeContext::setContextProperty().  Changes to the property's values are
    detected through the property's notify signal.  Setting a context object is both
    faster and easier than manually adding and maintaing context property values.

    The following example has the same effect as the previous one, but it uses a context
    object.

    \code
    class MyDataSet : ... {
        ...
        Q_PROPERTY(QAbstractItemModel *myModel READ model NOTIFY modelChanged)
        ...
    };

    MyDataSet myDataSet;
    QDeclarativeEngine engine;
    QDeclarativeContext *context = new QDeclarativeContext(engine.rootContext());
    context->setContextObject(&myDataSet);

    QDeclarativeComponent component(&engine);
    component.setData("import QtQuick 1.0\nListView { model: myModel }", QUrl());
    component.create(context);
    \endcode

    All properties added explicitly by QDeclarativeContext::setContextProperty() take
    precedence over the context object's properties.

    \section2 The Context Hierarchy

    Contexts form a hierarchy. The root of this hierarchy is the QML engine's
    \l {QDeclarativeEngine::rootContext()}{root context}. Child contexts inherit
    the context properties of their parents; if a child context sets a context property
    that already exists in its parent, the new context property overrides that of the
    parent.

    The following example defines two contexts - \c context1 and \c context2.  The
    second context overrides the "b" context property inherited from the first with a
    new value.

    \code
    QDeclarativeEngine engine;
    QDeclarativeContext *context1 = new QDeclarativeContext(engine.rootContext());
    QDeclarativeContext *context2 = new QDeclarativeContext(context1);

    context1->setContextProperty("a", 12);
    context1->setContextProperty("b", 12);

    context2->setContextProperty("b", 15);
    \endcode

    While QML objects instantiated in a context are not strictly owned by that
    context, their bindings are.  If a context is destroyed, the property bindings of
    outstanding QML objects will stop evaluating.

    \warning Setting the context object or adding new context properties after an object
    has been created in that context is an expensive operation (essentially forcing all bindings
    to reevaluate). Thus whenever possible you should complete "setup" of the context
    before using it to create any objects.

    \sa {Using QML Bindings in C++ Applications}
*/

/*! \internal */
QDeclarativeContext::QDeclarativeContext(QDeclarativeEngine *e, bool)
   : QObject(*(new QDeclarativeContextPrivate))
{
   Q_D(QDeclarativeContext);
   d->data = new QDeclarativeContextData(this);

   d->data->engine = e;
}

/*!
    Create a new QDeclarativeContext as a child of \a engine's root context, and the
    QObject \a parent.
*/
QDeclarativeContext::QDeclarativeContext(QDeclarativeEngine *engine, QObject *parent)
   : QObject(*(new QDeclarativeContextPrivate), parent)
{
   Q_D(QDeclarativeContext);
   d->data = new QDeclarativeContextData(this);

   d->data->setParent(engine ? QDeclarativeContextData::get(engine->rootContext()) : 0);
}

/*!
    Create a new QDeclarativeContext with the given \a parentContext, and the
    QObject \a parent.
*/
QDeclarativeContext::QDeclarativeContext(QDeclarativeContext *parentContext, QObject *parent)
   : QObject(*(new QDeclarativeContextPrivate), parent)
{
   Q_D(QDeclarativeContext);
   d->data = new QDeclarativeContextData(this);

   d->data->setParent(parentContext ? QDeclarativeContextData::get(parentContext) : 0);
}

/*!
    \internal
*/
QDeclarativeContext::QDeclarativeContext(QDeclarativeContextData *data)
   : QObject(*(new QDeclarativeContextPrivate), 0)
{
   Q_D(QDeclarativeContext);
   d->data = data;
}

/*!
    Destroys the QDeclarativeContext.

    Any expressions, or sub-contexts dependent on this context will be
    invalidated, but not destroyed (unless they are parented to the QDeclarativeContext
    object).
 */
QDeclarativeContext::~QDeclarativeContext()
{
   Q_D(QDeclarativeContext);

   if (!d->data->isInternal) {
      d->data->destroy();
   }
}

/*!
    Returns whether the context is valid.

    To be valid, a context must have a engine, and it's contextObject(), if any,
    must not have been deleted.
*/
bool QDeclarativeContext::isValid() const
{
   Q_D(const QDeclarativeContext);
   return d->data && d->data->isValid();
}

/*!
    Return the context's QDeclarativeEngine, or 0 if the context has no QDeclarativeEngine or the
    QDeclarativeEngine was destroyed.
*/
QDeclarativeEngine *QDeclarativeContext::engine() const
{
   Q_D(const QDeclarativeContext);
   return d->data->engine;
}

/*!
    Return the context's parent QDeclarativeContext, or 0 if this context has no
    parent or if the parent has been destroyed.
*/
QDeclarativeContext *QDeclarativeContext::parentContext() const
{
   Q_D(const QDeclarativeContext);
   return d->data->parent ? d->data->parent->asQDeclarativeContext() : 0;
}

/*!
    Return the context object, or 0 if there is no context object.
*/
QObject *QDeclarativeContext::contextObject() const
{
   Q_D(const QDeclarativeContext);
   return d->data->contextObject;
}

/*!
    Set the context \a object.
*/
void QDeclarativeContext::setContextObject(QObject *object)
{
   Q_D(QDeclarativeContext);

   QDeclarativeContextData *data = d->data;

   if (data->isInternal) {
      qWarning("QDeclarativeContext: Cannot set context object for internal context.");
      return;
   }

   if (!isValid()) {
      qWarning("QDeclarativeContext: Cannot set context object on invalid context.");
      return;
   }

   data->contextObject = object;
}

/*!
    Set a the \a value of the \a name property on this context.
*/
void QDeclarativeContext::setContextProperty(const QString &name, const QVariant &value)
{
   Q_D(QDeclarativeContext);
   if (d->notifyIndex == -1) {
      d->notifyIndex = this->metaObject()->methodCount();
   }

   QDeclarativeContextData *data = d->data;

   if (data->isInternal) {
      qWarning("QDeclarativeContext: Cannot set property on internal context.");
      return;
   }

   if (!isValid()) {
      qWarning("QDeclarativeContext: Cannot set property on invalid context.");
      return;
   }

   if (data->engine) {
      bool ok;
      QObject *o = QDeclarativeEnginePrivate::get(data->engine)->toQObject(value, &ok);
      if (ok) {
         setContextProperty(name, o);
         return;
      }
   }

   if (!data->propertyNames) {
      data->propertyNames = new QDeclarativeIntegerCache(data->engine);
   }

   int idx = data->propertyNames->value(name);
   if (idx == -1) {
      data->propertyNames->add(name, data->idValueCount + d->propertyValues.count());
      d->propertyValues.append(value);

      data->refreshExpressions();
   } else {
      d->propertyValues[idx] = value;
      QMetaObject::activate(this, idx + d->notifyIndex, 0);
   }
}

/*!
    Set the \a value of the \a name property on this context.

    QDeclarativeContext does \bold not take ownership of \a value.
*/
void QDeclarativeContext::setContextProperty(const QString &name, QObject *value)
{
   Q_D(QDeclarativeContext);
   if (d->notifyIndex == -1) {
      d->notifyIndex = this->metaObject()->methodCount();
   }

   QDeclarativeContextData *data = d->data;

   if (data->isInternal) {
      qWarning("QDeclarativeContext: Cannot set property on internal context.");
      return;
   }

   if (!isValid()) {
      qWarning("QDeclarativeContext: Cannot set property on invalid context.");
      return;
   }

   if (!data->propertyNames) {
      data->propertyNames = new QDeclarativeIntegerCache(data->engine);
   }
   int idx = data->propertyNames->value(name);

   if (idx == -1) {
      data->propertyNames->add(name, data->idValueCount + d->propertyValues.count());
      d->propertyValues.append(QVariant::fromValue(value));

      data->refreshExpressions();
   } else {
      d->propertyValues[idx] = QVariant::fromValue(value);
      QMetaObject::activate(this, idx + d->notifyIndex, 0);
   }
}

/*!
  Returns the value of the \a name property for this context
  as a QVariant.
 */
QVariant QDeclarativeContext::contextProperty(const QString &name) const
{
   Q_D(const QDeclarativeContext);
   QVariant value;
   int idx = -1;

   QDeclarativeContextData *data = d->data;

   if (data->propertyNames) {
      idx = data->propertyNames->value(name);
   }

   if (idx == -1) {
      QByteArray utf8Name = name.toUtf8();
      if (data->contextObject) {
         QObject *obj = data->contextObject;
         QDeclarativePropertyCache::Data local;
         QDeclarativePropertyCache::Data *property =
            QDeclarativePropertyCache::property(data->engine, obj, name, local);

         if (property) {
            value = obj->metaObject()->property(property->coreIndex).read(obj);
         }
      }
      if (!value.isValid() && parentContext()) {
         value = parentContext()->contextProperty(name);
      }
   } else {
      if (idx >= d->propertyValues.count()) {
         value = QVariant::fromValue(data->idValues[idx - d->propertyValues.count()].data());
      } else {
         value = d->propertyValues[idx];
      }
   }

   return value;
}

/*!
    Resolves the URL \a src relative to the URL of the
    containing component.

    \sa QDeclarativeEngine::baseUrl(), setBaseUrl()
*/
QUrl QDeclarativeContext::resolvedUrl(const QUrl &src)
{
   Q_D(QDeclarativeContext);
   return d->data->resolvedUrl(src);
}

QUrl QDeclarativeContextData::resolvedUrl(const QUrl &src)
{
   QDeclarativeContextData *ctxt = this;

   if (src.isRelative() && !src.isEmpty()) {
      if (ctxt) {
         while (ctxt) {
            if (ctxt->url.isValid()) {
               break;
            } else {
               ctxt = ctxt->parent;
            }
         }

         if (ctxt) {
            return ctxt->url.resolved(src);
         } else if (engine) {
            return engine->baseUrl().resolved(src);
         }
      }
      return QUrl();
   } else {
      return src;
   }
}


/*!
    Explicitly sets the url resolvedUrl() will use for relative references to \a baseUrl.

    Calling this function will override the url of the containing
    component used by default.

    \sa resolvedUrl()
*/
void QDeclarativeContext::setBaseUrl(const QUrl &baseUrl)
{
   Q_D(QDeclarativeContext);

   d->data->url = baseUrl;
}

/*!
    Returns the base url of the component, or the containing component
    if none is set.
*/
QUrl QDeclarativeContext::baseUrl() const
{
   Q_D(const QDeclarativeContext);
   const QDeclarativeContextData *data = d->data;
   while (data && data->url.isEmpty()) {
      data = data->parent;
   }

   if (data) {
      return data->url;
   } else {
      return QUrl();
   }
}

int QDeclarativeContextPrivate::context_count(QDeclarativeListProperty<QObject> *prop)
{
   QDeclarativeContext *context = static_cast<QDeclarativeContext *>(prop->object);
   QDeclarativeContextPrivate *d = QDeclarativeContextPrivate::get(context);
   int contextProperty = (int)(quintptr)prop->data;

   if (d->propertyValues.at(contextProperty).userType() != qMetaTypeId<QList<QObject *> >()) {
      return 0;
   } else {
      return ((const QList<QObject> *)d->propertyValues.at(contextProperty).constData())->count();
   }
}

QObject *QDeclarativeContextPrivate::context_at(QDeclarativeListProperty<QObject> *prop, int index)
{
   QDeclarativeContext *context = static_cast<QDeclarativeContext *>(prop->object);
   QDeclarativeContextPrivate *d = QDeclarativeContextPrivate::get(context);
   int contextProperty = (int)(quintptr)prop->data;

   if (d->propertyValues.at(contextProperty).userType() != qMetaTypeId<QList<QObject *> >()) {
      return 0;
   } else {
      return ((const QList<QObject *> *)d->propertyValues.at(contextProperty).constData())->at(index);
   }
}


QDeclarativeContextData::QDeclarativeContextData()
   : parent(0), engine(0), isInternal(false), publicContext(0), propertyNames(0), contextObject(0),
     imports(0), childContexts(0), nextChild(0), prevChild(0), expressions(0), contextObjects(0),
     contextGuards(0), idValues(0), idValueCount(0), optimizedBindings(0), linkedContext(0),
     componentAttached(0)
{
}

QDeclarativeContextData::QDeclarativeContextData(QDeclarativeContext *ctxt)
   : parent(0), engine(0), isInternal(false), publicContext(ctxt), propertyNames(0), contextObject(0),
     imports(0), childContexts(0), nextChild(0), prevChild(0), expressions(0), contextObjects(0),
     contextGuards(0), idValues(0), idValueCount(0), optimizedBindings(0), linkedContext(0),
     componentAttached(0)
{
}

void QDeclarativeContextData::invalidate()
{
   while (childContexts) {
      childContexts->invalidate();
   }

   while (componentAttached) {
      QDeclarativeComponentAttached *a = componentAttached;
      componentAttached = a->next;
      if (componentAttached) {
         componentAttached->prev = &componentAttached;
      }

      a->next = 0;
      a->prev = 0;

      emit a->destruction();
   }

   if (prevChild) {
      *prevChild = nextChild;
      if (nextChild) {
         nextChild->prevChild = prevChild;
      }
      nextChild = 0;
      prevChild = 0;
   }

   engine = 0;
   parent = 0;
}

void QDeclarativeContextData::clearContext()
{
   if (engine) {
      while (componentAttached) {
         QDeclarativeComponentAttached *a = componentAttached;
         componentAttached = a->next;
         if (componentAttached) {
            componentAttached->prev = &componentAttached;
         }

         a->next = 0;
         a->prev = 0;

         emit a->destruction();
      }
   }

   QDeclarativeAbstractExpression *expression = expressions;
   while (expression) {
      QDeclarativeAbstractExpression *nextExpression = expression->m_nextExpression;

      expression->m_context = 0;
      expression->m_prevExpression = 0;
      expression->m_nextExpression = 0;

      expression = nextExpression;
   }
   expressions = 0;
}

void QDeclarativeContextData::destroy()
{
   if (linkedContext) {
      linkedContext->destroy();
   }

   if (engine) {
      invalidate();
   }

   clearContext();

   while (contextObjects) {
      QDeclarativeData *co = contextObjects;
      contextObjects = contextObjects->nextContextObject;

      co->context = 0;
      co->outerContext = 0;
      co->nextContextObject = 0;
      co->prevContextObject = 0;
   }

   QDeclarativeGuardedContextData *contextGuard = contextGuards;
   while (contextGuard) {
      QDeclarativeGuardedContextData *next = contextGuard->m_next;
      contextGuard->m_next = 0;
      contextGuard->m_prev = 0;
      contextGuard->m_contextData = 0;
      contextGuard = next;
   }
   contextGuards = 0;

   if (propertyNames) {
      propertyNames->release();
   }

   if (imports) {
      imports->release();
   }

   if (optimizedBindings) {
      optimizedBindings->release();
   }

   delete [] idValues;

   if (isInternal) {
      delete publicContext;
   }

   delete this;
}

void QDeclarativeContextData::setParent(QDeclarativeContextData *p)
{
   if (p) {
      parent = p;
      engine = p->engine;
      nextChild = p->childContexts;
      if (nextChild) {
         nextChild->prevChild = &nextChild;
      }
      prevChild = &p->childContexts;
      p->childContexts = this;
   }
}

/*
Refreshes all expressions that could possibly depend on this context.  Refreshing flushes all
context-tree dependent caches in the expressions, and should occur every time the context tree
 *structure* (not values) changes.
*/
void QDeclarativeContextData::refreshExpressions()
{
   QDeclarativeContextData *child = childContexts;
   while (child) {
      child->refreshExpressions();
      child = child->nextChild;
   }

   QDeclarativeAbstractExpression *expression = expressions;
   while (expression) {
      expression->refresh();
      expression = expression->m_nextExpression;
   }
}

void QDeclarativeContextData::addObject(QObject *o)
{
   QDeclarativeData *data = QDeclarativeData::get(o, true);

   Q_ASSERT(data->context == 0);

   data->context = this;
   data->outerContext = this;

   data->nextContextObject = contextObjects;
   if (data->nextContextObject) {
      data->nextContextObject->prevContextObject = &data->nextContextObject;
   }
   data->prevContextObject = &contextObjects;
   contextObjects = data;
}

void QDeclarativeContextData::addImportedScript(const QDeclarativeParser::Object::ScriptBlock &script)
{
   if (!engine) {
      return;
   }

   QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(engine);
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   const QString &code = script.code;
   const QString &url = script.file;
   const QDeclarativeParser::Object::ScriptBlock::Pragmas &pragmas = script.pragmas;

   Q_ASSERT(!url.isEmpty());

   if (pragmas & QDeclarativeParser::Object::ScriptBlock::Shared) {

      QHash<QString, QScriptValue>::Iterator iter = enginePriv->m_sharedScriptImports.find(url);
      if (iter == enginePriv->m_sharedScriptImports.end()) {
         QScriptContext *scriptContext = QScriptDeclarativeClass::pushCleanContext(scriptEngine);

         scriptContext->pushScope(enginePriv->contextClass->newUrlContext(url));
         scriptContext->pushScope(enginePriv->globalClass->staticGlobalObject());

         QScriptValue scope = QScriptDeclarativeClass::newStaticScopeObject(scriptEngine);
         scriptContext->pushScope(scope);

         scriptEngine->evaluate(code, url, 1);

         if (scriptEngine->hasUncaughtException()) {
            QDeclarativeError error;
            QDeclarativeExpressionPrivate::exceptionToError(scriptEngine, error);
            enginePriv->warning(error);
         }

         scriptEngine->popContext();

         iter = enginePriv->m_sharedScriptImports.insert(url, scope);
      }

      importedScripts.append(*iter);

   } else {

      QScriptContext *scriptContext = QScriptDeclarativeClass::pushCleanContext(scriptEngine);

      scriptContext->pushScope(enginePriv->contextClass->newUrlContext(this, 0, url));
      scriptContext->pushScope(enginePriv->globalClass->staticGlobalObject());

      QScriptValue scope = QScriptDeclarativeClass::newStaticScopeObject(scriptEngine);
      scriptContext->pushScope(scope);

      scriptEngine->evaluate(code, url, 1);

      if (scriptEngine->hasUncaughtException()) {
         QDeclarativeError error;
         QDeclarativeExpressionPrivate::exceptionToError(scriptEngine, error);
         enginePriv->warning(error);
      }

      scriptEngine->popContext();

      importedScripts.append(scope);

   }
}

void QDeclarativeContextData::setIdProperty(int idx, QObject *obj)
{
   idValues[idx] = obj;
   idValues[idx].context = this;
}

void QDeclarativeContextData::setIdPropertyData(QDeclarativeIntegerCache *data)
{
   Q_ASSERT(!propertyNames);
   propertyNames = data;
   propertyNames->addref();

   idValueCount = data->count();
   idValues = new ContextGuard[idValueCount];
}

QString QDeclarativeContextData::findObjectId(const QObject *obj) const
{
   if (!idValues || !propertyNames) {
      return QString();
   }

   for (int i = 0; i < idValueCount; i++) {
      if (idValues[i] == obj) {
         return propertyNames->findId(i);
      }
   }

   if (linkedContext) {
      return linkedContext->findObjectId(obj);
   }
   return QString();
}

QDeclarativeContext *QDeclarativeContextData::asQDeclarativeContext()
{
   if (!publicContext) {
      publicContext = new QDeclarativeContext(this);
   }
   return publicContext;
}

QDeclarativeContextPrivate *QDeclarativeContextData::asQDeclarativeContextPrivate()
{
   return QDeclarativeContextPrivate::get(asQDeclarativeContext());
}

QT_END_NAMESPACE
