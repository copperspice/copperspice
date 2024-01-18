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

#ifndef QDECLARATIVECONTEXTSCRIPTCLASS_P_H
#define QDECLARATIVECONTEXTSCRIPTCLASS_P_H

#include <qdeclarativetypenamecache_p.h>
#include <qscriptdeclarativeclass_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QDeclarativeContext;
class QDeclarativeContextData;

class QDeclarativeContextScriptClass : public QScriptDeclarativeClass
{
 public:
   QDeclarativeContextScriptClass(QDeclarativeEngine *);
   ~QDeclarativeContextScriptClass();

   QScriptValue newContext(QDeclarativeContextData *, QObject * = 0);
   QScriptValue newUrlContext(QDeclarativeContextData *, QObject *, const QString &);
   QScriptValue newUrlContext(const QString &);
   QScriptValue newSharedContext();

   QDeclarativeContextData *contextFromValue(const QScriptValue &);
   QUrl urlFromValue(const QScriptValue &);

   QObject *setOverrideObject(QScriptValue &, QObject *);

 protected:
   virtual QScriptClass::QueryFlags queryProperty(Object *, const Identifier &,
         QScriptClass::QueryFlags flags);
   virtual Value property(Object *, const Identifier &);
   virtual void setProperty(Object *, const Identifier &name, const QScriptValue &);

 private:
   QScriptClass::QueryFlags queryProperty(QDeclarativeContextData *, QObject *scopeObject,
                                          const Identifier &,
                                          QScriptClass::QueryFlags flags,
                                          bool includeTypes);

   QDeclarativeEngine *engine;

   QObject *lastScopeObject;
   QDeclarativeContextData *lastContext;
   QDeclarativeTypeNameCache::Data *lastData;
   int lastPropertyIndex;
   QScriptValue lastFunction;

   uint m_id;
};

QT_END_NAMESPACE

#endif // QDECLARATIVECONTEXTSCRIPTCLASS_P_H

