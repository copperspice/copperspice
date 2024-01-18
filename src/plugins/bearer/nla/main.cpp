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

#include <qnlaengine.h>
#include <qbearerplugin_p.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

class QNlaEnginePlugin : public QBearerEnginePlugin
{
public:
    QNlaEnginePlugin();
    ~QNlaEnginePlugin();

    QStringList keys() const;
    QBearerEngine *create(const QString &key) const;
};

QNlaEnginePlugin::QNlaEnginePlugin()
{
}

QNlaEnginePlugin::~QNlaEnginePlugin()
{
}

QStringList QNlaEnginePlugin::keys() const
{
    return QStringList() << QLatin1String("nla");
}

QBearerEngine *QNlaEnginePlugin::create(const QString &key) const
{
    if (key == QLatin1String("nla"))
        return new QNlaEngine;
    else
        return 0;
}

Q_EXPORT_STATIC_PLUGIN(QNlaEnginePlugin)
Q_EXPORT_PLUGIN2(qnlabearer, QNlaEnginePlugin)

QT_END_NAMESPACE
