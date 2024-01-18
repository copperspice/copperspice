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

#include "qdeclarativescriptstring.h"

QT_BEGIN_NAMESPACE

class QDeclarativeScriptStringPrivate : public QSharedData
{
 public:
   QDeclarativeScriptStringPrivate() : context(0), scope(0) {}

   QDeclarativeContext *context;
   QObject *scope;
   QString script;
};

/*!
\class QDeclarativeScriptString
\since 4.7
\brief The QDeclarativeScriptString class encapsulates a script and its context.

QDeclarativeScriptString is used to create QObject properties that accept a script "assignment" from QML.

Normally, the following QML would result in a binding being established for the \c script
property; i.e. \c script would be assigned the value obtained from running \c {myObj.value = Math.max(myValue, 100)}

\qml
MyType {
    script: myObj.value = Math.max(myValue, 100)
}
\endqml

If instead the property had a type of QDeclarativeScriptString,
the script itself -- \e {myObj.value = Math.max(myValue, 100)} -- would be passed to the \c script property
and the class could choose how to handle it. Typically, the class will evaluate
the script at some later time using a QDeclarativeExpression.

\code
QDeclarativeExpression expr(scriptString.context(), scriptString.script(), scriptStr.scopeObject());
expr.value();
\endcode

\sa QDeclarativeExpression
*/

/*!
Constructs an empty instance.
*/
QDeclarativeScriptString::QDeclarativeScriptString()
   :  d(new QDeclarativeScriptStringPrivate)
{
}

/*!
Copies \a other.
*/
QDeclarativeScriptString::QDeclarativeScriptString(const QDeclarativeScriptString &other)
   : d(other.d)
{
}

/*!
\internal
*/
QDeclarativeScriptString::~QDeclarativeScriptString()
{
}

/*!
Assigns \a other to this.
*/
QDeclarativeScriptString &QDeclarativeScriptString::operator=(const QDeclarativeScriptString &other)
{
   d = other.d;
   return *this;
}

/*!
Returns the context for the script.
*/
QDeclarativeContext *QDeclarativeScriptString::context() const
{
   return d->context;
}

/*!
Sets the \a context for the script.
*/
void QDeclarativeScriptString::setContext(QDeclarativeContext *context)
{
   d->context = context;
}

/*!
Returns the scope object for the script.
*/
QObject *QDeclarativeScriptString::scopeObject() const
{
   return d->scope;
}

/*!
Sets the scope \a object for the script.
*/
void QDeclarativeScriptString::setScopeObject(QObject *object)
{
   d->scope = object;
}

/*!
Returns the script text.
*/
QString QDeclarativeScriptString::script() const
{
   return d->script;
}

/*!
Sets the \a script text.
*/
void QDeclarativeScriptString::setScript(const QString &script)
{
   d->script = script;
}

QT_END_NAMESPACE

