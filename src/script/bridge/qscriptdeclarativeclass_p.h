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

#ifndef QSCRIPTDECLARATIVECLASS_P_H
#define QSCRIPTDECLARATIVECLASS_P_H

#include <qscriptvalue.h>
#include <qscriptclass.h>

class QScriptDeclarativeClassPrivate;
class PersistentIdentifierPrivate;
class QScriptContext;

class Q_SCRIPT_EXPORT QScriptDeclarativeClass
{
 public:
#define QT_HAVE_QSCRIPTDECLARATIVECLASS_VALUE
   class Q_SCRIPT_EXPORT Value
   {
    public:
      Value();
      Value(const Value &);

      Value(QScriptContext *, int);
      Value(QScriptContext *, uint);
      Value(QScriptContext *, bool);
      Value(QScriptContext *, double);
      Value(QScriptContext *, float);
      Value(QScriptContext *, const QString &);
      Value(QScriptContext *, const QScriptValue &);
      Value(QScriptEngine *, int);
      Value(QScriptEngine *, uint);
      Value(QScriptEngine *, bool);
      Value(QScriptEngine *, double);
      Value(QScriptEngine *, float);
      Value(QScriptEngine *, const QString &);
      Value(QScriptEngine *, const QScriptValue &);
      ~Value();

      QScriptValue toScriptValue(QScriptEngine *) const;
    private:
      char dummy[8];
   };

   typedef void *Identifier;

   struct Object {
      virtual ~Object() {}
   };

   static QScriptValue newObject(QScriptEngine *, QScriptDeclarativeClass *, Object *);
   static Value newObjectValue(QScriptEngine *, QScriptDeclarativeClass *, Object *);
   static QScriptDeclarativeClass *scriptClass(const QScriptValue &);
   static Object *object(const QScriptValue &);

   static QScriptValue function(const QScriptValue &, const Identifier &);
   static QScriptValue property(const QScriptValue &, const Identifier &);
   static Value functionValue(const QScriptValue &, const Identifier &);
   static Value propertyValue(const QScriptValue &, const Identifier &);

   static QScriptValue scopeChainValue(QScriptContext *, int index);
   static QScriptContext *pushCleanContext(QScriptEngine *);

   static QScriptValue newStaticScopeObject(QScriptEngine *, int propertyCount, const QString *names,
      const QScriptValue *values, const QScriptValue::PropertyFlags *flags);

   static QScriptValue newStaticScopeObject(QScriptEngine *);

   class Q_SCRIPT_EXPORT PersistentIdentifier
   {
    public:
      Identifier identifier;

      PersistentIdentifier();
      ~PersistentIdentifier();

      PersistentIdentifier(const PersistentIdentifier &other);
      PersistentIdentifier &operator=(const PersistentIdentifier &other);

      QString toString() const;

    private:
      friend class QScriptDeclarativeClass;

      PersistentIdentifier(QScriptEnginePrivate *e)
         : identifier(nullptr), engine(e), d(nullptr)
      {
      }

      QScriptEnginePrivate *engine;
      void *d;
   };

   QScriptDeclarativeClass(QScriptEngine *engine);
   virtual ~QScriptDeclarativeClass();

   QScriptEngine *engine() const;

   bool supportsCall() const;
   void setSupportsCall(bool);

   PersistentIdentifier createPersistentIdentifier(const QString &);
   PersistentIdentifier createPersistentIdentifier(const Identifier &);

   QString toString(const Identifier &);
   bool startsWithUpper(const Identifier &);
   quint32 toArrayIndex(const Identifier &, bool *ok);

   virtual QScriptClass::QueryFlags queryProperty(Object *, const Identifier &,
      QScriptClass::QueryFlags flags);

   virtual Value property(Object *, const Identifier &);
   virtual void setProperty(Object *, const Identifier &name, const QScriptValue &);
   virtual QScriptValue::PropertyFlags propertyFlags(Object *, const Identifier &);
   virtual Value call(Object *, QScriptContext *);
   virtual bool compare(Object *, Object *);

   virtual QStringList propertyNames(Object *);

   virtual bool isQObject() const;
   virtual QObject *toQObject(Object *, bool *ok = nullptr);
   virtual QVariant toVariant(Object *, bool *ok = nullptr);

   QScriptContext *context() const;

 protected:
   friend class QScriptDeclarativeClassPrivate;
   QScopedPointer<QScriptDeclarativeClassPrivate> d_ptr;
};

#endif
