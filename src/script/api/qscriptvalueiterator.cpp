/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
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

QT_BEGIN_NAMESPACE

/*!
  \since 4.3
  \class QScriptValueIterator

  \brief The QScriptValueIterator class provides a Java-style iterator for QScriptValue.

  \ingroup script


  The QScriptValueIterator constructor takes a QScriptValue as
  argument.  After construction, the iterator is located at the very
  beginning of the sequence of properties. Here's how to iterate over
  all the properties of a QScriptValue:

  \snippet doc/src/snippets/code/src_script_qscriptvalueiterator.cpp 0

  The next() advances the iterator. The name(), value() and flags()
  functions return the name, value and flags of the last item that was
  jumped over.

  If you want to remove properties as you iterate over the
  QScriptValue, use remove(). If you want to modify the value of a
  property, use setValue().

  Note that QScriptValueIterator only iterates over the QScriptValue's
  own properties; i.e. it does not follow the prototype chain. You can
  use a loop like this to follow the prototype chain:

  \snippet doc/src/snippets/code/src_script_qscriptvalueiterator.cpp 1

  Note that QScriptValueIterator will not automatically skip over
  properties that have the QScriptValue::SkipInEnumeration flag set;
  that flag only affects iteration in script code.  If you want, you
  can skip over such properties with code like the following:

  \snippet doc/src/snippets/code/src_script_qscriptvalueiterator.cpp 2

  \sa QScriptValue::property()
*/

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

/*!
  Constructs an iterator for traversing \a object. The iterator is
  set to be at the front of the sequence of properties (before the
  first property).
*/
QScriptValueIterator::QScriptValueIterator(const QScriptValue &object)
   : d_ptr(0)
{
   if (object.isObject()) {
      d_ptr.reset(new QScriptValueIteratorPrivate());
      d_ptr->objectValue = object;
   }
}

/*!
  Destroys the iterator.
*/
QScriptValueIterator::~QScriptValueIterator()
{
}

/*!
  Returns true if there is at least one item ahead of the iterator
  (i.e. the iterator is \e not at the back of the property sequence);
  otherwise returns false.

  \sa next(), hasPrevious()
*/
bool QScriptValueIterator::hasNext() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->engine()) {
      return false;
   }

   const_cast<QScriptValueIteratorPrivate *>(d)->ensureInitialized();
   return d->it != d->propertyNames.end();
}

/*!
  Advances the iterator by one position.

  Calling this function on an iterator located at the back of the
  container leads to undefined results.

  \sa hasNext(), previous(), name()
*/
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

/*!
  Returns true if there is at least one item behind the iterator
  (i.e. the iterator is \e not at the front of the property sequence);
  otherwise returns false.

  \sa previous(), hasNext()
*/
bool QScriptValueIterator::hasPrevious() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->engine()) {
      return false;
   }

   const_cast<QScriptValueIteratorPrivate *>(d)->ensureInitialized();
   return d->it != d->propertyNames.begin();
}

/*!
  Moves the iterator back by one position.

  Calling this function on an iterator located at the front of the
  container leads to undefined results.

  \sa hasPrevious(), next(), name()
*/
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

/*!
  Moves the iterator to the front of the QScriptValue (before the
  first property).

  \sa toBack(), next()
*/
void QScriptValueIterator::toFront()
{
   Q_D(QScriptValueIterator);
   if (!d) {
      return;
   }
   d->ensureInitialized();
   d->it = d->propertyNames.begin();
}

/*!
  Moves the iterator to the back of the QScriptValue (after the
  last property).

  \sa toFront(), previous()
*/
void QScriptValueIterator::toBack()
{
   Q_D(QScriptValueIterator);
   if (!d) {
      return;
   }
   d->ensureInitialized();
   d->it = d->propertyNames.end();
}

/*!
  Returns the name of the last property that was jumped over using
  next() or previous().

  \sa value(), flags()
*/
QString QScriptValueIterator::name() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->initialized || !d->engine()) {
      return QString();
   }
   return d->current->ustring();
}

/*!
  \since 4.4

  Returns the name of the last property that was jumped over using
  next() or previous().
*/
QScriptString QScriptValueIterator::scriptName() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->initialized || !d->engine()) {
      return QScriptString();
   }
   return d->engine()->toStringHandle(*d->current);
}

/*!
  Returns the value of the last property that was jumped over using
  next() or previous().

  \sa setValue(), name()
*/
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

/*!
  Sets the \a value of the last property that was jumped over using
  next() or previous().

  \sa value(), name()
*/
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

/*!
  Returns the flags of the last property that was jumped over using
  next() or previous().

  \sa value()
*/
QScriptValue::PropertyFlags QScriptValueIterator::flags() const
{
   Q_D(const QScriptValueIterator);
   if (!d || !d->initialized || !d->engine()) {
      return 0;
   }
   QScript::APIShim shim(d->engine());
   return d->object()->propertyFlags(*d->current);
}

/*!
  Removes the last property that was jumped over using next()
  or previous().

  \sa setValue()
*/
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

/*!
  Makes the iterator operate on \a object. The iterator is set to be
  at the front of the sequence of properties (before the first
  property).
*/
QScriptValueIterator &QScriptValueIterator::operator=(QScriptValue &object)
{
   d_ptr.reset();
   if (object.isObject()) {
      d_ptr.reset(new QScriptValueIteratorPrivate());
      d_ptr->objectValue = object;
   }
   return *this;
}

QT_END_NAMESPACE
