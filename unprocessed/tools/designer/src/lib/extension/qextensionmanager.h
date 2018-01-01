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

#ifndef QEXTENSIONMANAGER_H
#define QEXTENSIONMANAGER_H

#include <QtDesigner/extension_global.h>
#include <QtDesigner/extension.h>
#include <QtCore/QHash>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QObject; // Fool syncqt

class QDESIGNER_EXTENSION_EXPORT QExtensionManager: public QObject, public QAbstractExtensionManager
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionManager)
public:
    QExtensionManager(QObject *parent = 0);
    ~QExtensionManager();

    virtual void registerExtensions(QAbstractExtensionFactory *factory, const QString &iid = QString());
    virtual void unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid = QString());

    virtual QObject *extension(QObject *object, const QString &iid) const;

private:
    typedef QList<QAbstractExtensionFactory*> FactoryList;
    typedef QHash<QString, FactoryList> FactoryMap;
    FactoryMap m_extensions;
    FactoryList m_globalExtension;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QEXTENSIONMANAGER_H
