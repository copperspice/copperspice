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
#include <qstringlist.h>
#include <qdebug.h>
#include <qvariant.h>
#include <qjson_p.h>
#include <qjsonwriter_p.h>

QT_BEGIN_NAMESPACE

QJsonObject::QJsonObject()
   : d(0), o(0)
{
}

/*!
    \internal
 */
QJsonObject::QJsonObject(QJsonPrivate::Data *data, QJsonPrivate::Object *object)
   : d(data), o(object)
{
   Q_ASSERT(d);
   Q_ASSERT(o);
   d->ref.ref();
}


/*!
    Destroys the object.
 */
QJsonObject::~QJsonObject()
{
   if (d && !d->ref.deref()) {
      delete d;
   }
}

/*!
    Creates a copy of \a other.

    Since QJsonObject is implicitly shared, the copy is shallow
    as long as the object does not get modified.
 */
QJsonObject::QJsonObject(const QJsonObject &other)
{
   d = other.d;
   o = other.o;
   if (d) {
      d->ref.ref();
   }
}

/*!
    Assigns \a other to this object.
 */
QJsonObject &QJsonObject::operator =(const QJsonObject &other)
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
   o = other.o;

   return *this;
}

/*!
    Converts the variant map \a map to a QJsonObject.

    The keys in \a map will be used as the keys in the JSON object,
    and the QVariant values will be converted to JSON values.

    \sa toVariantMap(), QJsonValue::fromVariant()
 */
QJsonObject QJsonObject::fromVariantMap(const QVariantMap &map)
{
   // ### this is implemented the trivial way, not the most efficient way

   QJsonObject object;
   for (QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it) {
      object.insert(it.key(), QJsonValue::fromVariant(it.value()));
   }
   return object;
}

/*!
    Converts this object to a QVariantMap.

    Returns the created map.
 */
QVariantMap QJsonObject::toVariantMap() const
{
   QVariantMap map;
   if (o) {
      for (uint i = 0; i < o->length; ++i) {
         QJsonPrivate::Entry *e = o->entryAt(i);
         map.insert(e->key(), QJsonValue(d, o, e->value).toVariant());
      }
   }
   return map;
}

/*!
    Returns a list of all keys in this object.
 */
QStringList QJsonObject::keys() const
{
   if (!d) {
      return QStringList();
   }

   QStringList keys;

   for (uint i = 0; i < o->length; ++i) {
      QJsonPrivate::Entry *e = o->entryAt(i);
      keys.append(e->key());
   }

   return keys;
}

/*!
    Returns the the number of (key, value) pairs stored in the object.
 */
int QJsonObject::size() const
{
   if (!d) {
      return 0;
   }

   return o->length;
}

/*!
    Returns \c true if the object is empty. This is the same as size() == 0.

    \sa size()
 */
bool QJsonObject::isEmpty() const
{
   if (!d) {
      return true;
   }

   return !o->length;
}

/*!
    Returns a QJsonValue representing the value for the key \a key.

    The returned QJsonValue is \c Undefined, if the key does not exist.

    \sa QJsonValue, QJsonValue::isUndefined()
 */
QJsonValue QJsonObject::value(const QString &key) const
{
   if (!d) {
      return QJsonValue();
   }

   bool keyExists;
   int i = o->indexOf(key, &keyExists);
   if (!keyExists) {
      return QJsonValue(QJsonValue::Undefined);
   }
   return QJsonValue(d, o, o->entryAt(i)->value);
}

/*!
    Returns a QJsonValue representing the value for the key \a key.

    This does the same as value().

    The returned QJsonValue is \c Undefined, if the key does not exist.

    \sa value(), QJsonValue, QJsonValue::isUndefined()
 */
QJsonValue QJsonObject::operator [](const QString &key) const
{
   return value(key);
}

/*!
    Returns a reference to the value for \a key.

    The return value is of type QJsonValueRef, a helper class for QJsonArray
    and QJsonObject. When you get an object of type QJsonValueRef, you can
    use it as if it were a reference to a QJsonValue. If you assign to it,
    the assignment will apply to the character in the QJsonArray of QJsonObject
    from which you got the reference.

    \sa value()
 */
QJsonValueRef QJsonObject::operator [](const QString &key)
{
   // ### somewhat inefficient, as we lookup the key twice if it doesn't yet exist
   bool keyExists = false;
   int index = o ? o->indexOf(key, &keyExists) : -1;
   if (!keyExists) {
      iterator i = insert(key, QJsonValue());
      index = i.i;
   }
   return QJsonValueRef(this, index);
}

/*!
    Inserts a new item with the key \a key and a value of \a value.

    If there is already an item with the key \a key then that item's value
    is replaced with \a value.

    Returns an iterator pointing to the inserted item.

    If the value is QJsonValue::Undefined, it will cause the key to get removed
    from the object. The returned iterator will then point to end()

    \sa remove(), take(), QJsonObject::iterator, end()
 */
QJsonObject::iterator QJsonObject::insert(const QString &key, const QJsonValue &value)
{
   if (value.t == QJsonValue::Undefined) {
      remove(key);
      return end();
   }
   QJsonValue val = value;

   bool latinOrIntValue;
   int valueSize = QJsonPrivate::Value::requiredStorage(val, &latinOrIntValue);

   bool latinKey = QJsonPrivate::useCompressed(key);
   int valueOffset = sizeof(QJsonPrivate::Entry) + QJsonPrivate::qStringSize(key, latinKey);
   int requiredSize = valueOffset + valueSize;

   detach(requiredSize + sizeof(QJsonPrivate::offset)); // offset for the new index entry

   if (!o->length) {
      o->tableOffset = sizeof(QJsonPrivate::Object);
   }

   bool keyExists = false;
   int pos = o->indexOf(key, &keyExists);
   if (keyExists) {
      ++d->compactionCounter;
   }

   uint off = o->reserveSpace(requiredSize, pos, 1, keyExists);
   if (!off) {
      return end();
   }

   QJsonPrivate::Entry *e = o->entryAt(pos);
   e->value.type = val.t;
   e->value.latinKey = latinKey;
   e->value.latinOrIntValue = latinOrIntValue;
   e->value.value = QJsonPrivate::Value::valueToStore(val, (char *)e - (char *)o + valueOffset);
   QJsonPrivate::copyString((char *)(e + 1), key, latinKey);
   if (valueSize) {
      QJsonPrivate::Value::copyData(val, (char *)e + valueOffset, latinOrIntValue);
   }

   if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(o->length) / 2u) {
      compact();
   }

   return iterator(this, pos);
}

/*!
    Removes \a key from the object.

    \sa insert(), take()
 */
void QJsonObject::remove(const QString &key)
{
   if (!d) {
      return;
   }

   bool keyExists;
   int index = o->indexOf(key, &keyExists);
   if (!keyExists) {
      return;
   }

   detach();
   o->removeItems(index, 1);
   ++d->compactionCounter;
   if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(o->length) / 2u) {
      compact();
   }
}

/*!
    Removes \a key from the object.

    Returns a QJsonValue containing the value referenced by \a key.
    If \a key was not contained in the object, the returned QJsonValue
    is Undefined.

    \sa insert(), remove(), QJsonValue
 */
QJsonValue QJsonObject::take(const QString &key)
{
   if (!o) {
      return QJsonValue(QJsonValue::Undefined);
   }

   bool keyExists;
   int index = o->indexOf(key, &keyExists);
   if (!keyExists) {
      return QJsonValue(QJsonValue::Undefined);
   }

   QJsonValue v(d, o, o->entryAt(index)->value);
   detach();
   o->removeItems(index, 1);
   ++d->compactionCounter;
   if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(o->length) / 2u) {
      compact();
   }

   return v;
}

/*!
    Returns \c true if the object contains key \a key.

    \sa insert(), remove(), take()
 */
bool QJsonObject::contains(const QString &key) const
{
   if (!o) {
      return false;
   }

   bool keyExists;
   o->indexOf(key, &keyExists);
   return keyExists;
}

/*!
    Returns \c true if \a other is equal to this object
 */
bool QJsonObject::operator==(const QJsonObject &other) const
{
   if (o == other.o) {
      return true;
   }

   if (!o) {
      return !other.o->length;
   }
   if (!other.o) {
      return !o->length;
   }
   if (o->length != other.o->length) {
      return false;
   }

   for (uint i = 0; i < o->length; ++i) {
      QJsonPrivate::Entry *e = o->entryAt(i);
      QJsonValue v(d, o, e->value);
      if (other.value(e->key()) != v) {
         return false;
      }
   }

   return true;
}

/*!
    Returns \c true if \a other is not equal to this object
 */
bool QJsonObject::operator!=(const QJsonObject &other) const
{
   return !(*this == other);
}

/*!
    Removes the (key, value) pair pointed to by the iterator \a it
    from the map, and returns an iterator to the next item in the
    map.

    \sa remove()
 */
QJsonObject::iterator QJsonObject::erase(QJsonObject::iterator it)
{
   Q_ASSERT(d && d->ref.load() == 1);

   if (it.o != this || it.i < 0 || it.i >= (int)o->length) {
      return iterator(this, o->length);
   }

   int index = it.i;

   o->removeItems(index, 1);
   ++d->compactionCounter;
   if (d->compactionCounter > 32u && d->compactionCounter >= unsigned(o->length) / 2u) {
      compact();
   }

   // iterator hasn't changed
   return it;
}

/*!
    Returns an iterator pointing to the item with key \a key in the
    map.

    If the map contains no item with key \a key, the function
    returns end().
 */
QJsonObject::iterator QJsonObject::find(const QString &key)
{
   bool keyExists = false;
   int index = o ? o->indexOf(key, &keyExists) : 0;
   if (!keyExists) {
      return end();
   }
   detach();
   return iterator(this, index);
}

/*! \fn QJsonObject::const_iterator QJsonObject::find(const QString &key) const

    \overload
*/

/*!
    Returns an const iterator pointing to the item with key \a key in the
    map.

    If the map contains no item with key \a key, the function
    returns constEnd().
 */
QJsonObject::const_iterator QJsonObject::constFind(const QString &key) const
{
   bool keyExists = false;
   int index = o ? o->indexOf(key, &keyExists) : 0;
   if (!keyExists) {
      return end();
   }
   return const_iterator(this, index);
}

void QJsonObject::detach(uint reserve)
{
   if (!d) {
      d = new QJsonPrivate::Data(reserve, QJsonValue::Object);
      o = static_cast<QJsonPrivate::Object *>(d->header->root());
      d->ref.ref();
      return;
   }
   if (reserve == 0 && d->ref.load() == 1) {
      return;
   }

   QJsonPrivate::Data *x = d->clone(o, reserve);
   x->ref.ref();
   if (!d->ref.deref()) {
      delete d;
   }
   d = x;
   o = static_cast<QJsonPrivate::Object *>(d->header->root());
}

/*!
    \internal
 */
void QJsonObject::compact()
{
   if (!d || !d->compactionCounter) {
      return;
   }

   detach();
   d->compact();
   o = static_cast<QJsonPrivate::Object *>(d->header->root());
}

/*!
    \internal
 */
QString QJsonObject::keyAt(int i) const
{
   Q_ASSERT(o && i >= 0 && i < (int)o->length);

   QJsonPrivate::Entry *e = o->entryAt(i);
   return e->key();
}

/*!
    \internal
 */
QJsonValue QJsonObject::valueAt(int i) const
{
   if (!o || i < 0 || i >= (int)o->length) {
      return QJsonValue(QJsonValue::Undefined);
   }

   QJsonPrivate::Entry *e = o->entryAt(i);
   return QJsonValue(d, o, e->value);
}

/*!
    \internal
 */
void QJsonObject::setValueAt(int i, const QJsonValue &val)
{
   Q_ASSERT(o && i >= 0 && i < (int)o->length);

   QJsonPrivate::Entry *e = o->entryAt(i);
   insert(e->key(), val);
}

QDebug operator<<(QDebug dbg, const QJsonObject &o)
{
   if (!o.o) {
      dbg << "QJsonObject()";
      return dbg;
   }
   QByteArray json;
   QJsonPrivate::Writer::objectToJson(o.o, json, 0, true);
   dbg.nospace() << "QJsonObject("
                 << json.constData() // print as utf-8 string without extra quotation marks
                 << ")";
   return dbg.space();
}

QT_END_NAMESPACE
