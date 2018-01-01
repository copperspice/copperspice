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

#ifndef QSTYLEPLUGIN_H
#define QSTYLEPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

class QStyle;

struct Q_GUI_EXPORT QStyleFactoryInterface : public QFactoryInterface {
   virtual QStyle *create(const QString &key) = 0;
};

#define QStyleFactoryInterface_iid "com.copperspice.QStyleFactoryInterface"

CS_DECLARE_INTERFACE(QStyleFactoryInterface, QStyleFactoryInterface_iid)

class Q_GUI_EXPORT QStylePlugin : public QObject, public QStyleFactoryInterface
{
   GUI_CS_OBJECT_MULTIPLE(QStylePlugin, QObject)
   CS_INTERFACES(QStyleFactoryInterface, QFactoryInterface)

 public:
   explicit QStylePlugin(QObject *parent = nullptr);
   ~QStylePlugin();

};

QT_END_NAMESPACE

#endif
