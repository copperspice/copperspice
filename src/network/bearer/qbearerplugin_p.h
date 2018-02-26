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

#ifndef QBEARERPLUGIN_P_H
#define QBEARERPLUGIN_P_H

#include <qbearerengine_p.h>

#include <qplugin.h>
#include <qfactoryinterface.h>

#ifndef QT_NO_BEARERMANAGEMENT

struct Q_NETWORK_EXPORT QBearerEngineFactoryInterface : public QFactoryInterface {
   virtual QBearerEngine *create(const QString &key) const = 0;
};

#define QBearerEngineFactoryInterface_iid "com.copperspice.QBearerEngineFactoryInterface"
CS_DECLARE_INTERFACE(QBearerEngineFactoryInterface, QBearerEngineFactoryInterface_iid)

class Q_NETWORK_EXPORT QBearerEnginePlugin : public QObject, public QBearerEngineFactoryInterface
{
   NET_CS_OBJECT_MULTIPLE(QBearerEnginePlugin, QObject)
   CS_INTERFACES(QBearerEngineFactoryInterface, QFactoryInterface)

 public:
   explicit QBearerEnginePlugin(QObject *parent = nullptr);
   virtual ~QBearerEnginePlugin();
   virtual QBearerEngine *create(const QString &key) const = 0;
};

#endif // QT_NO_BEARERMANAGEMENT

#endif // QBEARERPLUGIN_P_H
