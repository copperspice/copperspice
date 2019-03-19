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

#ifndef QSCREENDRIVERPLUGIN_QWS_H
#define QSCREENDRIVERPLUGIN_QWS_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

class QScreen;

struct Q_GUI_EXPORT QScreenDriverFactoryInterface : public QFactoryInterface {
   virtual QScreen *create(const QString &driver, int displayId) = 0;
};

#define QScreenDriverFactoryInterface_iid "com.copperspice.QScreenDriverFactoryInterface"
CS_DECLARE_INTERFACE(QScreenDriverFactoryInterface, QScreenDriverFactoryInterface_iid)

class Q_GUI_EXPORT QScreenDriverPlugin : public QObject, public QScreenDriverFactoryInterface
{
   GUI_CS_OBJECT(QScreenDriverPlugin)
   CS_INTERFACES(QScreenDriverFactoryInterface, QFactoryInterface)

 public:
   explicit QScreenDriverPlugin(QObject *parent = nullptr);
   ~QScreenDriverPlugin();

   virtual QStringList keys() const = 0;
   virtual QScreen *create(const QString &driver, int displayId) = 0;
};

QT_END_NAMESPACE

#endif // QSCREENDRIVERPLUGIN_QWS_H
