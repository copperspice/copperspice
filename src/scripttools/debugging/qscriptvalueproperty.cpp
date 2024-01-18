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

#include "qscriptvalueproperty_p.h"

#include <QtCore/qatomic.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QScriptValuePropertyPrivate
{
 public:
   QScriptValuePropertyPrivate();
   ~QScriptValuePropertyPrivate();

   QString name;
   QScriptValue value;
   QScriptValue::PropertyFlags flags;

   QAtomicInt ref;
};

QScriptValuePropertyPrivate::QScriptValuePropertyPrivate()
{
   ref.store(0);
}

QScriptValuePropertyPrivate::~QScriptValuePropertyPrivate()
{
}

/*!
  Constructs an invalid QScriptValueProperty.
*/
QScriptValueProperty::QScriptValueProperty()
   : d_ptr(0)
{
}

/*!
  Constructs a QScriptValueProperty with the given \a name,
  \a value and \a flags.
*/
QScriptValueProperty::QScriptValueProperty(const QString &name,
      const QScriptValue &value,
      QScriptValue::PropertyFlags flags)
   : d_ptr(new QScriptValuePropertyPrivate)
{
   d_ptr->name = name;
   d_ptr->value = value;
   d_ptr->flags = flags;
   d_ptr->ref.ref();
}

/*!
  Constructs a QScriptValueProperty that is a copy of the \a other property.
*/
QScriptValueProperty::QScriptValueProperty(const QScriptValueProperty &other)
   : d_ptr(other.d_ptr.data())
{
   if (d_ptr) {
      d_ptr->ref.ref();
   }
}

/*!
  Destroys this QScriptValueProperty.
*/
QScriptValueProperty::~QScriptValueProperty()
{
}

/*!
  Assigns the \a other property to this QScriptValueProperty.
*/
QScriptValueProperty &QScriptValueProperty::operator=(const QScriptValueProperty &other)
{
   d_ptr.assign(other.d_ptr.data());
   return *this;
}

/*!
  Returns the name of this QScriptValueProperty.
*/
QString QScriptValueProperty::name() const
{
   Q_D(const QScriptValueProperty);
   if (!d) {
      return QString();
   }
   return d->name;
}

/*!
  Returns the value of this QScriptValueProperty.
*/
QScriptValue QScriptValueProperty::value() const
{
   Q_D(const QScriptValueProperty);
   if (!d) {
      return QScriptValue();
   }
   return d->value;
}

/*!
  Returns the flags of this QScriptValueProperty.
*/
QScriptValue::PropertyFlags QScriptValueProperty::flags() const
{
   Q_D(const QScriptValueProperty);
   if (!d) {
      return 0;
   }
   return d->flags;
}

/*!
  Returns true if this QScriptValueProperty is valid, otherwise
  returns false.
*/
bool QScriptValueProperty::isValid() const
{
   Q_D(const QScriptValueProperty);
   return (d != 0);
}

QT_END_NAMESPACE
