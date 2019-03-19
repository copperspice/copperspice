/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QMOUSEDRIVERPLUGIN_QWS_H
#define QMOUSEDRIVERPLUGIN_QWS_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

class QWSMouseHandler;

struct Q_GUI_EXPORT QWSMouseHandlerFactoryInterface : public QFactoryInterface {
   virtual QWSMouseHandler *create(const QString &name, const QString &device) = 0;
};

#define QWSMouseHandlerFactoryInterface_iid "com.copperspice.QWSMouseHandlerFactoryInterface"
CS_DECLARE_INTERFACE(QWSMouseHandlerFactoryInterface, QWSMouseHandlerFactoryInterface_iid)

class Q_GUI_EXPORT QMouseDriverPlugin : public QObject, public QWSMouseHandlerFactoryInterface
{
   GUI_CS_OBJECT_MULTIPLE(QMouseDriverPlugin, QObject)
   CS_INTERFACES(QWSMouseHandlerFactoryInterface, QFactoryInterface)

 public:
   explicit QMouseDriverPlugin(QObject *parent = nullptr);
   ~QMouseDriverPlugin();

   virtual QStringList keys() const = 0;
   virtual QWSMouseHandler *create(const QString &driver, const QString &device) = 0;
};

QT_END_NAMESPACE

#endif // QMOUSEDRIVERPLUGIN_QWS_H
