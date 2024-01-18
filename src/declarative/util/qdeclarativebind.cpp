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

#include <qdeclarativebind_p.h>
#include <qdeclarativenullablevalue_p_p.h>
#include <qdeclarativeguard_p.h>
#include <qdeclarativeengine.h>
#include <qdeclarativecontext.h>
#include <qdeclarativeproperty.h>
#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptcontext.h>
#include <QtScript/qscriptengine.h>

QT_BEGIN_NAMESPACE

class QDeclarativeBindPrivate
{
 public:
   QDeclarativeBindPrivate() : when(true), componentComplete(true), obj(0) {}

   bool when : 1;
   bool componentComplete : 1;
   QDeclarativeGuard<QObject> obj;
   QString prop;
   QDeclarativeNullableValue<QVariant> value;
};


/*!
    \qmlclass Binding QDeclarativeBind
    \ingroup qml-working-with-data
    \since 4.7
    \brief The Binding element allows arbitrary property bindings to be created.

    Sometimes it is necessary to bind to a property of an object that wasn't
    directly instantiated by QML - generally a property of a class exported
    to QML by C++. In these cases, regular property binding doesn't work. Binding
    allows you to bind any value to any property.

    For example, imagine a C++ application that maps an "app.enteredText"
    property into QML. You could use Binding to update the enteredText property
    like this.
    \code
    TextEdit { id: myTextField; text: "Please type here..." }
    Binding { target: app; property: "enteredText"; value: myTextField.text }
    \endcode
    Whenever the text in the TextEdit is updated, the C++ property will be
    updated also.

    If the binding target or binding property is changed, the bound value is
    immediately pushed onto the new target.

    \sa QtDeclarative
*/
QDeclarativeBind::QDeclarativeBind(QObject *parent)
   : QObject(*(new QDeclarativeBindPrivate), parent)
{
}

QDeclarativeBind::~QDeclarativeBind()
{
}

/*!
    \qmlproperty bool Binding::when

    This property holds when the binding is active.
    This should be set to an expression that evaluates to true when you want the binding to be active.

    \code
    Binding {
        target: contactName; property: 'text'
        value: name; when: list.ListView.isCurrentItem
    }
    \endcode
*/
bool QDeclarativeBind::when() const
{
   Q_D(const QDeclarativeBind);
   return d->when;
}

void QDeclarativeBind::setWhen(bool v)
{
   Q_D(QDeclarativeBind);
   d->when = v;
   eval();
}

/*!
    \qmlproperty Object Binding::target

    The object to be updated.
*/
QObject *QDeclarativeBind::object()
{
   Q_D(const QDeclarativeBind);
   return d->obj;
}

void QDeclarativeBind::setObject(QObject *obj)
{
   Q_D(QDeclarativeBind);
   d->obj = obj;
   eval();
}

/*!
    \qmlproperty string Binding::property

    The property to be updated.
*/
QString QDeclarativeBind::property() const
{
   Q_D(const QDeclarativeBind);
   return d->prop;
}

void QDeclarativeBind::setProperty(const QString &p)
{
   Q_D(QDeclarativeBind);
   d->prop = p;
   eval();
}

/*!
    \qmlproperty any Binding::value

    The value to be set on the target object and property.  This can be a
    constant (which isn't very useful), or a bound expression.
*/
QVariant QDeclarativeBind::value() const
{
   Q_D(const QDeclarativeBind);
   return d->value.value;
}

void QDeclarativeBind::setValue(const QVariant &v)
{
   Q_D(QDeclarativeBind);
   d->value.value = v;
   d->value.isNull = false;
   eval();
}

void QDeclarativeBind::classBegin()
{
   Q_D(QDeclarativeBind);
   d->componentComplete = false;
}

void QDeclarativeBind::componentComplete()
{
   Q_D(QDeclarativeBind);
   d->componentComplete = true;
   eval();
}

void QDeclarativeBind::eval()
{
   Q_D(QDeclarativeBind);
   if (!d->obj || d->value.isNull || !d->when || !d->componentComplete) {
      return;
   }

   QDeclarativeProperty prop(d->obj, d->prop);
   prop.write(d->value.value);
}

QT_END_NAMESPACE
