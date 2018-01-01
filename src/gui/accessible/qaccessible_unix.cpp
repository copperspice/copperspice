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

#include <qaccessible.h>
#include <qaccessiblebridge.h>

#ifndef QT_NO_ACCESSIBILITY

#include <qcoreapplication.h>
#include <qmutex.h>
#include <qvector.h>
#include <qfactoryloader_p.h>

#include <stdlib.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
                          (QAccessibleBridgeFactoryInterface_iid, QLatin1String("/accessiblebridge")))

Q_GLOBAL_STATIC(QVector<QAccessibleBridge *>, bridges)
static bool isInit = false;

void QAccessible::initialize()
{
   if (isInit) {
      return;
   }
   isInit = true;

   if (qgetenv("QT_ACCESSIBILITY") != "1") {
      return;
   }

   const QStringList l = loader()->keys();
   for (int i = 0; i < l.count(); ++i) {
      if (QAccessibleBridgeFactoryInterface *factory =
               qobject_cast<QAccessibleBridgeFactoryInterface *>(loader()->instance(l.at(i)))) {
         QAccessibleBridge *bridge = factory->create(l.at(i));
         if (bridge) {
            bridges()->append(bridge);
         }
      }
   }

}

void QAccessible::cleanup()
{
   qDeleteAll(*bridges());
}

void QAccessible::updateAccessibility(QObject *o, int who, Event reason)
{
   Q_ASSERT(o);

   if (updateHandler) {
      updateHandler(o, who, reason);
      return;
   }

   initialize();
   if (!bridges() || bridges()->isEmpty()) {
      return;
   }

   QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(o);
   if (!iface) {
      return;
   }

   // updates for List/Table/Tree should send child
   if (who) {
      QAccessibleInterface *child;
      iface->navigate(QAccessible::Child, who, &child);
      if (child) {
         delete iface;
         iface = child;
         who = 0;
      }
   }

   for (int i = 0; i < bridges()->count(); ++i) {
      bridges()->at(i)->notifyAccessibilityUpdate(reason, iface, who);
   }
   delete iface;
}

void QAccessible::setRootObject(QObject *o)
{
   if (rootObjectHandler) {
      rootObjectHandler(o);
      return;
   }

   initialize();
   if (bridges()->isEmpty()) {
      return;
   }

   if (!o) {
      return;
   }

   for (int i = 0; i < bridges()->count(); ++i) {
      QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(o);
      bridges()->at(i)->setRootObject(iface);
   }
}

QT_END_NAMESPACE

#endif // QT_NO_ACCESSIBILITY

