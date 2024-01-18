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

#include <qcorewlanengine.h>
#include <qbearerplugin_p.h>
#include <qdebug.h>

#ifndef QT_NO_BEARERMANAGEMENT

QT_BEGIN_NAMESPACE

class QCoreWlanEnginePlugin : public QBearerEnginePlugin
{
public:
    QCoreWlanEnginePlugin();
    ~QCoreWlanEnginePlugin();

    QStringList keys() const;
    QBearerEngine *create(const QString &key) const;
};

QCoreWlanEnginePlugin::QCoreWlanEnginePlugin()
{
}

QCoreWlanEnginePlugin::~QCoreWlanEnginePlugin()
{
}

QStringList QCoreWlanEnginePlugin::keys() const
{
    return QStringList() << QLatin1String("corewlan");
}

QBearerEngine *QCoreWlanEnginePlugin::create(const QString &key) const
{
    if (key == QLatin1String("corewlan"))
        return new QCoreWlanEngine;
    else
        return 0;
}

Q_EXPORT_STATIC_PLUGIN(QCoreWlanEnginePlugin)
Q_EXPORT_PLUGIN2(qcorewlanbearer, QCoreWlanEnginePlugin)

QT_END_NAMESPACE

#endif // QT_NO_BEARERMANAGEMENT
