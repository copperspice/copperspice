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

#include <qscriptdeclarativeclass_p.h>
#include <qscriptdeclarativeobject_p.h>
#include <qscriptobject_p.h>
#include <qscriptstaticscopeobject_p.h>
#include <qscriptstring.h>
#include <qscriptengine.h>
#include <qscriptengineagent.h>
#include <qscriptengine_p.h>
#include <qscriptvalue_p.h>
#include <qscriptqobject_p.h>
#include <qscriptactivationobject_p.h>
#include <qstringlist.h>

QScriptDeclarativeClass::Value::Value()
{
   new (this) JSC::JSValue(JSC::jsUndefined());
}

QScriptDeclarativeClass::Value::Value(const Value &other)
{
   new (this) JSC::JSValue((JSC::JSValue &)other);
}

static QScriptDeclarativeClass::Value jscToValue(const JSC::JSValue &val)
{
   return QScriptDeclarativeClass::Value((QScriptDeclarativeClass::Value &)val);
}

QScriptDeclarativeClass::Value::Value(QScriptContext *ctxt, int value)
{
   new (this) JSC::JSValue(QScriptEnginePrivate::frameForContext(ctxt), value);
}

QScriptDeclarativeClass::Value::Value(QScriptContext *ctxt, uint value)
{
   new (this) JSC::JSValue(QScriptEnginePrivate::frameForContext(ctxt), value);
}

QScriptDeclarativeClass::Value::Value(QScriptContext *, bool value)
{
   if (value) {
      new (this) JSC::JSValue(JSC::JSValue::JSTrue);
   } else {
      new (this) JSC::JSValue(JSC::JSValue::JSFalse);
   }
}

QScriptDeclarativeClass::Value::Value(QScriptContext *ctxt, double value)
{
   new (this) JSC::JSValue(QScriptEnginePrivate::frameForContext(ctxt), value);
}

QScriptDeclarativeClass::Value::Value(QScriptContext *ctxt, float value)
{
   new (this) JSC::JSValue(QScriptEnginePrivate::frameForContext(ctxt), value);
}

QScriptDeclarativeClass::Value::Value(QScriptContext *ctxt, const QString &value)
{
   new (this) JSC::JSValue(JSC::jsString(QScriptEnginePrivate::frameForContext(ctxt), value));
}

QScriptDeclarativeClass::Value::Value(QScriptContext *ctxt, const QScriptValue &value)
{
   new (this) JSC::JSValue(QScriptEnginePrivate::get(ctxt->engine())->scriptValueToJSCValue(value));
}

QScriptDeclarativeClass::Value::Value(QScriptEngine *eng, int value)
{
   new (this) JSC::JSValue(QScriptEnginePrivate::get(eng)->currentFrame, value);
}

QScriptDeclarativeClass::Value::Value(QScriptEngine *eng, uint value)
{
   new (this) JSC::JSValue(QScriptEnginePrivate::get(eng)->currentFrame, value);
}

QScriptDeclarativeClass::Value::Value(QScriptEngine *eng, bool value)
{
   (void) eng;

   if (value) {
      new (this) JSC::JSValue(JSC::JSValue::JSTrue);
   } else {
      new (this) JSC::JSValue(JSC::JSValue::JSFalse);
   }
}

QScriptDeclarativeClass::Value::Value(QScriptEngine *eng, double value)
{
   new (this) JSC::JSValue(QScriptEnginePrivate::get(eng)->currentFrame, value);
}

QScriptDeclarativeClass::Value::Value(QScriptEngine *eng, float value)
{
   new (this) JSC::JSValue(QScriptEnginePrivate::get(eng)->currentFrame, value);
}

QScriptDeclarativeClass::Value::Value(QScriptEngine *eng, const QString &value)
{
   new (this) JSC::JSValue(JSC::jsString(QScriptEnginePrivate::get(eng)->currentFrame, value));
}

QScriptDeclarativeClass::Value::Value(QScriptEngine *eng, const QScriptValue &value)
{
   new (this) JSC::JSValue(QScriptEnginePrivate::get(eng)->scriptValueToJSCValue(value));
}

QScriptDeclarativeClass::Value::~Value()
{
   ((JSC::JSValue *)(this))->~JSValue();
}

QScriptValue QScriptDeclarativeClass::Value::toScriptValue(QScriptEngine *engine) const
{
   return QScriptEnginePrivate::get(engine)->scriptValueFromJSCValue((JSC::JSValue &) * this);
}

QScriptDeclarativeClass::PersistentIdentifier::PersistentIdentifier()
   : identifier(nullptr), engine(nullptr)
{
   new (&d) JSC::Identifier();
}

QScriptDeclarativeClass::PersistentIdentifier::~PersistentIdentifier()
{
   if (engine) {
      QScript::APIShim shim(engine);
      ((JSC::Identifier &)d).JSC::Identifier::~Identifier();
   } else {
      ((JSC::Identifier &)d).JSC::Identifier::~Identifier();
   }
}

QScriptDeclarativeClass::PersistentIdentifier::PersistentIdentifier(const PersistentIdentifier &other)
{
   identifier = other.identifier;
   engine = other.engine;
   new (&d) JSC::Identifier((JSC::Identifier &)(other.d));
}

QScriptDeclarativeClass::PersistentIdentifier &QScriptDeclarativeClass::PersistentIdentifier::operator=
(const PersistentIdentifier &other)
{
   identifier = other.identifier;
   engine = other.engine;
   ((JSC::Identifier &)d) = (JSC::Identifier &)(other.d);
   return *this;
}

QString QScriptDeclarativeClass::PersistentIdentifier::toString() const
{
   return ((JSC::Identifier &)d).ustring();
}

QScriptDeclarativeClass::QScriptDeclarativeClass(QScriptEngine *engine)
   : d_ptr(new QScriptDeclarativeClassPrivate)
{
   Q_ASSERT(sizeof(void *) == sizeof(JSC::Identifier));
   d_ptr->q_ptr = this;
   d_ptr->engine = engine;
}

QScriptValue QScriptDeclarativeClass::newObject(QScriptEngine *engine,
   QScriptDeclarativeClass *scriptClass, Object *object)
{
   Q_ASSERT(engine);
   Q_ASSERT(scriptClass);

   QScriptEnginePrivate *p = QScriptEnginePrivate::cs_getPrivate(engine);

   QScript::APIShim shim(p);

   JSC::ExecState *exec = p->currentFrame;
   QScriptObject *result = new (exec) QScriptObject(p->scriptObjectStructure);
   result->setDelegate(new QScript::DeclarativeObjectDelegate(scriptClass, object));
   return p->scriptValueFromJSCValue(result);
}

QScriptDeclarativeClass::Value QScriptDeclarativeClass::newObjectValue(QScriptEngine *engine, QScriptDeclarativeClass *scriptClass,
   Object *object)
{
   Q_ASSERT(engine);
   Q_ASSERT(scriptClass);

   QScriptEnginePrivate *p = QScriptEnginePrivate::cs_getPrivate(engine);
   QScript::APIShim shim(p);

   JSC::ExecState *exec = p->currentFrame;
   QScriptObject *result = new (exec) QScriptObject(p->scriptObjectStructure);
   result->setDelegate(new QScript::DeclarativeObjectDelegate(scriptClass, object));
   return jscToValue(JSC::JSValue(result));
}

QScriptDeclarativeClass *QScriptDeclarativeClass::scriptClass(const QScriptValue &v)
{
   QScriptValuePrivate *d = QScriptValuePrivate::get(v);
   if (! d || ! d->isJSC()) {
      return nullptr;
   }

   return QScriptEnginePrivate::declarativeClass(d->jscValue);
}

QScriptDeclarativeClass::Object *QScriptDeclarativeClass::object(const QScriptValue &v)
{
   QScriptValuePrivate *d = QScriptValuePrivate::get(v);
   if (! d || ! d->isJSC()) {
      return nullptr;
   }

   return QScriptEnginePrivate::declarativeObject(d->jscValue);
}

QScriptValue QScriptDeclarativeClass::function(const QScriptValue &v, const Identifier &name)
{
   QScriptValuePrivate *d = QScriptValuePrivate::get(v);

   if (! d->isObject()) {
      return QScriptValue();
   }

   QScript::APIShim shim(d->engine);
   JSC::ExecState *exec = d->engine->currentFrame;
   JSC::JSObject *object = d->jscValue.getObject();
   JSC::PropertySlot slot(const_cast<JSC::JSObject *>(object));
   JSC::JSValue result;

   JSC::Identifier id(exec, (JSC::UString::Rep *)name);

   if (const_cast<JSC::JSObject *>(object)->getOwnPropertySlot(exec, id, slot)) {
      result = slot.getValue(exec, id);
      if (QScript::isFunction(result)) {
         return d->engine->scriptValueFromJSCValue(result);
      }
   }

   return QScriptValue();
}

QScriptValue QScriptDeclarativeClass::property(const QScriptValue &v, const Identifier &name)
{
   QScriptValuePrivate *d = QScriptValuePrivate::get(v);

   if (!d->isObject()) {
      return QScriptValue();
   }

   QScript::APIShim shim(d->engine);
   JSC::ExecState *exec = d->engine->currentFrame;
   JSC::JSObject *object = d->jscValue.getObject();
   JSC::PropertySlot slot(const_cast<JSC::JSObject *>(object));
   JSC::JSValue result;

   JSC::Identifier id(exec, (JSC::UString::Rep *)name);

   if (const_cast<JSC::JSObject *>(object)->getOwnPropertySlot(exec, id, slot)) {
      result = slot.getValue(exec, id);
      return d->engine->scriptValueFromJSCValue(result);
   }

   return QScriptValue();
}

QScriptDeclarativeClass::Value QScriptDeclarativeClass::functionValue(const QScriptValue &v, const Identifier &name)
{
   QScriptValuePrivate *d = QScriptValuePrivate::get(v);

   if (!d->isObject()) {
      return Value();
   }

   QScript::APIShim shim(d->engine);
   JSC::ExecState *exec = d->engine->currentFrame;
   JSC::JSObject *object = d->jscValue.getObject();
   JSC::PropertySlot slot(const_cast<JSC::JSObject *>(object));
   JSC::JSValue result;

   JSC::Identifier id(exec, (JSC::UString::Rep *)name);

   if (const_cast<JSC::JSObject *>(object)->getOwnPropertySlot(exec, id, slot)) {
      result = slot.getValue(exec, id);
      if (QScript::isFunction(result)) {
         return jscToValue(result);
      }
   }

   return Value();
}

QScriptDeclarativeClass::Value QScriptDeclarativeClass::propertyValue(const QScriptValue &v, const Identifier &name)
{
   QScriptValuePrivate *d = QScriptValuePrivate::get(v);

   if (!d->isObject()) {
      return Value();
   }

   QScript::APIShim shim(d->engine);
   JSC::ExecState *exec = d->engine->currentFrame;
   JSC::JSObject *object = d->jscValue.getObject();
   JSC::PropertySlot slot(const_cast<JSC::JSObject *>(object));
   JSC::JSValue result;

   JSC::Identifier id(exec, (JSC::UString::Rep *)name);

   if (const_cast<JSC::JSObject *>(object)->getOwnPropertySlot(exec, id, slot)) {
      result = slot.getValue(exec, id);
      return jscToValue(result);
   }

   return Value();
}

/*
Returns the scope chain entry at \a index.  If index is less than 0, returns
entries starting at the end.  For example, scopeChainValue(context, -1) will return
the value last in the scope chain.
*/
QScriptValue QScriptDeclarativeClass::scopeChainValue(QScriptContext *context, int index)
{
   context->activationObject(); //ensure the creation of the normal scope for native context
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(context);
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);

   JSC::ScopeChainNode *node = frame->scopeChain();
   JSC::ScopeChainIterator it(node);

   if (index < 0) {
      int count = 0;
      for (it = node->begin(); it != node->end(); ++it) {
         ++count;
      }

      index = qAbs(index);
      if (index > count) {
         return QScriptValue();
      } else {
         index = count - index;
      }
   }

   for (it = node->begin(); it != node->end(); ++it) {

      if (index == 0) {

         JSC::JSObject *object = *it;
         if (! object) {
            return QScriptValue();
         }

         if (object->inherits(&QScript::QScriptActivationObject::info)
            && (static_cast<QScript::QScriptActivationObject *>(object)->delegate() != nullptr)) {
            // Return the object that property access is being delegated to
            object = static_cast<QScript::QScriptActivationObject *>(object)->delegate();
         }
         return engine->scriptValueFromJSCValue(object);

      } else {
         --index;
      }

   }

   return QScriptValue();
}


QScriptContext *QScriptDeclarativeClass::pushCleanContext(QScriptEngine *engine)
{
   if (! engine) {
      return nullptr;
   }

   return engine->pushContext();
}

QScriptDeclarativeClass::~QScriptDeclarativeClass()
{
}

QScriptEngine *QScriptDeclarativeClass::engine() const
{
   return d_ptr->engine;
}

bool QScriptDeclarativeClass::supportsCall() const
{
   return d_ptr->supportsCall;
}

void QScriptDeclarativeClass::setSupportsCall(bool c)
{
   d_ptr->supportsCall = c;
}

QScriptDeclarativeClass::PersistentIdentifier
QScriptDeclarativeClass::createPersistentIdentifier(const QString &str)
{
   QScriptEnginePrivate *p = QScriptEnginePrivate::cs_getPrivate(d_ptr->engine);

   QScript::APIShim shim(p);
   JSC::ExecState *exec = p->currentFrame;

   PersistentIdentifier rv(p);
   new (&rv.d) JSC::Identifier(exec, (UChar *)str.constData(), str.size());
   rv.identifier = (void *)((JSC::Identifier &)rv.d).ustring().rep();

   return rv;
}

QScriptDeclarativeClass::PersistentIdentifier
QScriptDeclarativeClass::createPersistentIdentifier(const Identifier &id)
{
   QScriptEnginePrivate *p = QScriptEnginePrivate::cs_getPrivate(d_ptr->engine);

   QScript::APIShim shim(p);
   JSC::ExecState *exec = p->currentFrame;

   PersistentIdentifier rv(p);
   new (&rv.d) JSC::Identifier(exec, (JSC::UString::Rep *)id);
   rv.identifier = (void *)((JSC::Identifier &)rv.d).ustring().rep();
   return rv;
}

QString QScriptDeclarativeClass::toString(const Identifier &identifier)
{
   JSC::UString::Rep *r = (JSC::UString::Rep *)identifier;
   return QString((QChar *)r->data(), r->size());
}

bool QScriptDeclarativeClass::startsWithUpper(const Identifier &identifier)
{
   JSC::UString::Rep *r = (JSC::UString::Rep *)identifier;

   if (r->size() < 1) {
      return false;
   }

   return QChar(char32_t(r->data()[0])).category() == QChar::Letter_Uppercase;
}

quint32 QScriptDeclarativeClass::toArrayIndex(const Identifier &identifier, bool *ok)
{
   JSC::UString::Rep *r = (JSC::UString::Rep *)identifier;
   JSC::UString s(r);
   return s.toArrayIndex(ok);
}

QScriptClass::QueryFlags QScriptDeclarativeClass::queryProperty(Object *object,
   const Identifier &name, QScriptClass::QueryFlags flags)
{
   Q_UNUSED(object);
   Q_UNUSED(name);
   Q_UNUSED(flags);
   return Qt::EmptyFlag;
}

QScriptDeclarativeClass::Value QScriptDeclarativeClass::property(Object *object, const Identifier &name)
{
   Q_UNUSED(object);
   Q_UNUSED(name);
   return Value();
}

void QScriptDeclarativeClass::setProperty(Object *object, const Identifier &name, const QScriptValue &value)
{
   Q_UNUSED(object);
   Q_UNUSED(name);
   Q_UNUSED(value);
}

QScriptValue::PropertyFlags QScriptDeclarativeClass::propertyFlags(Object *object, const Identifier &name)
{
   Q_UNUSED(object);
   Q_UNUSED(name);
   return Qt::EmptyFlag;
}

QScriptDeclarativeClass::Value QScriptDeclarativeClass::call(Object *object, QScriptContext *ctxt)
{
   Q_UNUSED(object);
   Q_UNUSED(ctxt);
   return Value();
}

bool QScriptDeclarativeClass::compare(Object *o, Object *o2)
{
   return o == o2;
}

QStringList QScriptDeclarativeClass::propertyNames(Object *object)
{
   Q_UNUSED(object);
   return QStringList();
}

bool QScriptDeclarativeClass::isQObject() const
{
   return false;
}

QObject *QScriptDeclarativeClass::toQObject(Object *, bool *ok)
{
   if (ok) {
      *ok = false;
   }

   return nullptr;
}

QVariant QScriptDeclarativeClass::toVariant(Object *, bool *ok)
{
   if (ok) {
      *ok = false;
   }

   return QVariant();
}

QScriptContext *QScriptDeclarativeClass::context() const
{
   return d_ptr->context;
}

/*!
  Creates a scope object with a fixed set of undeletable properties.
*/
QScriptValue QScriptDeclarativeClass::newStaticScopeObject(
   QScriptEngine *engine, int propertyCount, const QString *names,
   const QScriptValue *values, const QScriptValue::PropertyFlags *flags)
{
   QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
   QScript::APIShim shim(eng_p);
   JSC::ExecState *exec = eng_p->currentFrame;
   QScriptStaticScopeObject::PropertyInfo *props = new QScriptStaticScopeObject::PropertyInfo[propertyCount];
   for (int i = 0; i < propertyCount; ++i) {
      unsigned attribs = QScriptEnginePrivate::propertyFlagsToJSCAttributes(flags[i]);
      Q_ASSERT_X(attribs & JSC::DontDelete, Q_FUNC_INFO, "All properties must be undeletable");
      JSC::Identifier id = JSC::Identifier(exec, names[i]);
      JSC::JSValue jsval = eng_p->scriptValueToJSCValue(values[i]);
      props[i] = QScriptStaticScopeObject::PropertyInfo(id, jsval, attribs);
   }
   QScriptValue result = eng_p->scriptValueFromJSCValue(new (exec)QScriptStaticScopeObject(
            eng_p->staticScopeObjectStructure,
            propertyCount, props));
   delete[] props;
   return result;
}

/*!
  Creates a static scope object that's initially empty, but to which new
  properties can be added.
*/
QScriptValue QScriptDeclarativeClass::newStaticScopeObject(QScriptEngine *engine)
{
   QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
   QScript::APIShim shim(eng_p);
   return eng_p->scriptValueFromJSCValue(new (eng_p->currentFrame)QScriptStaticScopeObject(
            eng_p->staticScopeObjectStructure));
}

