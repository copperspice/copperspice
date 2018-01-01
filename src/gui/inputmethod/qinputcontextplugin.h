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

#ifndef QINPUTCONTEXTPLUGIN_H
#define QINPUTCONTEXTPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_IM)

class QInputContext;
class QInputContextPluginPrivate;

struct Q_GUI_EXPORT QInputContextFactoryInterface : public QFactoryInterface {
   virtual QInputContext *create( const QString &key ) = 0;
   virtual QStringList languages( const QString &key ) = 0;
   virtual QString displayName( const QString &key ) = 0;
   virtual QString description( const QString &key ) = 0;
};

#define QInputContextFactoryInterface_iid "com.copperspice.QInputContextFactoryInterface"
CS_DECLARE_INTERFACE(QInputContextFactoryInterface, QInputContextFactoryInterface_iid)

class Q_GUI_EXPORT QInputContextPlugin : public QObject, public QInputContextFactoryInterface
{
   GUI_CS_OBJECT(QInputContextPlugin)
   CS_INTERFACES(QInputContextFactoryInterface, QFactoryInterface)

 public:
   explicit QInputContextPlugin(QObject *parent = nullptr);
   ~QInputContextPlugin();
};

#endif // QT_NO_IM

QT_END_NAMESPACE

#endif // QINPUTCONTEXTPLUGIN_H
