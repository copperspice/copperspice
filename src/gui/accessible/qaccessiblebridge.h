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

#ifndef QACCESSIBLEBRIDGE_H
#define QACCESSIBLEBRIDGE_H

#include <qplugin.h>
#include <qfactoryinterface.h>

#ifndef QT_NO_ACCESSIBILITY

class QAccessibleInterface;
class QAccessibleEvent;

class QAccessibleBridge
{
 public:
   virtual ~QAccessibleBridge()
   {
   }

   virtual void setRootObject(QAccessibleInterface *object) = 0;
   virtual void notifyAccessibilityUpdate(QAccessibleEvent *event) = 0;
};

#define QAccessibleBridgeInterface_ID "com.copperspice.CS.AccessibleBridgeInterface"

class Q_GUI_EXPORT QAccessibleBridgePlugin : public QObject
{
   GUI_CS_OBJECT(QAccessibleBridgePlugin)

 public:
   explicit QAccessibleBridgePlugin(QObject *parent = nullptr);
   ~QAccessibleBridgePlugin();

   virtual QAccessibleBridge *create(const QString &key) = 0;
};

#endif

#endif
