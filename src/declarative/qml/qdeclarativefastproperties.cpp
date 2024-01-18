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

#include "private/qdeclarativefastproperties_p.h"

#include <private/qdeclarativeitem_p.h>

QT_BEGIN_NAMESPACE

// Adding entries to the QDeclarativeFastProperties class allows the QML
// binding optimizer to bypass Qt's meta system and read and, more
// importantly, subscribe to properties directly.  Any property that is
// primarily read from bindings is a candidate for inclusion as a fast
// property.

static void QObject_objectName(QObject *object, void *output, QDeclarativeNotifierEndpoint *endpoint)
{
   if (endpoint) {
      endpoint->connect(QDeclarativeData::get(object, true)->objectNameNotifier());
   }
   *((QString *)output) = object->objectName();
}

QDeclarativeFastProperties::QDeclarativeFastProperties()
{
   add(&QDeclarativeItem::staticMetaObject, QDeclarativeItem::staticMetaObject.indexOfProperty("parent"),
       QDeclarativeItemPrivate::parentProperty);
   add(&QObject::staticMetaObject, QObject::staticMetaObject.indexOfProperty("objectName"),
       QObject_objectName);
}

int QDeclarativeFastProperties::accessorIndexForProperty(const QMetaObject *metaObject, int propertyIndex)
{
   Q_ASSERT(metaObject);
   Q_ASSERT(propertyIndex >= 0);

   // Find the "real" metaObject
   while (metaObject->propertyOffset() > propertyIndex) {
      metaObject = metaObject->superClass();
   }

   QHash<QPair<const QMetaObject *, int>, int>::Iterator iter =
      m_index.find(qMakePair(metaObject, propertyIndex));
   if (iter != m_index.end()) {
      return *iter;
   } else {
      return -1;
   }
}

void QDeclarativeFastProperties::add(const QMetaObject *metaObject, int propertyIndex, Accessor accessor)
{
   Q_ASSERT(metaObject);
   Q_ASSERT(propertyIndex >= 0);

   // Find the "real" metaObject
   while (metaObject->propertyOffset() > propertyIndex) {
      metaObject = metaObject->superClass();
   }

   QPair<const QMetaObject *, int> data = qMakePair(metaObject, propertyIndex);
   int accessorIndex = m_accessors.count();
   m_accessors.append(accessor);
   m_index.insert(data, accessorIndex);
}

QT_END_NAMESPACE
