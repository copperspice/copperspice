/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#ifndef QDBUSXMLPARSER_H
#define QDBUSXMLPARSER_H

#include <QtCore/qmap.h>
#include <QtXml/qdom.h>
#include <qdbusmacros.h>
#include "qdbusintrospection_p.h"

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QDBusXmlParser
{
    QString m_service;
    QString m_path;
    QDomElement m_node;

public:
    QDBusXmlParser(const QString & service, const QString& path, const QString & xmlData);
    QDBusXmlParser(const QString & service, const QString& path, const QDomElement& node);

    QDBusIntrospection::Interfaces interfaces() const;
    QSharedDataPointer<QDBusIntrospection::Object> object() const;
    QSharedDataPointer<QDBusIntrospection::ObjectTree> objectTree() const;
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif
