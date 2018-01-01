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

#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qjsonarray.h>
#include <qjsonvalue.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <qdebug.h>

#include <qjsonwriter_p.h>
#include <qjson_p.h>

QT_BEGIN_NAMESPACE

QJsonArray::QJsonArray()
   : d(0), a(0)
{
}


QJsonArray::QJsonArray(QJsonPrivate::Data *data, QJsonPrivate::Array *array)
   : d(data), a(array)
{
   Q_ASSERT(data);
   Q_ASSERT(array);
   d->ref.ref();
}

QJsonArray::~QJsonArray()
{
   if (d && !d->ref.deref()) {
      delete d;
   }
}

QJsonArray::QJsonArray(const QJsonArray &other)
{
   d = other.d;
   a = other.a;
   if (d) {
      d->ref.ref();
   }
}

QJsonArray &QJsonArray::operator =(const QJsonArray &other)
{
   if (d != other.d) {
      if (d && !d->ref.deref()) {
         delete d;
      }
      d = other.d;
      if (d) {
         d->ref.ref();
      }
   }
   a = other.a;

   return *this;
}

QJsonArray QJsonArray::fromStringList(const QStringList &list)
{
   QJsonArray array;
   for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it) {
      array.append(QJsonValue(*it));
   }
   return array;
}

QJsonArray QJsonArray::fromVariantList(const QVariantList &list)
{
   QJsonArray array;
   for (QVariantList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it) {
      array.append(QJsonValue::fromVariant(*it));
   }
   return array;
}

QVariantList QJsonArray::toVariantList() const
{
   QVariantList list;

   if (a) {
      for (int i = 0; i < (int)a->length; ++i) {
         list.append(QJsonValue(d, a, a->at(i)).toVariant());
      }
   }
   return list;
}

int QJsonArray::size() const
{
   if (!d) {
      return 0;
   }

   return (int)a->length;
}

/*!
    \fn QJsonArray::count() const

    Same as size().

    \sa size()
*/

/*!
    Returns \c true if the object is empty. This is the same as size() == 0.

    \sa size()
 */
bool QJsonArray::isEmpty() const
{
   if (!d) {
      return true;
   }

   return !a->length;
}

/*!
    Returns a QJsonValue representing the value for index \a i.

    The returned QJsonValue is \c Undefined, if \a i is out of bounds.

 */
QJsonValue QJsonArray::at(int i) const
{
   if (!a || i < 0 || i >= (int)a->length) {
      return QJsonValue(QJsonValue::Undefined);
   }

   return QJsonValue(d, a, a->at(i));
}

/*!
    Returns the first value stored in the array.

    Same as \c at(0).

    \sa at()
 */
QJsonValue QJsonArray::first() const
{
   return at(0);
}

/*!
    Returns the last value stored in the array.

    Same as \c{at(size() - 1)}.

    \sa at()
 */
QJsonValue QJsonArray::last() const
{
   return at(a ? (a->length - 1) : 0);
}

/*!
    Inserts \a value at the beginning of the array.

    This is the same as \c{insert(0, value)} and will prepend \a value to the array.

    \sa append(), insert()
 */
void QJsonArray::prepend(const QJsonValue &value)
{
   insert(0, value);
}

/*!
    Inserts \a value at the end of the array.

    \sa prepend(), insert()
 */
void QJsonArray::append(const QJsonValue &value)
{
   insert(a ? (int)a->length : 0, value);
}

/*!
    Removes the value at index position \a i. \a i must be a valid
    index position in the array (i.e., \c{0 <= i < size()}).

    \sa insert(), replace()
 */
void QJsonArray::removeAt(int i)
{
   if (!a || i < 0 || i >= (int)a->length) {
      return;
   }

   detach();
   a->removeItems(i, 1);
   ++d->compactionCounter;
   if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(a->length) / 2u) {
      compact();
   }
}

/*! \fn void QJsonArray::removeFirst()

    Removes the first item in the array. Calling this function is
    equivalent to calling \c{removeAt(0)}. The array must not be empty. If
    the array can be empty, call isEmpty() before calling this
    function.

    \sa removeAt(), removeLast()
*/

/*! \fn void QJsonArray::removeLast()

    Removes the last item in the array. Calling this function is
    equivalent to calling \c{removeAt(size() - 1)}. The array must not be
    empty. If the array can be empty, call isEmpty() before calling
    this function.

    \sa removeAt(), removeFirst()
*/

/*!
    Removes the item at index position \a i and returns it. \a i must
    be a valid index position in the array (i.e., \c{0 <= i < size()}).

    If you don't use the return value, removeAt() is more efficient.

    \sa removeAt()
 */
QJsonValue QJsonArray::takeAt(int i)
{
   if (!a || i < 0 || i >= (int)a->length) {
      return QJsonValue(QJsonValue::Undefined);
   }

   QJsonValue v(d, a, a->at(i));
   removeAt(i); // detaches
   return v;
}

/*!
    Inserts \a value at index position \a i in the array. If \a i
    is \c 0, the value is prepended to the array. If \a i is size(), the
    value is appended to the array.

    \sa append(), prepend(), replace(), removeAt()
 */
void QJsonArray::insert(int i, const QJsonValue &value)
{
   Q_ASSERT (i >= 0 && i <= (a ? (int)a->length : 0));
   QJsonValue val = value;

   bool compressed;
   int valueSize = QJsonPrivate::Value::requiredStorage(val, &compressed);

   detach(valueSize + sizeof(QJsonPrivate::Value));

   if (!a->length) {
      a->tableOffset = sizeof(QJsonPrivate::Array);
   }

   int valueOffset = a->reserveSpace(valueSize, i, 1, false);
   if (!valueOffset) {
      return;
   }

   QJsonPrivate::Value &v = (*a)[i];
   v.type = (val.t == QJsonValue::Undefined ? QJsonValue::Null : val.t);
   v.latinOrIntValue = compressed;
   v.latinKey = false;
   v.value = QJsonPrivate::Value::valueToStore(val, valueOffset);
   if (valueSize) {
      QJsonPrivate::Value::copyData(val, (char *)a + valueOffset, compressed);
   }
}

/*!
    \fn QJsonArray::iterator QJsonArray::insert(iterator before, const QJsonValue &value)

    Inserts \a value before the position pointed to by \a before, and returns an iterator
    pointing to the newly inserted item.

    \sa erase(), insert()
*/

/*!
    \fn QJsonArray::iterator QJsonArray::erase(iterator it)

    Removes the item pointed to by \a it, and returns an iterator pointing to the
    next item.

    \sa removeAt()
*/

/*!
    Replaces the item at index position \a i with \a value. \a i must
    be a valid index position in the array (i.e., \c{0 <= i < size()}).

    \sa operator[](), removeAt()
 */
void QJsonArray::replace(int i, const QJsonValue &value)
{
   Q_ASSERT (a && i >= 0 && i < (int)(a->length));
   QJsonValue val = value;

   bool compressed;
   int valueSize = QJsonPrivate::Value::requiredStorage(val, &compressed);

   detach(valueSize);

   if (!a->length) {
      a->tableOffset = sizeof(QJsonPrivate::Array);
   }

   int valueOffset = a->reserveSpace(valueSize, i, 1, true);
   if (!valueOffset) {
      return;
   }

   QJsonPrivate::Value &v = (*a)[i];
   v.type = (val.t == QJsonValue::Undefined ? QJsonValue::Null : val.t);
   v.latinOrIntValue = compressed;
   v.latinKey = false;
   v.value = QJsonPrivate::Value::valueToStore(val, valueOffset);
   if (valueSize) {
      QJsonPrivate::Value::copyData(val, (char *)a + valueOffset, compressed);
   }

   ++d->compactionCounter;
   if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(a->length) / 2u) {
      compact();
   }
}

/*!
    Returns \c true if the array contains an occurrence of \a value, otherwise \c false.

    \sa count()
 */
bool QJsonArray::contains(const QJsonValue &value) const
{
   for (int i = 0; i < size(); i++) {
      if (at(i) == value) {
         return true;
      }
   }
   return false;
}

/*!
    Returns the value at index position \a i as a modifiable reference.
    \a i must be a valid index position in the array (i.e., \c{0 <= i <
    size()}).

    The return value is of type QJsonValueRef, a helper class for QJsonArray
    and QJsonObject. When you get an object of type QJsonValueRef, you can
    use it as if it were a reference to a QJsonValue. If you assign to it,
    the assignment will apply to the character in the QJsonArray of QJsonObject
    from which you got the reference.

    \sa at()
 */
QJsonValueRef QJsonArray::operator [](int i)
{
   Q_ASSERT(a && i >= 0 && i < (int)a->length);
   return QJsonValueRef(this, i);
}

/*!
    \overload

    Same as at().
 */
QJsonValue QJsonArray::operator[](int i) const
{
   return at(i);
}

/*!
    Returns \c true if this array is equal to \a other.
 */
bool QJsonArray::operator==(const QJsonArray &other) const
{
   if (a == other.a) {
      return true;
   }

   if (!a) {
      return !other.a->length;
   }
   if (!other.a) {
      return !a->length;
   }
   if (a->length != other.a->length) {
      return false;
   }

   for (int i = 0; i < (int)a->length; ++i) {
      if (QJsonValue(d, a, a->at(i)) != QJsonValue(other.d, other.a, other.a->at(i))) {
         return false;
      }
   }
   return true;
}

/*!
    Returns \c true if this array is not equal to \a other.
 */
bool QJsonArray::operator!=(const QJsonArray &other) const
{
   return !(*this == other);
}

void QJsonArray::detach(uint reserve)
{
   if (!d) {
      d = new QJsonPrivate::Data(reserve, QJsonValue::Array);
      a = static_cast<QJsonPrivate::Array *>(d->header->root());
      d->ref.ref();
      return;
   }
   if (reserve == 0 && d->ref.load() == 1) {
      return;
   }

   QJsonPrivate::Data *x = d->clone(a, reserve);
   x->ref.ref();
   if (!d->ref.deref()) {
      delete d;
   }
   d = x;
   a = static_cast<QJsonPrivate::Array *>(d->header->root());
}

/*!
    \internal
 */
void QJsonArray::compact()
{
   if (!d || !d->compactionCounter) {
      return;
   }

   detach();
   d->compact();
   a = static_cast<QJsonPrivate::Array *>(d->header->root());
}


QDebug operator<<(QDebug dbg, const QJsonArray &a)
{
   if (!a.a) {
      dbg << "QJsonArray()";
      return dbg;
   }
   QByteArray json;
   QJsonPrivate::Writer::arrayToJson(a.a, json, 0, true);
   dbg.nospace() << "QJsonArray("
                 << json.constData() // print as utf-8 string without extra quotation marks
                 << ")";
   return dbg.space();
}

QT_END_NAMESPACE

