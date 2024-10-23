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

#include <qsqlrecord.h>

#include <qdebug.h>
#include <qstringlist.h>
#include <qatomic.h>
#include <qsqlfield.h>
#include <qstring.h>
#include <qvector.h>

class QSqlRecordPrivate
{
 public:
   QSqlRecordPrivate();
   QSqlRecordPrivate(const QSqlRecordPrivate &other);

   bool contains(int index) {
      return index >= 0 && index < fields.count();
   }
   QString createField(int index, const QString &prefix) const;

   QVector<QSqlField> fields;
   QAtomicInt ref;
};

QSqlRecordPrivate::QSqlRecordPrivate()
{
   ref = 1;
}

QSqlRecordPrivate::QSqlRecordPrivate(const QSqlRecordPrivate &other): fields(other.fields)
{
   ref = 1;
}

QString QSqlRecordPrivate::createField(int index, const QString &prefix) const
{
   QString f;
   if (!prefix.isEmpty()) {
      f = prefix + QLatin1Char('.');
   }
   f += fields.at(index).name();
   return f;
}

QSqlRecord::QSqlRecord()
{
   d = new QSqlRecordPrivate();
}

QSqlRecord::QSqlRecord(const QSqlRecord &other)
{
   d = other.d;
   d->ref.ref();
}

QSqlRecord &QSqlRecord::operator=(const QSqlRecord &other)
{
   qAtomicAssign(d, other.d);
   return *this;
}

QSqlRecord::~QSqlRecord()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

bool QSqlRecord::operator==(const QSqlRecord &other) const
{
   return d->fields == other.d->fields;
}

QVariant QSqlRecord::value(int index) const
{
   return d->fields.value(index).value();
}

QVariant QSqlRecord::value(const QString &name) const
{
   return value(indexOf(name));
}

QString QSqlRecord::fieldName(int index) const
{
   return d->fields.value(index).name();
}

int QSqlRecord::indexOf(const QString &name) const
{
   QString nm = name.toUpper();
   for (int i = 0; i < count(); ++i) {
      if (d->fields.at(i).name().toUpper() == nm) { // TODO: case-insensitive comparison
         return i;
      }
   }
   return -1;
}

QSqlField QSqlRecord::field(int index) const
{
   return d->fields.value(index);
}

QSqlField QSqlRecord::field(const QString &name) const
{
   return field(indexOf(name));
}

void QSqlRecord::append(const QSqlField &field)
{
   detach();
   d->fields.append(field);
}

void QSqlRecord::insert(int pos, const QSqlField &field)
{
   detach();
   d->fields.insert(pos, field);
}

void QSqlRecord::replace(int pos, const QSqlField &field)
{
   if (!d->contains(pos)) {
      return;
   }

   detach();
   d->fields[pos] = field;
}

void QSqlRecord::remove(int pos)
{
   if (!d->contains(pos)) {
      return;
   }

   detach();
   d->fields.remove(pos);
}

void QSqlRecord::clear()
{
   detach();
   d->fields.clear();
}

bool QSqlRecord::isEmpty() const
{
   return d->fields.isEmpty();
}

bool QSqlRecord::contains(const QString &name) const
{
   return indexOf(name) >= 0;
}

void QSqlRecord::clearValues()
{
   detach();
   int count = d->fields.count();
   for (int i = 0; i < count; ++i) {
      d->fields[i].clear();
   }
}

void QSqlRecord::setGenerated(const QString &name, bool generated)
{
   setGenerated(indexOf(name), generated);
}

void QSqlRecord::setGenerated(int index, bool generated)
{
   if (!d->contains(index)) {
      return;
   }
   detach();
   d->fields[index].setGenerated(generated);
}

bool QSqlRecord::isNull(int index) const
{
   return d->fields.value(index).isNull();
}

bool QSqlRecord::isNull(const QString &name) const
{
   return isNull(indexOf(name));
}

void QSqlRecord::setNull(int index)
{
   if (!d->contains(index)) {
      return;
   }
   detach();
   d->fields[index].clear();
}

void QSqlRecord::setNull(const QString &name)
{
   setNull(indexOf(name));
}

bool QSqlRecord::isGenerated(const QString &name) const
{
   return isGenerated(indexOf(name));
}

bool QSqlRecord::isGenerated(int index) const
{
   return d->fields.value(index).isGenerated();
}

int QSqlRecord::count() const
{
   return d->fields.count();
}

void QSqlRecord::setValue(int index, const QVariant &val)
{
   if (!d->contains(index)) {
      return;
   }
   detach();
   d->fields[index].setValue(val);
}

void QSqlRecord::setValue(const QString &name, const QVariant &val)
{
   setValue(indexOf(name), val);
}

void QSqlRecord::detach()
{
   qAtomicDetach(d);
}

QDebug operator<<(QDebug dbg, const QSqlRecord &r)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   const int count = r.count();
   dbg << "QSqlRecord(" << count << ')';
   for (int i = 0; i < count; ++i) {
      dbg.nospace();
      dbg << '\n' << qSetFieldWidth(2) << right << i << left << qSetFieldWidth(0) << ':';
      dbg.space();
      dbg << r.field(i) << r.value(i).toString();
   }
   return dbg;
}

QSqlRecord QSqlRecord::keyValues(const QSqlRecord &keyFields) const
{
   QSqlRecord retValues(keyFields);

   for (int i = retValues.count() - 1; i >= 0; --i) {
      retValues.setValue(i, value(retValues.fieldName(i)));
   }

   return retValues;
}

