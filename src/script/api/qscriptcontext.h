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

#ifndef QSCRIPTCONTEXT_H
#define QSCRIPTCONTEXT_H

#include <qscriptvalue.h>

class QScriptContextPrivate;

class Q_SCRIPT_EXPORT QScriptContext
{
 public:
   enum ExecutionState {
      NormalState,
      ExceptionState
   };

   enum Error {
      UnknownError,
      ReferenceError,
      SyntaxError,
      TypeError,
      RangeError,
      URIError
   };

   QScriptContext(const QScriptContext &) = delete;
   QScriptContext &operator=(const QScriptContext &) = delete;

   ~QScriptContext();

   QScriptContext *parentContext() const;
   QScriptEngine *engine() const;

   ExecutionState state() const;
   QScriptValue callee() const;

   int argumentCount() const;
   QScriptValue argument(int index) const;
   QScriptValue argumentsObject() const;

   QList<QScriptValue> scopeChain() const;
   void pushScope(const QScriptValue &object);
   QScriptValue popScope();

   QScriptValue returnValue() const;
   void setReturnValue(const QScriptValue &result);

   QScriptValue activationObject() const;
   void setActivationObject(const QScriptValue &activation);

   QScriptValue thisObject() const;
   void setThisObject(const QScriptValue &thisObject);

   bool isCalledAsConstructor() const;

   QScriptValue throwValue(const QScriptValue &value);
   QScriptValue throwError(Error error, const QString &text);
   QScriptValue throwError(const QString &text);

   QStringList backtrace() const;

   QString toString() const;

 private:
   Q_DECLARE_PRIVATE(QScriptContext)

   QScriptContext();
   QScriptContextPrivate *d_ptr;
};

CS_DECLARE_METATYPE(QScriptContext)

#endif
