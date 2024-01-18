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

#include <qplatform_accessibility.h>

#include <qalgorithms.h>
#include <qaccessibleplugin.h>
#include <qaccessibleobject.h>
#include <qaccessiblebridge.h>
#include <qapplication.h>
#include <qdebug.h>

#include <qfactoryloader_p.h>

#ifndef QT_NO_ACCESSIBILITY

static QFactoryLoader *bridgeloader()
{
   static QFactoryLoader retval(QAccessibleBridgeInterface_ID, "/accessiblebridge");
   return &retval;
}

static QVector<QAccessibleBridge *> *bridges()
{
   static QVector<QAccessibleBridge *> retval;
   return &retval;
}

QPlatformAccessibility::QPlatformAccessibility()
   : m_active(false)
{
}

QPlatformAccessibility::~QPlatformAccessibility()
{
}

void QPlatformAccessibility::notifyAccessibilityUpdate(QAccessibleEvent *event)
{
   initialize();

   if (!bridges() || bridges()->isEmpty()) {
      return;
   }

   for (int i = 0; i < bridges()->count(); ++i) {
      bridges()->at(i)->notifyAccessibilityUpdate(event);
   }
}

void QPlatformAccessibility::setRootObject(QObject *o)
{
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

void QPlatformAccessibility::initialize()
{
   static bool isInit = false;
   if (isInit) {
      return;
   }

   isInit = true;      // not atomic

   auto keySet = bridgeloader()->keySet();

   QAccessibleBridgePlugin *factory = nullptr;
   QSet<QAccessibleBridgePlugin *> usedSet;

   for (auto item : keySet) {
      factory = qobject_cast<QAccessibleBridgePlugin *>(bridgeloader()->instance(item));

      if (factory != nullptr && ! usedSet.contains(factory)) {
         usedSet.insert(factory);

         if (QAccessibleBridge *bridge = factory->create(item)) {
            bridges()->append(bridge);
         }
      }
   }
}

void QPlatformAccessibility::cleanup()
{
   qDeleteAll(*bridges());
}

void QPlatformAccessibility::setActive(bool active)
{
   m_active = active;
   QAccessible::setActive(active);
}

#endif // QT_NO_ACCESSIBILITY
