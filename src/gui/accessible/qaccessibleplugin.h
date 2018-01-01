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

#ifndef QACCESSIBLEPLUGIN_H
#define QACCESSIBLEPLUGIN_H

#include <QtGui/qaccessible.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ACCESSIBILITY

class QStringList;
class QAccessibleInterface;

struct Q_GUI_EXPORT QAccessibleFactoryInterface : public QAccessible, public QFactoryInterface {
   virtual QAccessibleInterface *create(const QString &key, QObject *object) = 0;
};

#define QAccessibleFactoryInterface_iid "com.copperspice.QAccessibleFactoryInterface"
CS_DECLARE_INTERFACE(QAccessibleFactoryInterface, QAccessibleFactoryInterface_iid)

class QAccessiblePluginPrivate;

class Q_GUI_EXPORT QAccessiblePlugin : public QObject, public QAccessibleFactoryInterface
{
   GUI_CS_OBJECT_MULTIPLE(QAccessiblePlugin, QObject)
   CS_INTERFACES(QAccessibleFactoryInterface, QFactoryInterface)

 public:
   explicit QAccessiblePlugin(QObject *parent = nullptr);
   ~QAccessiblePlugin();  
};

#endif // QT_NO_ACCESSIBILITY

QT_END_NAMESPACE

#endif // QACCESSIBLEPLUGIN_H
