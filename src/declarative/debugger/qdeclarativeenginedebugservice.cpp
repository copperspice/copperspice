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

#include <qdeclarativeenginedebugservice_p.h>
#include <qdeclarativeboundsignal_p.h>
#include <qdeclarativeengine.h>
#include <qdeclarativemetatype_p.h>
#include <qdeclarativeproperty.h>
#include <qdeclarativeproperty_p.h>
#include <qdeclarativebinding_p.h>
#include <qdeclarativecontext_p.h>
#include <qdeclarativewatcher_p.h>
#include <qdeclarativevaluetype_p.h>
#include <qdeclarativevmemetaobject_p.h>
#include <qdeclarativeexpression_p.h>
#include <qdeclarativepropertychanges_p.h>

#include <QtCore/qdebug.h>
#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QDeclarativeEngineDebugService, qmlEngineDebugService);

QDeclarativeEngineDebugService *QDeclarativeEngineDebugService::instance()
{
   return qmlEngineDebugService();
}

QDeclarativeEngineDebugService::QDeclarativeEngineDebugService(QObject *parent)
   : QDeclarativeDebugService(QLatin1String("DeclarativeDebugger"), parent),
     m_watch(new QDeclarativeWatcher(this))
{
   QObject::connect(m_watch, SIGNAL(propertyChanged(int, int, QMetaProperty, QVariant)),
                    this, SLOT(propertyChanged(int, int, QMetaProperty, QVariant)));
}

QDataStream &operator<<(QDataStream &ds,
                        const QDeclarativeEngineDebugService::QDeclarativeObjectData &data)
{
   ds << data.url << data.lineNumber << data.columnNumber << data.idString
      << data.objectName << data.objectType << data.objectId << data.contextId
      << data.parentId;
   return ds;
}

QDataStream &operator>>(QDataStream &ds,
                        QDeclarativeEngineDebugService::QDeclarativeObjectData &data)
{
   ds >> data.url >> data.lineNumber >> data.columnNumber >> data.idString
      >> data.objectName >> data.objectType >> data.objectId >> data.contextId
      >> data.parentId;
   return ds;
}

QDataStream &operator<<(QDataStream &ds,
                        const QDeclarativeEngineDebugService::QDeclarativeObjectProperty &data)
{
   ds << (int)data.type << data.name << data.value << data.valueTypeName
      << data.binding << data.hasNotifySignal;
   return ds;
}

QDataStream &operator>>(QDataStream &ds,
                        QDeclarativeEngineDebugService::QDeclarativeObjectProperty &data)
{
   int type;
   ds >> type >> data.name >> data.value >> data.valueTypeName
      >> data.binding >> data.hasNotifySignal;
   data.type = (QDeclarativeEngineDebugService::QDeclarativeObjectProperty::Type)type;
   return ds;
}

static inline bool isSignalPropertyName(const QString &signalName)
{
   // see QmlCompiler::isSignalPropertyName
   return signalName.length() >= 3 && signalName.startsWith(QLatin1String("on")) &&
          signalName.at(2).isLetter() && signalName.at(2).isUpper();
}

static bool hasValidSignal(QObject *object, const QString &propertyName)
{
   if (!isSignalPropertyName(propertyName)) {
      return false;
   }

   QString signalName = propertyName.mid(2);
   signalName[0] = signalName.at(0).toLower();

   int sigIdx = QDeclarativePropertyPrivate::findSignalByName(object->metaObject(), signalName.toLatin1()).methodIndex();

   if (sigIdx == -1) {
      return false;
   }

   return true;
}

QDeclarativeEngineDebugService::QDeclarativeObjectProperty
QDeclarativeEngineDebugService::propertyData(QObject *obj, int propIdx)
{
   QDeclarativeObjectProperty rv;

   QMetaProperty prop = obj->metaObject()->property(propIdx);

   rv.type = QDeclarativeObjectProperty::Unknown;
   rv.valueTypeName = QString::fromUtf8(prop.typeName());
   rv.name = QString::fromUtf8(prop.name());
   rv.hasNotifySignal = prop.hasNotifySignal();
   QDeclarativeAbstractBinding *binding =
      QDeclarativePropertyPrivate::binding(QDeclarativeProperty(obj, rv.name));
   if (binding) {
      rv.binding = binding->expression();
   }

   if (QDeclarativeValueTypeFactory::isValueType(prop.userType())) {
      rv.type = QDeclarativeObjectProperty::Basic;
   } else if (QDeclarativeMetaType::isQObject(prop.userType()))  {
      rv.type = QDeclarativeObjectProperty::Object;
   } else if (QDeclarativeMetaType::isList(prop.userType())) {
      rv.type = QDeclarativeObjectProperty::List;
   } else if (prop.userType() == QMetaType::QVariant) {
      rv.type = QDeclarativeObjectProperty::Variant;
   }

   QVariant value;
   if (rv.type != QDeclarativeObjectProperty::Unknown && prop.userType() != 0) {
      value = prop.read(obj);
   }
   rv.value = valueContents(value);

   return rv;
}

QVariant QDeclarativeEngineDebugService::valueContents(const QVariant &value) const
{
   int userType = value.userType();

   if (value.type() == QVariant::List) {
      QVariantList contents;
      QVariantList list = value.toList();
      int count = list.size();
      for (int i = 0; i < count; i++) {
         contents << valueContents(list.at(i));
      }
      return contents;
   }

   if (value.type() == QVariant::Map) {
      QVariantMap contents;
      QMapIterator<QString, QVariant> i(value.toMap());
      while (i.hasNext()) {
         i.next();
         contents.insert(i.key(), valueContents(i.value()));
      }
      return contents;
   }

   if (QDeclarativeValueTypeFactory::isValueType(userType)) {
      return value;
   }

   if (QDeclarativeMetaType::isQObject(userType)) {
      QObject *o = QDeclarativeMetaType::toQObject(value);
      if (o) {
         QString name = o->objectName();
         if (name.isEmpty()) {
            name = QLatin1String("<unnamed object>");
         }
         return name;
      }
   }

   return QLatin1String("<unknown value>");
}

void QDeclarativeEngineDebugService::buildObjectDump(QDataStream &message,
      QObject *object, bool recur, bool dumpProperties)
{
   message << objectData(object);

   QObjectList children = object->children();

   int childrenCount = children.count();
   for (int ii = 0; ii < children.count(); ++ii) {
      if (qobject_cast<QDeclarativeContext *>(children[ii]) || QDeclarativeBoundSignal::cast(children[ii])) {
         --childrenCount;
      }
   }

   message << childrenCount << recur;

   QList<QDeclarativeObjectProperty> fakeProperties;

   for (int ii = 0; ii < children.count(); ++ii) {
      QObject *child = children.at(ii);
      if (qobject_cast<QDeclarativeContext *>(child)) {
         continue;
      }
      if (!QDeclarativeBoundSignal::cast(child)) {
         if (recur) {
            buildObjectDump(message, child, recur, dumpProperties);
         } else {
            message << objectData(child);
         }
      }
   }

   if (!dumpProperties) {
      message << 0;
      return;
   }

   QList<int> propertyIndexes;
   for (int ii = 0; ii < object->metaObject()->propertyCount(); ++ii) {
      if (object->metaObject()->property(ii).isScriptable()) {
         propertyIndexes << ii;
      }
   }

   for (int ii = 0; ii < children.count(); ++ii) {
      QObject *child = children.at(ii);
      if (qobject_cast<QDeclarativeContext *>(child)) {
         continue;
      }
      QDeclarativeBoundSignal *signal = QDeclarativeBoundSignal::cast(child);
      if (signal) {
         QDeclarativeObjectProperty prop;
         prop.type = QDeclarativeObjectProperty::SignalProperty;
         prop.hasNotifySignal = false;
         QDeclarativeExpression *expr = signal->expression();
         if (expr) {
            prop.value = expr->expression();
            QObject *scope = expr->scopeObject();
            if (scope) {
               QString sig = QLatin1String(scope->metaObject()->method(signal->index()).signature());
               int lparen = sig.indexOf(QLatin1Char('('));
               if (lparen >= 0) {
                  QString methodName = sig.mid(0, lparen);
                  prop.name = QLatin1String("on") + methodName[0].toUpper()
                              + methodName.mid(1);
               }
            }
         }
         fakeProperties << prop;
      }
   }

   message << propertyIndexes.size() + fakeProperties.count();

   for (int ii = 0; ii < propertyIndexes.size(); ++ii) {
      message << propertyData(object, propertyIndexes.at(ii));
   }

   for (int ii = 0; ii < fakeProperties.count(); ++ii) {
      message << fakeProperties[ii];
   }
}

void QDeclarativeEngineDebugService::prepareDeferredObjects(QObject *obj)
{
   qmlExecuteDeferred(obj);

   QObjectList children = obj->children();
   for (int ii = 0; ii < children.count(); ++ii) {
      QObject *child = children.at(ii);
      prepareDeferredObjects(child);
   }

}

void QDeclarativeEngineDebugService::buildObjectList(QDataStream &message, QDeclarativeContext *ctxt)
{
   QDeclarativeContextData *p = QDeclarativeContextData::get(ctxt);

   QString ctxtName = ctxt->objectName();
   int ctxtId = QDeclarativeDebugService::idForObject(ctxt);

   message << ctxtName << ctxtId;

   int count = 0;

   QDeclarativeContextData *child = p->childContexts;
   while (child) {
      ++count;
      child = child->nextChild;
   }

   message << count;

   child = p->childContexts;
   while (child) {
      buildObjectList(message, child->asQDeclarativeContext());
      child = child->nextChild;
   }

   // Clean deleted objects
   QDeclarativeContextPrivate *ctxtPriv = QDeclarativeContextPrivate::get(ctxt);
   for (int ii = 0; ii < ctxtPriv->instances.count(); ++ii) {
      if (!ctxtPriv->instances.at(ii)) {
         ctxtPriv->instances.removeAt(ii);
         --ii;
      }
   }

   message << ctxtPriv->instances.count();
   for (int ii = 0; ii < ctxtPriv->instances.count(); ++ii) {
      message << objectData(ctxtPriv->instances.at(ii));
   }
}

void QDeclarativeEngineDebugService::buildStatesList(QDeclarativeContext *ctxt, bool cleanList = false)
{
   if (cleanList) {
      m_allStates.clear();
   }

   QDeclarativeContextPrivate *ctxtPriv = QDeclarativeContextPrivate::get(ctxt);
   for (int ii = 0; ii < ctxtPriv->instances.count(); ++ii) {
      buildStatesList(ctxtPriv->instances.at(ii));
   }

   QDeclarativeContextData *child = QDeclarativeContextData::get(ctxt)->childContexts;
   while (child) {
      buildStatesList(child->asQDeclarativeContext());
      child = child->nextChild;
   }
}

void QDeclarativeEngineDebugService::buildStatesList(QObject *obj)
{
   if (QDeclarativeState *state = qobject_cast<QDeclarativeState *>(obj)) {
      m_allStates.append(state);
   }

   QObjectList children = obj->children();
   for (int ii = 0; ii < children.count(); ++ii) {
      buildStatesList(children.at(ii));
   }
}

QDeclarativeEngineDebugService::QDeclarativeObjectData
QDeclarativeEngineDebugService::objectData(QObject *object)
{
   QDeclarativeData *ddata = QDeclarativeData::get(object);
   QDeclarativeObjectData rv;
   if (ddata && ddata->outerContext) {
      rv.url = ddata->outerContext->url;
      rv.lineNumber = ddata->lineNumber;
      rv.columnNumber = ddata->columnNumber;
   } else {
      rv.lineNumber = -1;
      rv.columnNumber = -1;
   }

   QDeclarativeContext *context = qmlContext(object);
   if (context) {
      QDeclarativeContextData *cdata = QDeclarativeContextData::get(context);
      if (cdata) {
         rv.idString = cdata->findObjectId(object);
      }
   }

   rv.objectName = object->objectName();
   rv.objectId = QDeclarativeDebugService::idForObject(object);
   rv.contextId = QDeclarativeDebugService::idForObject(qmlContext(object));
   rv.parentId = QDeclarativeDebugService::idForObject(object->parent());

   QDeclarativeType *type = QDeclarativeMetaType::qmlType(object->metaObject());
   if (type) {
      QString typeName = QLatin1String(type->qmlTypeName());
      int lastSlash = typeName.lastIndexOf(QLatin1Char('/'));
      rv.objectType = lastSlash < 0 ? typeName : typeName.mid(lastSlash + 1);
   } else {
      rv.objectType = QString::fromUtf8(object->metaObject()->className());
      int marker = rv.objectType.indexOf(QLatin1String("_QMLTYPE_"));
      if (marker != -1) {
         rv.objectType = rv.objectType.left(marker);
      }
   }

   return rv;
}

void QDeclarativeEngineDebugService::messageReceived(const QByteArray &message)
{
   QDataStream ds(message);

   int queryId;
   QByteArray type;
   ds >> type >> queryId;

   QByteArray reply;
   QDataStream rs(&reply, QIODevice::WriteOnly);

   if (type == "LIST_ENGINES") {

      rs << QByteArray("LIST_ENGINES_R");
      rs << queryId << m_engines.count();

      for (int ii = 0; ii < m_engines.count(); ++ii) {
         QDeclarativeEngine *engine = m_engines.at(ii);

         QString engineName = engine->objectName();
         int engineId = QDeclarativeDebugService::idForObject(engine);
         rs << engineName << engineId;
      }

   } else if (type == "LIST_OBJECTS") {
      int engineId = -1;
      ds >> engineId;

      QDeclarativeEngine *engine =
         qobject_cast<QDeclarativeEngine *>(QDeclarativeDebugService::objectForId(engineId));

      rs << QByteArray("LIST_OBJECTS_R") << queryId;

      if (engine) {
         buildObjectList(rs, engine->rootContext());
         buildStatesList(engine->rootContext(), true);
      }

   } else if (type == "FETCH_OBJECT") {
      int objectId;
      bool recurse;
      bool dumpProperties = true;

      ds >> objectId >> recurse >> dumpProperties;

      QObject *object = QDeclarativeDebugService::objectForId(objectId);

      rs << QByteArray("FETCH_OBJECT_R") << queryId;

      if (object) {
         if (recurse) {
            prepareDeferredObjects(object);
         }
         buildObjectDump(rs, object, recurse, dumpProperties);
      }

   } else if (type == "WATCH_OBJECT") {
      int objectId;

      ds >> objectId;
      bool ok = m_watch->addWatch(queryId, objectId);

      rs << QByteArray("WATCH_OBJECT_R") << queryId << ok;

   } else if (type == "WATCH_PROPERTY") {
      int objectId;
      QByteArray property;

      ds >> objectId >> property;
      bool ok = m_watch->addWatch(queryId, objectId, property);

      rs << QByteArray("WATCH_PROPERTY_R") << queryId << ok;

   } else if (type == "WATCH_EXPR_OBJECT") {
      int debugId;
      QString expr;

      ds >> debugId >> expr;
      bool ok = m_watch->addWatch(queryId, debugId, expr);

      rs << QByteArray("WATCH_EXPR_OBJECT_R") << queryId << ok;

   } else if (type == "NO_WATCH") {

      m_watch->removeWatch(queryId);
   } else if (type == "EVAL_EXPRESSION") {
      int objectId;
      QString expr;

      ds >> objectId >> expr;

      QObject *object = QDeclarativeDebugService::objectForId(objectId);
      QDeclarativeContext *context = qmlContext(object);
      QVariant result;
      if (object && context) {
         QDeclarativeExpression exprObj(context, object, expr);
         bool undefined = false;
         QVariant value = exprObj.evaluate(&undefined);
         if (undefined) {
            result = QLatin1String("<undefined>");
         } else {
            result = valueContents(value);
         }
      } else {
         result = QLatin1String("<unknown context>");
      }

      rs << QByteArray("EVAL_EXPRESSION_R") << queryId << result;

   } else if (type == "SET_BINDING") {
      int objectId;
      QString propertyName;
      QVariant expr;
      bool isLiteralValue;
      QString filename;
      int line;
      ds >> objectId >> propertyName >> expr >> isLiteralValue;
      if (!ds.atEnd()) { // backward compatibility from 2.1, 2.2
         ds >> filename >> line;
      }
      setBinding(objectId, propertyName, expr, isLiteralValue, filename, line);

      rs << QByteArray("SET_BINDING_R") << queryId;

   } else if (type == "RESET_BINDING") {
      int objectId;
      QString propertyName;
      ds >> objectId >> propertyName;
      resetBinding(objectId, propertyName);

      rs << QByteArray("SET_BINDING_R") << queryId;

   } else if (type == "SET_METHOD_BODY") {
      int objectId;
      QString methodName;
      QString methodBody;
      ds >> objectId >> methodName >> methodBody;
      setMethodBody(objectId, methodName, methodBody);

      rs << QByteArray("SET_BINDING_R") << queryId;
   }
   sendMessage(reply);
}

void QDeclarativeEngineDebugService::setBinding(int objectId,
      const QString &propertyName,
      const QVariant &expression,
      bool isLiteralValue,
      QString filename,
      int line)
{
   QObject *object = objectForId(objectId);
   QDeclarativeContext *context = qmlContext(object);

   if (object && context) {
      QDeclarativeProperty property(object, propertyName, context);
      if (property.isValid()) {

         bool inBaseState = true;

         foreach(QWeakPointer<QDeclarativeState> statePointer, m_allStates) {
            if (QDeclarativeState *state = statePointer.data()) {
               // here we assume that the revert list on itself defines the base state
               if (state->isStateActive() && state->containsPropertyInRevertList(object, propertyName)) {
                  inBaseState = false;

                  QDeclarativeBinding *newBinding = 0;
                  if (!isLiteralValue) {
                     newBinding = new QDeclarativeBinding(expression.toString(), object, context);
                     newBinding->setTarget(property);
                     newBinding->setNotifyOnValueChanged(true);
                     newBinding->setSourceLocation(filename, line);
                  }

                  state->changeBindingInRevertList(object, propertyName, newBinding);

                  if (isLiteralValue) {
                     state->changeValueInRevertList(object, propertyName, expression);
                  }
               }
            }
         }

         if (inBaseState) {
            if (isLiteralValue) {
               property.write(expression);
            } else if (hasValidSignal(object, propertyName)) {
               QDeclarativeExpression *declarativeExpression = new QDeclarativeExpression(context, object, expression.toString());
               QDeclarativePropertyPrivate::setSignalExpression(property, declarativeExpression);
               declarativeExpression->setSourceLocation(filename, line);
            } else if (property.isProperty()) {
               QDeclarativeBinding *binding = new QDeclarativeBinding(expression.toString(), object, context);
               binding->setTarget(property);
               binding->setSourceLocation(filename, line);
               binding->setNotifyOnValueChanged(true);
               QDeclarativeAbstractBinding *oldBinding = QDeclarativePropertyPrivate::setBinding(property, binding);
               if (oldBinding) {
                  oldBinding->destroy();
               }
               binding->update();
            } else {
               qWarning() << "QDeclarativeEngineDebugService::setBinding: unable to set property" << propertyName << "on object" <<
                          object;
            }
         }

      } else {
         // not a valid property
         if (QDeclarativePropertyChanges *propertyChanges = qobject_cast<QDeclarativePropertyChanges *>(object)) {
            if (isLiteralValue) {
               propertyChanges->changeValue(propertyName, expression);
            } else {
               propertyChanges->changeExpression(propertyName, expression.toString());
            }
         } else {
            qWarning() << "QDeclarativeEngineDebugService::setBinding: unable to set property" << propertyName << "on object" <<
                       object;
         }
      }
   }
}

void QDeclarativeEngineDebugService::resetBinding(int objectId, const QString &propertyName)
{
   QObject *object = objectForId(objectId);
   QDeclarativeContext *context = qmlContext(object);

   if (object && context) {
      if (object->property(propertyName.toLatin1()).isValid()) {
         QDeclarativeProperty property(object, propertyName);
         QDeclarativeAbstractBinding *oldBinding = QDeclarativePropertyPrivate::binding(property);
         if (oldBinding) {
            QDeclarativeAbstractBinding *oldBinding = QDeclarativePropertyPrivate::setBinding(property, 0);
            if (oldBinding) {
               oldBinding->destroy();
            }
         }
         if (property.isResettable()) {
            // Note: this will reset the property in any case, without regard to states
            // Right now almost no QDeclarativeItem has reset methods for its properties (with the
            // notable exception of QDeclarativeAnchors), so this is not a big issue
            // later on, setBinding does take states into account
            property.reset();
         } else {
            // overwrite with default value
            if (QDeclarativeType *objType = QDeclarativeMetaType::qmlType(object->metaObject())) {
               if (QObject *emptyObject = objType->create()) {
                  if (emptyObject->property(propertyName.toLatin1()).isValid()) {
                     QVariant defaultValue = QDeclarativeProperty(emptyObject, propertyName).read();
                     if (defaultValue.isValid()) {
                        setBinding(objectId, propertyName, defaultValue, true);
                     }
                  }
                  delete emptyObject;
               }
            }
         }
      } else if (hasValidSignal(object, propertyName)) {
         QDeclarativeProperty property(object, propertyName, context);
         QDeclarativePropertyPrivate::setSignalExpression(property, 0);
      } else {
         if (QDeclarativePropertyChanges *propertyChanges = qobject_cast<QDeclarativePropertyChanges *>(object)) {
            propertyChanges->removeProperty(propertyName);
         }
      }
   }
}

void QDeclarativeEngineDebugService::setMethodBody(int objectId, const QString &method, const QString &body)
{
   QObject *object = objectForId(objectId);
   QDeclarativeContext *context = qmlContext(object);
   if (!object || !context || !context->engine()) {
      return;
   }
   QDeclarativeContextData *contextData = QDeclarativeContextData::get(context);
   if (!contextData) {
      return;
   }

   QDeclarativePropertyCache::Data dummy;
   QDeclarativePropertyCache::Data *prop =
      QDeclarativePropertyCache::property(context->engine(), object, method, dummy);

   if (!prop || !(prop->flags & QDeclarativePropertyCache::Data::IsVMEFunction)) {
      return;
   }

   QMetaMethod metaMethod = object->metaObject()->method(prop->coreIndex);
   QList<QByteArray> paramNames = metaMethod.parameterNames();

   QString paramStr;
   for (int ii = 0; ii < paramNames.count(); ++ii) {
      if (ii != 0) {
         paramStr.append(QLatin1String(","));
      }
      paramStr.append(QString::fromUtf8(paramNames.at(ii)));
   }

   QString jsfunction = QLatin1String("(function ") + method + QLatin1String("(") + paramStr +
                        QLatin1String(") {");
   jsfunction += body;
   jsfunction += QLatin1String("\n})");

   QDeclarativeVMEMetaObject *vmeMetaObject =
      static_cast<QDeclarativeVMEMetaObject *>(QObjectPrivate::get(object)->metaObject);
   Q_ASSERT(vmeMetaObject); // the fact we found the property above should guarentee this

   int lineNumber = vmeMetaObject->vmeMethodLineNumber(prop->coreIndex);
   vmeMetaObject->setVmeMethod(prop->coreIndex, QDeclarativeExpressionPrivate::evalInObjectScope(contextData, object,
                               jsfunction, contextData->url.toString(), lineNumber, 0));
}

void QDeclarativeEngineDebugService::propertyChanged(int id, int objectId, const QMetaProperty &property,
      const QVariant &value)
{
   QByteArray reply;
   QDataStream rs(&reply, QIODevice::WriteOnly);

   rs << QByteArray("UPDATE_WATCH") << id << objectId << QByteArray(property.name()) << valueContents(value);

   sendMessage(reply);
}

void QDeclarativeEngineDebugService::addEngine(QDeclarativeEngine *engine)
{
   Q_ASSERT(engine);
   Q_ASSERT(!m_engines.contains(engine));

   m_engines.append(engine);
}

void QDeclarativeEngineDebugService::remEngine(QDeclarativeEngine *engine)
{
   Q_ASSERT(engine);
   Q_ASSERT(m_engines.contains(engine));

   m_engines.removeAll(engine);
}

void QDeclarativeEngineDebugService::objectCreated(QDeclarativeEngine *engine, QObject *object)
{
   Q_ASSERT(engine);
   Q_ASSERT(m_engines.contains(engine));

   int engineId = QDeclarativeDebugService::idForObject(engine);
   int objectId = QDeclarativeDebugService::idForObject(object);
   int parentId = QDeclarativeDebugService::idForObject(object->parent());

   QByteArray reply;
   QDataStream rs(&reply, QIODevice::WriteOnly);

   rs << QByteArray("OBJECT_CREATED") << -1 << engineId << objectId << parentId;
   sendMessage(reply);
}

QT_END_NAMESPACE
