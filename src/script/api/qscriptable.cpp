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

#include "qscriptable.h"
#include "qscriptable_p.h"
#include "qscriptengine.h"

QScriptable::QScriptable()
   : d_ptr(new QScriptablePrivate())
{
   d_ptr->q_ptr = this;
}

/*!
  \internal
*/
QScriptable::~QScriptable()
{
}

/*!
  Returns a pointer to the QScriptEngine associated with the current
  Qt function call, or 0 if the Qt function was not invoked from
  script code.
*/
QScriptEngine *QScriptable::engine() const
{
   Q_D(const QScriptable);
   return d->engine;
}

/*!
  Returns a pointer to the QScriptContext associated with the current
  Qt function call, or 0 if the Qt function was not invoked from
  script code.
*/
QScriptContext *QScriptable::context() const
{
   if (QScriptEngine *e = engine()) {
      return e->currentContext();
   }

   return nullptr;
}

/*!
  Returns the `this' object associated with the current Qt function
  call, or an invalid QScriptValue if the Qt function was not invoked
  from script code.
*/

QScriptValue QScriptable::thisObject() const
{
   if (QScriptContext *c = context()) {
      return c->thisObject();
   }

   return QScriptValue();
}

/*!
  Returns the number of arguments passed to the function in this
  invocation, or -1 if the Qt function was not invoked from script
  code.

  \sa argument()
*/
int QScriptable::argumentCount() const
{
   if (QScriptContext *c = context()) {
      return c->argumentCount();
   }

   return -1;
}

/*!
  Returns the function argument at the given \a index, or an invalid
  QScriptValue if the Qt function was not invoked from script code.

  \sa argumentCount()
*/
QScriptValue QScriptable::argument(int index) const
{
   if (QScriptContext *c = context()) {
      return c->argument(index);
   }

   return QScriptValue();
}

