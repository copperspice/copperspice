/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QACCESSIBLECACHE_P
#define QACCESSIBLECACHE_P

#include <qglobal.h>
#include <qobject.h>
#include <qhash.h>

#include <qaccessible.h>

#ifndef QT_NO_ACCESSIBILITY

#ifdef __OBJC__
@class QMacAccessibilityElement;

#else
using QMacAccessibilityElement = struct objc_object;

#endif

class Q_GUI_EXPORT QAccessibleCache : public QObject
{
   GUI_CS_OBJECT(QAccessibleCache)

 public:
   static QAccessibleCache *instance();
   QAccessibleInterface *interfaceForId(QAccessible::Id id) const;
   QAccessible::Id idForInterface(QAccessibleInterface *iface) const;
   QAccessible::Id insert(QObject *object, QAccessibleInterface *iface) const;
   void deleteInterface(QAccessible::Id id, QObject *obj = 0);

#ifdef Q_OS_MAC
   QMacAccessibilityElement *elementForId(QAccessible::Id axid) const;
   void insertElement(QAccessible::Id axid, QMacAccessibilityElement *element) const;
#endif

 private:
   GUI_CS_SLOT_1(Private, void objectDestroyed(QObject *obj))
   GUI_CS_SLOT_2(objectDestroyed)

   QAccessible::Id acquireId() const;

   mutable QHash<QAccessible::Id, QAccessibleInterface *> idToInterface;
   mutable QHash<QAccessibleInterface *, QAccessible::Id> interfaceToId;
   mutable QHash<QObject *, QAccessible::Id> objectToId;

#ifdef Q_OS_MAC
   void removeCocoaElement(QAccessible::Id axid);
   mutable QHash<QAccessible::Id, QMacAccessibilityElement *> cocoaElements;
#endif

   friend class QAccessible;
   friend class QAccessibleInterface;
};

#endif // QT_NO_ACCESSIBILITY

#endif
