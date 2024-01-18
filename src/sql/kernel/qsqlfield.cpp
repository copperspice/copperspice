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

#include <qsqlfield.h>
#include <qatomic.h>
#include <qdebug.h>

class QSqlFieldPrivate
{
 public:
   QSqlFieldPrivate(const QString &name, QVariant::Type type)
      : ref(1), nm(name), ro(false), type(type), req(QSqlField::Unknown),
        len(-1), prec(-1), tp(-1), gen(true), autoval(false) {
   }

   QSqlFieldPrivate(const QSqlFieldPrivate &other)
      : ref(1),
        nm(other.nm),
        ro(other.ro),
        type(other.type),
        req(other.req),
        len(other.len),
        prec(other.prec),
        def(other.def),
        tp(other.tp),
        gen(other.gen),
        autoval(other.autoval) {
   }

   bool operator==(const QSqlFieldPrivate &other) const {
      return (nm == other.nm
            && ro == other.ro
            && type == other.type
            && req == other.req
            && len == other.len
            && prec == other.prec
            && def == other.def
            && gen == other.gen
            && autoval == other.autoval);
   }

   QAtomicInt ref;
   QString nm;
   uint ro: 1;
   QVariant::Type type;
   QSqlField::RequiredStatus req;
   int len;
   int prec;
   QVariant def;
   int tp;
   uint gen: 1;
   uint autoval: 1;
};

QSqlField::QSqlField(const QString &fieldName, QVariant::Type type)
{
   d = new QSqlFieldPrivate(fieldName, type);
   val = QVariant();
}

QSqlField::QSqlField(const QSqlField &other)
{
   d = other.d;
   d->ref.ref();
   val = other.val;
}

QSqlField &QSqlField::operator=(const QSqlField &other)
{
   qAtomicAssign(d, other.d);
   val = other.val;
   return *this;
}

bool QSqlField::operator==(const QSqlField &other) const
{
   return ((d == other.d || *d == *other.d)
         && val == other.val);
}

QSqlField::~QSqlField()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

/*!
    Sets the required status of this field to \a required.

    \sa requiredStatus() setType() setLength() setPrecision() setDefaultValue() setGenerated() setReadOnly()
*/
void QSqlField::setRequiredStatus(RequiredStatus required)
{
   detach();
   d->req = required;
}

/*! \fn void QSqlField::setRequired(bool required)

    Sets the required status of this field to \l Required if \a
    required is true; otherwise sets it to \l Optional.

    \sa setRequiredStatus(), requiredStatus()
*/

/*!
    Sets the field's length to \a fieldLength. For strings this is the
    maximum number of characters the string can hold; the meaning
    varies for other types.

    \sa length() setType() setRequiredStatus() setPrecision() setDefaultValue() setGenerated() setReadOnly()
*/
void QSqlField::setLength(int fieldLength)
{
   detach();
   d->len = fieldLength;
}

/*!
    Sets the field's \a precision. This only affects numeric fields.

    \sa precision() setType() setRequiredStatus() setLength() setDefaultValue() setGenerated() setReadOnly()
*/
void QSqlField::setPrecision(int precision)
{
   detach();
   d->prec = precision;
}

/*!
    Sets the default value used for this field to \a value.

    \sa defaultValue() value() setType() setRequiredStatus() setLength() setPrecision() setGenerated() setReadOnly()
*/
void QSqlField::setDefaultValue(const QVariant &value)
{
   detach();
   d->def = value;
}

/*!
    \internal
*/
void QSqlField::setSqlType(int type)
{
   detach();
   d->tp = type;
}

/*!
    Sets the generated state. If \a gen is false, no SQL will
    be generated for this field; otherwise, Qt classes such as
    QSqlQueryModel and QSqlTableModel will generate SQL for this
    field.

    \sa isGenerated() setType() setRequiredStatus() setLength() setPrecision() setDefaultValue() setReadOnly()
*/
void QSqlField::setGenerated(bool gen)
{
   detach();
   d->gen = gen;
}


/*!
    Sets the value of the field to \a value. If the field is read-only
    (isReadOnly() returns true), nothing happens.

    If the data type of \a value differs from the field's current
    data type, an attempt is made to cast it to the proper type. This
    preserves the data type of the field in the case of assignment,
    e.g. a QString to an integer data type.

    To set the value to NULL, use clear().

    \sa value() isReadOnly() defaultValue()
*/

void QSqlField::setValue(const QVariant &value)
{
   if (isReadOnly()) {
      return;
   }
   val = value;
}

/*!
    Clears the value of the field and sets it to NULL.
    If the field is read-only, nothing happens.

    \sa setValue() isReadOnly() requiredStatus()
*/

void QSqlField::clear()
{
   if (isReadOnly()) {
      return;
   }
   val = QVariant();
}

/*!
    Sets the name of the field to \a name.

    \sa name()
*/

void QSqlField::setName(const QString &name)
{
   detach();
   d->nm = name;
}

void QSqlField::setReadOnly(bool readOnly)
{
   detach();
   d->ro = readOnly;
}


QString QSqlField::name() const
{
   return d->nm;
}

QVariant::Type QSqlField::type() const
{
   return d->type;
}

void QSqlField::setType(QVariant::Type type)
{
   detach();
   d->type = type;
   if (!val.isValid()) {
      val = QVariant();
   }
}

bool QSqlField::isReadOnly() const
{
   return d->ro;
}

bool QSqlField::isNull() const
{
   return ! val.isValid();
}

void QSqlField::detach()
{
   qAtomicDetach(d);
}

QSqlField::RequiredStatus QSqlField::requiredStatus() const
{
   return d->req;
}

int QSqlField::length() const
{
   return d->len;
}

int QSqlField::precision() const
{
   return d->prec;
}

QVariant QSqlField::defaultValue() const
{
   return d->def;
}

int QSqlField::typeID() const
{
   return d->tp;
}

bool QSqlField::isGenerated() const
{
   return d->gen;
}

bool QSqlField::isValid() const
{
   return d->type != QVariant::Invalid;
}

QDebug operator<<(QDebug dbg, const QSqlField &f)
{
   dbg.nospace() << "QSqlField(" << f.name() << ", " << QVariant::typeToName(f.type());

   if (f.length() >= 0) {
      dbg.nospace() << ", length: " << f.length();
   }

   if (f.precision() >= 0) {
      dbg.nospace() << ", precision: " << f.precision();
   }

   if (f.requiredStatus() != QSqlField::Unknown)
      dbg.nospace() << ", required: "
         << (f.requiredStatus() == QSqlField::Required ? "yes" : "no");

   dbg.nospace() << ", generated: " << (f.isGenerated() ? "yes" : "no");

   if (f.typeID() >= 0) {
      dbg.nospace() << ", typeID: " << f.typeID();
   }

   if (f.defaultValue().isValid()) {
      dbg.nospace() << ", auto-value: \"" << f.defaultValue().toString() << '\"';
   }

   dbg.nospace() << ')';

   return dbg.space();
}

bool QSqlField::isAutoValue() const
{
   return d->autoval;
}

void QSqlField::setAutoValue(bool autoVal)
{
   detach();
   d->autoval = autoVal;
}
