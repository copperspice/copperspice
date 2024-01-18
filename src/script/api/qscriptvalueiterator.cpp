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

#include "config.h"
#include "qscriptvalueiterator.h"

#include "qscriptstring.h"
#include "qscriptengine.h"
#include "qscriptengine_p.h"
#include "qscriptvalue_p.h"
#include "qlinkedlist.h"


#include "JSObject.h"
#include "PropertyNameArray.h"
#include "JSArray.h"
#include "JSFunction.h"

class QScriptValueIteratorPrivate
{
 public:
   QScriptValueIteratorPrivate()
      : initialized(false) {
   }

   ~QScriptValueIteratorPrivate() {
      if (!initialized) {
         return;
      }
      QScriptEnginePrivate *eng_p = engine();
      if (!eng_p) {
         return;
      }
      QScript::APIShim shim(eng_p);
      propertyNames.clear(); //destroying the identifiers need to be done under the APIShim guard
   }

   QScriptValuePrivate *object() const {
      return QScriptValuePrivate::get(objectValue);
   }

   QScriptEnginePrivate *engine() const {
      return QScriptEnginePrivate::get(objectValue.engine());
   }

   void ensureInitialized() {
      if (initialized) {
         return;
      }
      QScriptEnginePrivate *eng_p = engine();
      QScript::APIShim shim(eng_p);
      JSC::ExecState *exec = eng_p->globalExec();
      JSC::PropertyNameArray propertyNamesArray(exec);
      JSC::asObject(object()->jscValue)->getOwnPropertyNames(exec, propertyNamesArray, JSC::IncludeDontEnumProperties);

      JSC::PropertyNameArray::const_iterator propertyNamesIt = propertyNamesArray.begin();
      for (; propertyNamesIt != propertyNamesArray.end(); ++propertyNamesIt) {
         propertyNames.append(*propertyNamesIt);
      }
      it = propertyNames.begin();
      initialized = true;
   }

   QScriptValue objectValue;
   QLinkedList<JSC::Identifier> propertyNames;
   QLinkedList<JSC::Identifier>::iterator it;
   QLinkedList<JSC::Identifier>::iterator current;
   bool initialized;
};

QScriptValueIterator::QScriptValueIterator(const QScriptValue &object)
   : d_ptr(nullptr)
{
   if (object.isObject()) {
      d_ptr.reset(new QScriptValueIteratorPrivate());
      d_ptr->objectValue = object;
   }
}

QScriptValueIterator::~QScriptValueIterator()
{
}

bool QScriptValueIterator::hasNext() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->engine()) {
      return false;
   }

   const_cast<QScriptValueIteratorPrivate *>(d)->ensureInitialized();
   return d->it != d->propertyNames.end();
}

void QScriptValueIterator::next()
{
   Q_D(QScriptValueIterator);
   if (!d) {
      return;
   }
   d->ensureInitialized();

   d->current = d->it;
   ++(d->it);
}

bool QScriptValueIterator::hasPrevious() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->engine()) {
      return false;
   }

   const_cast<QScriptValueIteratorPrivate *>(d)->ensureInitialized();
   return d->it != d->propertyNames.begin();
}

void QScriptValueIterator::previous()
{
   Q_D(QScriptValueIterator);
   if (!d) {
      return;
   }
   d->ensureInitialized();
   --(d->it);
   d->current = d->it;
}

void QScriptValueIterator::toFront()
{
   Q_D(QScriptValueIterator);
   if (!d) {
      return;
   }
   d->ensureInitialized();
   d->it = d->propertyNames.begin();
}

void QScriptValueIterator::toBack()
{
   Q_D(QScriptValueIterator);
   if (!d) {
      return;
   }
   d->ensureInitialized();
   d->it = d->propertyNames.end();
}

QString QScriptValueIterator::name() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->initialized || !d->engine()) {
      return QString();
   }
   return d->current->ustring();
}

QScriptString QScriptValueIterator::scriptName() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->initialized || !d->engine()) {
      return QScriptString();
   }
   return d->engine()->toStringHandle(*d->current);
}

QScriptValue QScriptValueIterator::value() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->initialized || !d->engine()) {
      return QScriptValue();
   }
   QScript::APIShim shim(d->engine());
   JSC::JSValue jsValue = d->object()->property(*d->current);
   return d->engine()->scriptValueFromJSCValue(jsValue);
}

void QScriptValueIterator::setValue(const QScriptValue &value)
{
   Q_D(QScriptValueIterator);
   if (!d || !d->initialized || !d->engine()) {
      return;
   }
   QScript::APIShim shim(d->engine());
   JSC::JSValue jsValue = d->engine()->scriptValueToJSCValue(value);
   d->object()->setProperty(*d->current, jsValue);
}

QScriptValue::PropertyFlags QScriptValueIterator::flags() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->initialized || !d->engine()) {
      return Qt::EmptyFlag;
   }
   QScript::APIShim shim(d->engine());
   return d->object()->propertyFlags(*d->current);
}

void QScriptValueIterator::remove()
{
   Q_D(QScriptValueIterator);
   if (!d || !d->initialized || !d->engine()) {
      return;
   }
   QScript::APIShim shim(d->engine());
   d->object()->setProperty(*d->current, JSC::JSValue());
   d->propertyNames.erase(d->current);
}

QScriptValueIterator &QScriptValueIterator::operator=(QScriptValue &object)
{
   d_ptr.reset();
   if (object.isObject()) {
      d_ptr.reset(new QScriptValueIteratorPrivate());
      d_ptr->objectValue = object;
   }
   return *this;
}
