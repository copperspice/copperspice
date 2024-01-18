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

#ifndef QDECLARATIVEOBJECTSCRIPTCLASS_P_H
#define QDECLARATIVEOBJECTSCRIPTCLASS_P_H

#include <qdeclarativepropertycache_p.h>
#include <qdeclarativetypenamecache_p.h>
#include <qscriptdeclarativeclass_p.h>
#include <QtScript/qscriptengine.h>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QScriptContext;
class QScriptEngine;
class QDeclarativeContextData;
struct MethodData;

class QDeclarativeObjectMethodScriptClass : public QScriptDeclarativeClass
{
 public:
   QDeclarativeObjectMethodScriptClass(QDeclarativeEngine *);
   ~QDeclarativeObjectMethodScriptClass();

   QScriptValue newMethod(QObject *, const QDeclarativePropertyCache::Data *);

 protected:
   virtual Value call(Object *, QScriptContext *);
   virtual QScriptClass::QueryFlags queryProperty(Object *, const Identifier &, QScriptClass::QueryFlags flags);
   virtual Value property(Object *, const Identifier &);

 private:
   int enumType(const QMetaObject *, const QString &);

   Value callPrecise(QObject *, const QDeclarativePropertyCache::Data &, QScriptContext *);
   Value callOverloaded(MethodData *, QScriptContext *);
   Value callMethod(QObject *, int index, int returnType, int argCount, int *argTypes, QScriptContext *ctxt);

   int matchScore(const QScriptValue &, int, const QByteArray &);
   QDeclarativePropertyCache::Data *relatedMethod(QObject *, QDeclarativePropertyCache::Data *current,
         QDeclarativePropertyCache::Data &dummy);

   PersistentIdentifier m_connectId;
   PersistentIdentifier m_disconnectId;
   QScriptValue m_connect;
   QScriptValue m_disconnect;

   static QScriptValue connect(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue disconnect(QScriptContext *context, QScriptEngine *engine);

   QDeclarativeEngine *engine;
};

class QDeclarativeObjectScriptClass : public QScriptDeclarativeClass
{
 public:
   QDeclarativeObjectScriptClass(QDeclarativeEngine *);
   ~QDeclarativeObjectScriptClass();

   QScriptValue newQObject(QObject *, int type = QMetaType::QObjectStar);

   QObject *toQObject(const QScriptValue &) const;
   int objectType(const QScriptValue &) const;

   enum QueryHint {
      ImplicitObject = 0x01,
      SkipAttachedProperties = 0x02
   };
   using QueryHints = QFlags<QueryHint>;

   QScriptClass::QueryFlags queryProperty(QObject *, const Identifier &,
                                          QScriptClass::QueryFlags flags,
                                          QDeclarativeContextData *evalContext,
                                          QueryHints hints = 0);

   Value property(QObject *, const Identifier &);

   void setProperty(QObject *, const Identifier &name, const QScriptValue &,
                    QScriptContext *context, QDeclarativeContextData *evalContext = 0);
   virtual QStringList propertyNames(Object *);
   virtual bool compare(Object *, Object *);

 protected:
   virtual QScriptClass::QueryFlags queryProperty(Object *, const Identifier &,
         QScriptClass::QueryFlags flags);

   virtual Value property(Object *, const Identifier &);
   virtual void setProperty(Object *, const Identifier &name, const QScriptValue &);
   virtual bool isQObject() const;
   virtual QObject *toQObject(Object *, bool *ok = 0);

 private:
   friend class QDeclarativeObjectMethodScriptClass;
   QDeclarativeObjectMethodScriptClass methods;

   QDeclarativeTypeNameCache::Data *lastTNData;
   QDeclarativePropertyCache::Data *lastData;
   QDeclarativePropertyCache::Data local;

   PersistentIdentifier m_destroyId;
   PersistentIdentifier m_toStringId;
   QScriptValue m_destroy;
   QScriptValue m_toString;

   static QScriptValue tostring(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue destroy(QScriptContext *context, QScriptEngine *engine);

   QDeclarativeEngine *engine;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativeObjectScriptClass::QueryHints);

QT_END_NAMESPACE

#endif // QDECLARATIVEOBJECTSCRIPTCLASS_P_H

