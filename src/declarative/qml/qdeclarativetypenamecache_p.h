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

#ifndef QDECLARATIVETYPENAMECACHE_P_H
#define QDECLARATIVETYPENAMECACHE_P_H

#include "qdeclarativerefcount_p.h"
#include "qdeclarativecleanup_p.h"
#include <qscriptdeclarativeclass_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeType;
class QDeclarativeEngine;
class QDeclarativeTypeNameCache : public QDeclarativeRefCount, public QDeclarativeCleanup
{
 public:
   QDeclarativeTypeNameCache(QDeclarativeEngine *);
   virtual ~QDeclarativeTypeNameCache();

   struct Data {
      inline Data();
      inline ~Data();
      QDeclarativeType *type;
      QDeclarativeTypeNameCache *typeNamespace;
      int importedScriptIndex;
   };

   void add(const QString &, int);
   void add(const QString &, QDeclarativeType *);
   void add(const QString &, QDeclarativeTypeNameCache *);

   Data *data(const QString &) const;
   inline Data *data(const QScriptDeclarativeClass::Identifier &id) const;

 protected:
   virtual void clear();

 private:
   struct RData : public Data {
      QScriptDeclarativeClass::PersistentIdentifier identifier;
   };
   typedef QHash<QString, RData *> StringCache;
   typedef QHash<QScriptDeclarativeClass::Identifier, RData *> IdentifierCache;

   StringCache stringCache;
   IdentifierCache identifierCache;
   QDeclarativeEngine *engine;
};

QDeclarativeTypeNameCache::Data::Data()
   : type(0), typeNamespace(0), importedScriptIndex(-1)
{
}

QDeclarativeTypeNameCache::Data::~Data()
{
   if (typeNamespace) {
      typeNamespace->release();
   }
}

QDeclarativeTypeNameCache::Data *QDeclarativeTypeNameCache::data(const QScriptDeclarativeClass::Identifier &id) const
{
   return identifierCache.value(id);
}

QT_END_NAMESPACE

#endif // QDECLARATIVETYPENAMECACHE_P_H

