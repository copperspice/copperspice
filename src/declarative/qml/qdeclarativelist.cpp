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

#include "qdeclarativelist.h"
#include "private/qdeclarativelist_p.h"
#include "private/qdeclarativeengine_p.h"
#include "private/qdeclarativeproperty_p.h"

QT_BEGIN_NAMESPACE

QDeclarativeListReferencePrivate::QDeclarativeListReferencePrivate()
   : propertyType(-1), refCount(1)
{
}

QDeclarativeListReference QDeclarativeListReferencePrivate::init(const QDeclarativeListProperty<QObject> &prop,
      int propType, QDeclarativeEngine *engine)
{
   QDeclarativeListReference rv;

   if (!prop.object) {
      return rv;
   }

   QDeclarativeEnginePrivate *p = engine ? QDeclarativeEnginePrivate::get(engine) : 0;

   int listType = p ? p->listType(propType) : QDeclarativeMetaType::listType(propType);
   if (listType == -1) {
      return rv;
   }

   rv.d = new QDeclarativeListReferencePrivate;
   rv.d->object = prop.object;
   rv.d->elementType = p ? p->rawMetaObjectForType(listType) : QDeclarativeMetaType::qmlType(listType)->baseMetaObject();
   rv.d->property = prop;
   rv.d->propertyType = propType;

   return rv;
}

void QDeclarativeListReferencePrivate::addref()
{
   Q_ASSERT(refCount > 0);
   ++refCount;
}

void QDeclarativeListReferencePrivate::release()
{
   Q_ASSERT(refCount > 0);
   --refCount;
   if (!refCount) {
      delete this;
   }
}

/*!
\class QDeclarativeListReference
\since 4.7
\module QtDeclarative
\brief The QDeclarativeListReference class allows the manipulation of QDeclarativeListProperty properties.

QDeclarativeListReference allows C++ programs to read from, and assign values to a QML list property in a
simple and type safe way.  A QDeclarativeListReference can be created by passing an object and property
name or through a QDeclarativeProperty instance.  These two are equivalant:

\code
QDeclarativeListReference ref1(object, "children");

QDeclarativeProperty ref2(object, "children");
QDeclarativeListReference ref2 = qvariant_cast<QDeclarativeListReference>(ref2.read());
\endcode

Not all QML list properties support all operations.  A set of methods, canAppend(), canAt(), canClear() and
canCount() allow programs to query whether an operation is supported on a given property.

QML list properties are typesafe.  Only QObject's that derive from the correct base class can be assigned to
the list.  The listElementType() method can be used to query the QMetaObject of the QObject type supported.
Attempting to add objects of the incorrect type to a list property will fail.

Like with normal lists, when accessing a list element by index, it is the callers responsibility to ensure
that it does not request an out of range element using the count() method before calling at().
*/

/*!
Constructs an invalid instance.
*/
QDeclarativeListReference::QDeclarativeListReference()
   : d(0)
{
}

/*!
Constructs a QDeclarativeListReference for \a object's \a property.  If \a property is not a list
property, an invalid QDeclarativeListReference is created.  If \a object is destroyed after
the reference is constructed, it will automatically become invalid.  That is, it is safe to hold
QDeclarativeListReference instances even after \a object is deleted.

Passing \a engine is required to access some QML created list properties.  If in doubt, and an engine
is available, pass it.
*/
QDeclarativeListReference::QDeclarativeListReference(QObject *object, const char *property, QDeclarativeEngine *engine)
   : d(0)
{
   if (!object || !property) {
      return;
   }

   QDeclarativePropertyCache::Data local;
   QDeclarativePropertyCache::Data *data =
      QDeclarativePropertyCache::property(engine, object, QLatin1String(property), local);

   if (!data || !(data->flags & QDeclarativePropertyCache::Data::IsQList)) {
      return;
   }

   QDeclarativeEnginePrivate *p = engine ? QDeclarativeEnginePrivate::get(engine) : 0;

   int listType = p ? p->listType(data->propType) : QDeclarativeMetaType::listType(data->propType);
   if (listType == -1) {
      return;
   }

   d = new QDeclarativeListReferencePrivate;
   d->object = object;
   d->elementType = p ? p->rawMetaObjectForType(listType) : QDeclarativeMetaType::qmlType(listType)->baseMetaObject();
   d->propertyType = data->propType;

   void *args[] = { &d->property, 0 };
   QMetaObject::metacall(object, QMetaObject::ReadProperty, data->coreIndex, args);
}

/*! \internal */
QDeclarativeListReference::QDeclarativeListReference(const QDeclarativeListReference &o)
   : d(o.d)
{
   if (d) {
      d->addref();
   }
}

/*! \internal */
QDeclarativeListReference &QDeclarativeListReference::operator=(const QDeclarativeListReference &o)
{
   if (o.d) {
      o.d->addref();
   }
   if (d) {
      d->release();
   }
   d = o.d;
   return *this;
}

/*! \internal */
QDeclarativeListReference::~QDeclarativeListReference()
{
   if (d) {
      d->release();
   }
}

/*!
Returns true if the instance refers to a valid list property, otherwise false.
*/
bool QDeclarativeListReference::isValid() const
{
   return d && d->object;
}

/*!
Returns the list property's object.  Returns 0 if the reference is invalid.
*/
QObject *QDeclarativeListReference::object() const
{
   if (isValid()) {
      return d->object;
   } else {
      return 0;
   }
}

/*!
Returns the QMetaObject for the elements stored in the list property.  Returns 0 if the reference
is invalid.

The QMetaObject can be used ahead of time to determine whether a given instance can be added
to a list.
*/
const QMetaObject *QDeclarativeListReference::listElementType() const
{
   if (isValid()) {
      return d->elementType;
   } else {
      return 0;
   }
}

/*!
Returns true if the list property can be appended to, otherwise false.  Returns false if the
reference is invalid.

\sa append()
*/
bool QDeclarativeListReference::canAppend() const
{
   return (isValid() && d->property.append);
}

/*!
Returns true if the list property can queried by index, otherwise false.  Returns false if the
reference is invalid.

\sa at()
*/
bool QDeclarativeListReference::canAt() const
{
   return (isValid() && d->property.at);
}

/*!
Returns true if the list property can be cleared, otherwise false.  Returns false if the
reference is invalid.

\sa clear()
*/
bool QDeclarativeListReference::canClear() const
{
   return (isValid() && d->property.clear);
}

/*!
Returns true if the list property can be queried for its element count, otherwise false.
Returns false if the reference is invalid.

\sa count()
*/
bool QDeclarativeListReference::canCount() const
{
   return (isValid() && d->property.count);
}

/*!
Appends \a object to the list.  Returns true if the operation succeeded, otherwise false.

\sa canAppend()
*/
bool QDeclarativeListReference::append(QObject *object) const
{
   if (!canAppend()) {
      return false;
   }

   if (object && !QDeclarativePropertyPrivate::canConvert(object->metaObject(), d->elementType)) {
      return false;
   }

   d->property.append(&d->property, object);

   return true;
}

/*!
Returns the list element at \a index, or 0 if the operation failed.

\sa canAt()
*/
QObject *QDeclarativeListReference::at(int index) const
{
   if (!canAt()) {
      return 0;
   }

   return d->property.at(&d->property, index);
}

/*!
Clears the list.  Returns true if the operation succeeded, otherwise false.

\sa canClear()
*/
bool QDeclarativeListReference::clear() const
{
   if (!canClear()) {
      return false;
   }

   d->property.clear(&d->property);

   return true;
}

/*!
Returns the number of objects in the list, or 0 if the operation failed.
*/
int QDeclarativeListReference::count() const
{
   if (!canCount()) {
      return 0;
   }

   return d->property.count(&d->property);
}

/*!
\class QDeclarativeListProperty
\since 4.7
\brief The QDeclarativeListProperty class allows applications to expose list-like
properties to QML.

QML has many list properties, where more than one object value can be assigned.
The use of a list property from QML looks like this:

\code
FruitBasket {
    fruit: [
        Apple {},
        Orange{},
        Banana{}
    ]
}
\endcode

The QDeclarativeListProperty encapsulates a group of function pointers that represet the
set of actions QML can perform on the list - adding items, retrieving items and
clearing the list.  In the future, additional operations may be supported.  All
list properties must implement the append operation, but the rest are optional.

To provide a list property, a C++ class must implement the operation callbacks,
and then return an appropriate QDeclarativeListProperty value from the property getter.
List properties should have no setter.  In the example above, the Q_PROPERTY()
declarative will look like this:

\code
Q_PROPERTY(QDeclarativeListProperty<Fruit> fruit READ fruit);
\endcode

QML list properties are typesafe - in this case \c {Fruit} is a QObject type that
\c {Apple}, \c {Orange} and \c {Banana} all derive from.

\note QDeclarativeListProperty can only be used for lists of QObject-derived object pointers.

\sa {Object and List Property Types}

*/

/*!
\fn QDeclarativeListProperty::QDeclarativeListProperty()
\internal
*/

/*!
\fn QDeclarativeListProperty::QDeclarativeListProperty(QObject *object, QList<T *> &list)

Convenience constructor for making a QDeclarativeListProperty value from an existing
QList \a list.  The \a list reference must remain valid for as long as \a object
exists.  \a object must be provided.

Generally this constructor should not be used in production code, as a
writable QList violates QML's memory management rules.  However, this constructor
can very useful while prototyping.
*/

/*!
\fn QDeclarativeListProperty::QDeclarativeListProperty(QObject *object, void *data, AppendFunction append,
                                     CountFunction count = 0, AtFunction at = 0,
                                     ClearFunction clear = 0)

Construct a QDeclarativeListProperty from a set of operation functions.  An opaque \a data handle
may be passed which can be accessed from within the operation functions.  The list property
remains valid while \a object exists.

The \a append operation is compulsory and must be provided, while the \a count, \a at and
\a clear methods are optional.
*/

/*!
\typedef QDeclarativeListProperty::AppendFunction

Synonym for \c {void (*)(QDeclarativeListProperty<T> *property, T *value)}.

Append the \a value to the list \a property.
*/

/*!
\typedef QDeclarativeListProperty::CountFunction

Synonym for \c {int (*)(QDeclarativeListProperty<T> *property)}.

Return the number of elements in the list \a property.
*/

/*!
\fn bool QDeclarativeListProperty::operator==(const QDeclarativeListProperty &other) const

Returns true if this QDeclarativeListProperty is equal to \a other, otherwise false.
*/

/*!
\typedef QDeclarativeListProperty::AtFunction

Synonym for \c {T *(*)(QDeclarativeListProperty<T> *property, int index)}.

Return the element at position \a index in the list \a property.
*/

/*!
\typedef QDeclarativeListProperty::ClearFunction

Synonym for \c {void (*)(QDeclarativeListProperty<T> *property)}.

Clear the list \a property.
*/

QT_END_NAMESPACE
