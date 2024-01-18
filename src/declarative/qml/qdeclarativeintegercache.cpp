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

#include "private/qdeclarativeintegercache_p.h"

#include "private/qdeclarativeengine_p.h"
#include "private/qdeclarativemetatype_p.h"

QT_BEGIN_NAMESPACE

QDeclarativeIntegerCache::QDeclarativeIntegerCache(QDeclarativeEngine *e)
   : QDeclarativeCleanup(e), engine(e)
{
}

QDeclarativeIntegerCache::~QDeclarativeIntegerCache()
{
   clear();
}

void QDeclarativeIntegerCache::clear()
{
   qDeleteAll(stringCache);
   stringCache.clear();
   identifierCache.clear();
   engine = 0;
}

QString QDeclarativeIntegerCache::findId(int value) const
{
   for (StringCache::ConstIterator iter = stringCache.begin();
         iter != stringCache.end(); ++iter) {
      if (iter.value() && iter.value()->value == value) {
         return iter.key();
      }
   }
   return QString();
}

void QDeclarativeIntegerCache::add(const QString &id, int value)
{
   Q_ASSERT(!stringCache.contains(id));

   QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(engine);

   // ### use contextClass
   Data *d = new Data(enginePriv->objectClass->createPersistentIdentifier(id), value);

   stringCache.insert(id, d);
   identifierCache.insert(d->identifier, d);
}

int QDeclarativeIntegerCache::value(const QString &id)
{
   Data *d = stringCache.value(id);
   return d ? d->value : -1;
}

QT_END_NAMESPACE
