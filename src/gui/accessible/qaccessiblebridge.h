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

#ifndef QACCESSIBLEBRIDGE_H
#define QACCESSIBLEBRIDGE_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ACCESSIBILITY

class QAccessibleInterface;

class QAccessibleBridge
{
 public:
   virtual ~QAccessibleBridge() {}
   virtual void setRootObject(QAccessibleInterface *) = 0;
   virtual void notifyAccessibilityUpdate(int, QAccessibleInterface *, int) = 0;
};

struct Q_GUI_EXPORT QAccessibleBridgeFactoryInterface : public QFactoryInterface {
   virtual QAccessibleBridge *create(const QString &name) = 0;
};

#define QAccessibleBridgeFactoryInterface_iid "com.copperspice.QAccessibleBridgeFactoryInterface"
CS_DECLARE_INTERFACE(QAccessibleBridgeFactoryInterface, QAccessibleBridgeFactoryInterface_iid)

class Q_GUI_EXPORT QAccessibleBridgePlugin : public QObject, public QAccessibleBridgeFactoryInterface
{
   GUI_CS_OBJECT(QAccessibleBridgePlugin)
   CS_INTERFACES(QAccessibleBridgeFactoryInterface, QFactoryInterface)

 public:
   explicit QAccessibleBridgePlugin(QObject *parent = nullptr);
   ~QAccessibleBridgePlugin();
};

#endif // QT_NO_ACCESSIBILITY

QT_END_NAMESPACE

#endif // QACCESSIBLEBRIDGE_H
