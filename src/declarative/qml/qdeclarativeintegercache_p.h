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

#ifndef QDECLARATIVEINTEGERCACHE_P_H
#define QDECLARATIVEINTEGERCACHE_P_H

#include <qdeclarativerefcount_p.h>
#include <qdeclarativecleanup_p.h>
#include <QtCore/qhash.h>
#include <qscriptdeclarativeclass_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeType;
class QDeclarativeEngine;

class QDeclarativeIntegerCache : public QDeclarativeRefCount, public QDeclarativeCleanup
{
 public:
   QDeclarativeIntegerCache(QDeclarativeEngine *);
   virtual ~QDeclarativeIntegerCache();

   inline int count() const;
   void add(const QString &, int);
   int value(const QString &);
   QString findId(int value) const;
   inline int value(const QScriptDeclarativeClass::Identifier &id) const;

 protected:
   virtual void clear();

 private:
   struct Data : public QScriptDeclarativeClass::PersistentIdentifier {
      Data(const QScriptDeclarativeClass::PersistentIdentifier &i, int v)
         : QScriptDeclarativeClass::PersistentIdentifier(i), value(v) {}

      int value;
   };

   typedef QHash<QString, Data *> StringCache;
   typedef QHash<QScriptDeclarativeClass::Identifier, Data *> IdentifierCache;

   StringCache stringCache;
   IdentifierCache identifierCache;
   QDeclarativeEngine *engine;
};

int QDeclarativeIntegerCache::value(const QScriptDeclarativeClass::Identifier &id) const
{
   Data *d = identifierCache.value(id);
   return d ? d->value : -1;
}

int QDeclarativeIntegerCache::count() const
{
   return stringCache.count();
}

QT_END_NAMESPACE

#endif // QDECLARATIVEINTEGERCACHE_P_H

