/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
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
   explicit QGraphicsSystemPlugin(QObject *parent = 0);
   ~QGraphicsSystemPlugin();

   virtual QStringList keys() const override = 0;
   virtual QGraphicsSystem *create(const QString &key) override = 0;
};

QT_END_NAMESPACE

#endif // QGRAPHICSSYSTEMEPLUGIN_H
