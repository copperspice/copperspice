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

#ifndef QSCRIPTCLASS_H
#define QSCRIPTCLASS_H

#include <qstring.h>
#include <qvariant.h>
#include <qscopedpointer.h>
#include <qscriptvalue.h>

class QScriptString;
class QScriptClassPropertyIterator;
class QScriptClassPrivate;

class Q_SCRIPT_EXPORT QScriptClass
{
 public:
   enum QueryFlag {
      HandlesReadAccess  = 0x01,
      HandlesWriteAccess = 0x02
   };
   using QueryFlags = QFlags<QueryFlag>;

   enum Extension {
      Callable,
      HasInstance
   };

   QScriptClass(QScriptEngine *engine);

   QScriptClass(const QScriptClass &) = delete;
   QScriptClass &operator=(const QScriptClass &) = delete;

   virtual ~QScriptClass();

   QScriptEngine *engine() const;

   virtual QueryFlags queryProperty(const QScriptValue &object,
      const QScriptString &name, QueryFlags flags, uint *id);

   virtual QScriptValue property(const QScriptValue &object,
      const QScriptString &name, uint id);

   virtual void setProperty(QScriptValue &object, const QScriptString &name,
      uint id, const QScriptValue &value);

   virtual QScriptValue::PropertyFlags propertyFlags(
      const QScriptValue &object, const QScriptString &name, uint id);

   virtual QScriptClassPropertyIterator *newIterator(const QScriptValue &object);

   virtual QScriptValue prototype() const;

   virtual QString name() const;

   virtual bool supportsExtension(Extension extension) const;
   virtual QVariant extension(Extension extension,
      const QVariant &argument = QVariant());

 protected:
   QScriptClass(QScriptEngine *engine, QScriptClassPrivate &dd);
   QScopedPointer<QScriptClassPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QScriptClass)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QScriptClass::QueryFlags)

#endif
