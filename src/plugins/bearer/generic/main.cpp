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

#include <qgenericengine.h>
#include <qbearerplugin_p.h>
#include <qdebug.h>

#ifndef QT_NO_BEARERMANAGEMENT

QT_BEGIN_NAMESPACE

class QGenericEnginePlugin : public QBearerEnginePlugin
{
public:
    QGenericEnginePlugin();
    ~QGenericEnginePlugin();

    QStringList keys() const;
    QBearerEngine *create(const QString &key) const;
};

QGenericEnginePlugin::QGenericEnginePlugin()
{
}

QGenericEnginePlugin::~QGenericEnginePlugin()
{
}

QStringList QGenericEnginePlugin::keys() const
{
    return QStringList() << QLatin1String("generic");
}

QBearerEngine *QGenericEnginePlugin::create(const QString &key) const
{
    if (key == QLatin1String("generic"))
        return new QGenericEngine;
    else
        return 0;
}

Q_EXPORT_STATIC_PLUGIN(QGenericEnginePlugin)
Q_EXPORT_PLUGIN2(qgenericbearer, QGenericEnginePlugin)

QT_END_NAMESPACE

#endif // QT_NO_BEARERMANAGEMENT
