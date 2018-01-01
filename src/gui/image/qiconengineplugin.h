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

#ifndef QICONENGINEPLUGIN_H
#define QICONENGINEPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

class QIconEngine;
class QIconEngineV2;

struct Q_GUI_EXPORT QIconEngineFactoryInterface : public QFactoryInterface {
   virtual QIconEngine *create(const QString &filename) = 0;
};

#define QIconEngineFactoryInterface_iid "com.copperspice.QIconEngineFactoryInterface"
CS_DECLARE_INTERFACE(QIconEngineFactoryInterface, QIconEngineFactoryInterface_iid)

class Q_GUI_EXPORT QIconEnginePlugin : public QObject, public QIconEngineFactoryInterface
{
   GUI_CS_OBJECT_MULTIPLE(QIconEnginePlugin, QObject)
   CS_INTERFACES(QIconEngineFactoryInterface, QFactoryInterface)

 public:
   QIconEnginePlugin(QObject *parent = nullptr);
   ~QIconEnginePlugin();
};

// ### Qt5/remove version 2
struct Q_GUI_EXPORT QIconEngineFactoryInterfaceV2 : public QFactoryInterface {
   virtual QIconEngineV2 *create(const QString &filename = QString()) = 0;
};

#define QIconEngineFactoryInterfaceV2_iid "com.copperspice.QIconEngineFactoryInterfaceV2"
CS_DECLARE_INTERFACE(QIconEngineFactoryInterfaceV2, QIconEngineFactoryInterfaceV2_iid)

class Q_GUI_EXPORT QIconEnginePluginV2 : public QObject, public QIconEngineFactoryInterfaceV2
{
   GUI_CS_OBJECT_MULTIPLE(QIconEnginePluginV2, QObject)
   CS_INTERFACES(QIconEngineFactoryInterfaceV2, QFactoryInterface)

 public:
   QIconEnginePluginV2(QObject *parent = nullptr);
   ~QIconEnginePluginV2();
};

QT_END_NAMESPACE

#endif // QICONENGINEPLUGIN_H
