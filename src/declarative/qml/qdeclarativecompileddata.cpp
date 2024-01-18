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

#include <qdeclarativecompiler_p.h>
#include <qdeclarativeengine.h>
#include <qdeclarativecomponent.h>
#include <qdeclarativecomponent_p.h>
#include <qdeclarativecontext.h>
#include <qdeclarativecontext_p.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

int QDeclarativeCompiledData::pack(const char *data, size_t size)
{
   const char *p = packData.constData();
   unsigned int ps = packData.size();

   for (unsigned int ii = 0; (ii + size) <= ps; ii += sizeof(int)) {
      if (0 == ::memcmp(p + ii, data, size)) {
         return ii;
      }
   }

   int rv = packData.size();
   packData.append(data, size);
   return rv;
}

int QDeclarativeCompiledData::indexForString(const QString &data)
{
   int idx = primitives.indexOf(data);
   if (idx == -1) {
      idx = primitives.count();
      primitives << data;
   }
   return idx;
}

int QDeclarativeCompiledData::indexForByteArray(const QByteArray &data)
{
   int idx = datas.indexOf(data);
   if (idx == -1) {
      idx = datas.count();
      datas << data;
   }
   return idx;
}

int QDeclarativeCompiledData::indexForUrl(const QUrl &data)
{
   int idx = urls.indexOf(data);
   if (idx == -1) {
      idx = urls.count();
      urls << data;
   }
   return idx;
}

int QDeclarativeCompiledData::indexForFloat(float *data, int count)
{
   Q_ASSERT(count > 0);

   for (int ii = 0; ii <= floatData.count() - count; ++ii) {
      bool found = true;
      for (int jj = 0; jj < count; ++jj) {
         if (floatData.at(ii + jj) != data[jj]) {
            found = false;
            break;
         }
      }

      if (found) {
         return ii;
      }
   }

   int idx = floatData.count();
   for (int ii = 0; ii < count; ++ii) {
      floatData << data[ii];
   }

   return idx;
}

int QDeclarativeCompiledData::indexForInt(int *data, int count)
{
   Q_ASSERT(count > 0);

   for (int ii = 0; ii <= intData.count() - count; ++ii) {
      bool found = true;
      for (int jj = 0; jj < count; ++jj) {
         if (intData.at(ii + jj) != data[jj]) {
            found = false;
            break;
         }
      }

      if (found) {
         return ii;
      }
   }

   int idx = intData.count();
   for (int ii = 0; ii < count; ++ii) {
      intData << data[ii];
   }

   return idx;
}

int QDeclarativeCompiledData::indexForLocation(const QDeclarativeParser::Location &l)
{
   // ### FIXME
   int rv = locations.count();
   locations << l;
   return rv;
}

int QDeclarativeCompiledData::indexForLocation(const QDeclarativeParser::LocationSpan &l)
{
   // ### FIXME
   int rv = locations.count();
   locations << l.start << l.end;
   return rv;
}

QDeclarativeCompiledData::QDeclarativeCompiledData(QDeclarativeEngine *engine)
   : QDeclarativeCleanup(engine), importCache(0), root(0), rootPropertyCache(0)
{
}

QDeclarativeCompiledData::~QDeclarativeCompiledData()
{
   for (int ii = 0; ii < types.count(); ++ii) {
      if (types.at(ii).component) {
         types.at(ii).component->release();
      }
      if (types.at(ii).typePropertyCache) {
         types.at(ii).typePropertyCache->release();
      }
   }

   for (int ii = 0; ii < propertyCaches.count(); ++ii) {
      propertyCaches.at(ii)->release();
   }

   for (int ii = 0; ii < contextCaches.count(); ++ii) {
      contextCaches.at(ii)->release();
   }

   if (importCache) {
      importCache->release();
   }

   if (rootPropertyCache) {
      rootPropertyCache->release();
   }

   qDeleteAll(cachedPrograms);
   qDeleteAll(cachedClosures);
}

void QDeclarativeCompiledData::clear()
{
   qDeleteAll(cachedPrograms);
   qDeleteAll(cachedClosures);
   for (int ii = 0; ii < cachedClosures.count(); ++ii) {
      cachedClosures[ii] = 0;
   }
   for (int ii = 0; ii < cachedPrograms.count(); ++ii) {
      cachedPrograms[ii] = 0;
   }
}

const QMetaObject *QDeclarativeCompiledData::TypeReference::metaObject() const
{
   if (type) {
      return type->metaObject();
   } else {
      Q_ASSERT(component);
      return component->root;
   }
}

/*!
Returns the property cache, if one alread exists.  The cache is not referenced.
*/
QDeclarativePropertyCache *QDeclarativeCompiledData::TypeReference::propertyCache() const
{
   if (type) {
      return typePropertyCache;
   } else {
      return component->rootPropertyCache;
   }
}

/*!
Returns the property cache, creating one if it doesn't already exist.  The cache is not referenced.
*/
QDeclarativePropertyCache *QDeclarativeCompiledData::TypeReference::createPropertyCache(QDeclarativeEngine *engine)
{
   if (typePropertyCache) {
      return typePropertyCache;
   } else if (type) {
      typePropertyCache = QDeclarativeEnginePrivate::get(engine)->cache(type->metaObject());
      typePropertyCache->addref();
      return typePropertyCache;
   } else {
      return component->rootPropertyCache;
   }
}


void QDeclarativeCompiledData::dumpInstructions()
{
   if (!name.isEmpty()) {
      qWarning() << name;
   }
   qWarning().nospace() << "Index\tLine\tOperation\t\tData1\tData2\tData3\tComments";
   qWarning().nospace() << "-------------------------------------------------------------------------------";
   for (int ii = 0; ii < bytecode.count(); ++ii) {
      dump(&bytecode[ii], ii);
   }
   qWarning().nospace() << "-------------------------------------------------------------------------------";
}


QT_END_NAMESPACE
