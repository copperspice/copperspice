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

#include <qaccessiblecache_p.h>

#ifndef QT_NO_ACCESSIBILITY

static QAccessibleCache *qAccessibleCache()
{
   static QAccessibleCache retval;
   return &retval;
}

QAccessibleCache *QAccessibleCache::instance()
{
   return qAccessibleCache();
}

/*
  The ID is always in the range [INT_MAX+1, UINT_MAX].
  This makes it easy on windows to reserve the positive integer range
  for the index of a child and not clash with the unique ids.
*/
QAccessible::Id QAccessibleCache::acquireId() const
{
   static constexpr const QAccessible::Id FirstId = QAccessible::Id(INT_MAX) + 1;

   static QAccessible::Id lastUsedId = FirstId;

   while (idToInterface.contains(lastUsedId)) {
      // (wrap back when when we reach UINT_MAX - 1)
      // -1 because on Android -1 is taken for the "View" so just avoid it completely for consistency
      if (lastUsedId == UINT_MAX - 1) {
         lastUsedId = FirstId;
      } else {
         ++lastUsedId;
      }
   }

   return lastUsedId;
}

QAccessibleInterface *QAccessibleCache::interfaceForId(QAccessible::Id id) const
{
   return idToInterface.value(id);
}

QAccessible::Id QAccessibleCache::idForInterface(QAccessibleInterface *iface) const
{
   return interfaceToId.value(iface);
}

QAccessible::Id QAccessibleCache::insert(QObject *object, QAccessibleInterface *iface) const
{
   Q_ASSERT(iface);

   Q_ASSERT(! objectToId.contains(object));
   Q_ASSERT_X(! interfaceToId.contains(iface), "", "Accessible interface inserted into cache twice");

   QAccessible::Id id = acquireId();
   QObject *obj = iface->object();

   Q_ASSERT(object == obj);

   if (obj) {
      objectToId.insert(obj, id);
      connect(obj, &QObject::destroyed, this, &QAccessibleCache::objectDestroyed);
   }

   idToInterface.insert(id, iface);
   interfaceToId.insert(iface, id);

   return id;
}

void QAccessibleCache::objectDestroyed(QObject *obj)
{
   QAccessible::Id id = objectToId.value(obj);

   if (id) {
      Q_ASSERT_X(idToInterface.contains(id), "", "QObject with accessible interface deleted");
      deleteInterface(id, obj);
   }
}

void QAccessibleCache::deleteInterface(QAccessible::Id id, QObject *obj)
{
   QAccessibleInterface *iface = idToInterface.take(id);
   interfaceToId.take(iface);
   if (!obj) {
      obj = iface->object();
   }
   if (obj) {
      objectToId.remove(obj);
   }
   delete iface;

#ifdef Q_OS_DARWIN
   removeCocoaElement(id);
#endif
}


#endif
