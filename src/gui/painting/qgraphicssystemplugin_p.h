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

#ifndef QGRAPHICSSYSTEMPLUGIN_P_H
#define QGRAPHICSSYSTEMPLUGIN_P_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

class QGraphicsSystem;

struct QGraphicsSystemFactoryInterface : public QFactoryInterface {
   virtual QGraphicsSystem *create(const QString &key) = 0;
};

#define QGraphicsSystemFactoryInterface_iid "com.copperspice.QGraphicsSystemFactoryInterface"

CS_DECLARE_INTERFACE(QGraphicsSystemFactoryInterface, QGraphicsSystemFactoryInterface_iid)

class Q_GUI_EXPORT QGraphicsSystemPlugin : public QObject, public QGraphicsSystemFactoryInterface
{
   GUI_CS_OBJECT_MULTIPLE(QGraphicsSystemPlugin, QObject)
   CS_INTERFACES(QGraphicsSystemFactoryInterface, QFactoryInterface)

 public:
   explicit QGraphicsSystemPlugin(QObject *parent = nullptr);
   ~QGraphicsSystemPlugin();   
};

QT_END_NAMESPACE

#endif
