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

#ifndef QDECORATIONPLUGIN_QWS_H
#define QDECORATIONPLUGIN_QWS_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

class QDecoration;

struct Q_GUI_EXPORT QDecorationFactoryInterface : public QFactoryInterface {
   virtual QDecoration *create(const QString &key) = 0;
};

#define QDecorationFactoryInterface_iid "com.copperspice.QDecorationFactoryInterface"
CS_DECLARE_INTERFACE(QDecorationFactoryInterface, QDecorationFactoryInterface_iid)

class Q_GUI_EXPORT QDecorationPlugin : public QObject, public QDecorationFactoryInterface
{
   GUI_CS_OBJECT_MULTIPLE(QDecorationPlugin, QObject)
   CS_INTERFACES(QDecorationFactoryInterface, QFactoryInterface)

 public:
   explicit QDecorationPlugin(QObject *parent = nullptr);
   ~QDecorationPlugin();

   virtual QStringList keys() const = 0;
   virtual QDecoration *create(const QString &key) = 0;
};

QT_END_NAMESPACE

#endif // QDECORATIONPLUGIN_QWS_H
