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

#ifndef QSQLDRIVERPLUGIN_H
#define QSQLDRIVERPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

class QSqlDriver;

struct Q_SQL_EXPORT QSqlDriverFactoryInterface : public QFactoryInterface {
   virtual QSqlDriver *create(const QString &name) = 0;
};

#define QSqlDriverFactoryInterface_iid "com.copperspice.QSqlDriverFactoryInterface"
CS_DECLARE_INTERFACE(QSqlDriverFactoryInterface, QSqlDriverFactoryInterface_iid)

class Q_SQL_EXPORT QSqlDriverPlugin : public QObject, public QSqlDriverFactoryInterface
{
   SQL_CS_OBJECT_MULTIPLE(QSqlDriverPlugin, QObject)
   CS_INTERFACES(QSqlDriverFactoryInterface, QFactoryInterface)

 public:
   explicit QSqlDriverPlugin(QObject *parent = nullptr);
   ~QSqlDriverPlugin();
};

QT_END_NAMESPACE

#endif // QSQLDRIVERPLUGIN_H
