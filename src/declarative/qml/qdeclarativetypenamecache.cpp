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

#include <qdeclarativetypenamecache_p.h>
#include <qdeclarativeengine_p.h>

QT_BEGIN_NAMESPACE

QDeclarativeTypeNameCache::QDeclarativeTypeNameCache(QDeclarativeEngine *e)
   : QDeclarativeCleanup(e), engine(e)
{
}

QDeclarativeTypeNameCache::~QDeclarativeTypeNameCache()
{
   clear();
}

void QDeclarativeTypeNameCache::clear()
{
   qDeleteAll(stringCache);
   stringCache.clear();
   identifierCache.clear();
   engine = 0;
}

void QDeclarativeTypeNameCache::add(const QString &name, int importedScriptIndex)
{
   if (stringCache.contains(name)) {
      return;
   }

   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);

   RData *data = new RData;
   // ### Use typename class
   data->identifier = ep->objectClass->createPersistentIdentifier(name);
   data->importedScriptIndex = importedScriptIndex;
   stringCache.insert(name, data);
   identifierCache.insert(data->identifier.identifier, data);
}

void QDeclarativeTypeNameCache::add(const QString &name, QDeclarativeType *type)
{
   if (stringCache.contains(name)) {
      return;
   }

   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);

   RData *data = new RData;
   // ### Use typename class
   data->identifier = ep->objectClass->createPersistentIdentifier(name);
   data->type = type;
   stringCache.insert(name, data);
   identifierCache.insert(data->identifier.identifier, data);
}

void QDeclarativeTypeNameCache::add(const QString &name, QDeclarativeTypeNameCache *typeNamespace)
{
   if (stringCache.contains(name)) {
      return;
   }

   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);

   RData *data = new RData;
   // ### Use typename class
   data->identifier = ep->objectClass->createPersistentIdentifier(name);
   data->typeNamespace = typeNamespace;
   stringCache.insert(name, data);
   identifierCache.insert(data->identifier.identifier, data);
   typeNamespace->addref();
}

QDeclarativeTypeNameCache::Data *QDeclarativeTypeNameCache::data(const QString &id) const
{
   return stringCache.value(id);
}

QT_END_NAMESPACE

