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

#include "qscriptclasspropertyiterator.h"

#include "qscriptstring.h"

class QScriptClassPropertyIteratorPrivate
{
   Q_DECLARE_PUBLIC(QScriptClassPropertyIterator)
 public:
   QScriptClassPropertyIteratorPrivate() {}
   virtual ~QScriptClassPropertyIteratorPrivate() {}

   QScriptValue object;

   QScriptClassPropertyIterator *q_ptr;
};

/*!
  Constructs an iterator for traversing \a object.

  Subclasses should ensure that the iterator is set to the front of the
  sequence of properties (before the first property).
*/
QScriptClassPropertyIterator::QScriptClassPropertyIterator(const QScriptValue &object)
   : d_ptr(new QScriptClassPropertyIteratorPrivate)
{
   d_ptr->q_ptr = this;
   d_ptr->object = object;
}

/*!
  \internal
*/
QScriptClassPropertyIterator::QScriptClassPropertyIterator(const QScriptValue &object,
   QScriptClassPropertyIteratorPrivate &dd)
   : d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   d_ptr->object = object;
}

/*!
  Destroys the iterator.
*/
QScriptClassPropertyIterator::~QScriptClassPropertyIterator()
{
}

/*!
  Returns the Qt Script object this iterator is traversing.
*/
QScriptValue QScriptClassPropertyIterator::object() const
{
   Q_D(const QScriptClassPropertyIterator);
   return d->object;
}

/*!
  \fn bool QScriptClassPropertyIterator::hasNext() const

  Returns true if there is at least one item ahead of the iterator
  (i.e. the iterator is \e not at the back of the property sequence);
  otherwise returns false.

  \sa next(), hasPrevious()
*/

/*!
  \fn void QScriptClassPropertyIterator::next()

  Advances the iterator by one position.

  Calling this function on an iterator located at the back of the
  container leads to undefined results.

  \sa hasNext(), previous(), name()
*/

/*!
  \fn bool QScriptClassPropertyIterator::hasPrevious() const

  Returns true if there is at least one item behind the iterator
  (i.e. the iterator is \e not at the front of the property sequence);
  otherwise returns false.

  \sa previous(), hasNext()
*/

/*!
  \fn void QScriptClassPropertyIterator::previous()

  Moves the iterator back by one position.

  Calling this function on an iterator located at the front of the
  container leads to undefined results.

  \sa hasPrevious(), next(), name()
*/

/*!
  \fn void QScriptClassPropertyIterator::toFront()

  Moves the iterator to the front of the QScriptValue (before the
  first property).

  \sa toBack(), next()
*/

/*!
  \fn void QScriptClassPropertyIterator::toBack()

  Moves the iterator to the back of the QScriptValue (after the
  last property).

  \sa toFront(), previous()
*/

/*!
  \fn QScriptString QScriptClassPropertyIterator::name() const

  Returns the name of the last property that was jumped over using
  next() or previous().

  \sa id()
*/

/*!
  \fn uint QScriptClassPropertyIterator::id() const

  Returns the id of the last property that was jumped over using
  next() or previous().

  The default implementation returns 0.

  \sa name()
*/
uint QScriptClassPropertyIterator::id() const
{
   return 0;
}

/*!
  Returns the flags of the last property that was jumped over using
  next() or previous().

  The default implementation calls the propertyFlags() function of
  object() with argument name().
*/
QScriptValue::PropertyFlags QScriptClassPropertyIterator::flags() const
{
   return object().propertyFlags(name());
}

