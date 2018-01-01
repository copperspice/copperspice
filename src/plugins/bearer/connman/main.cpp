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

#include <qconnmanengine.h>
#include <qbearerplugin_p.h>
#include <QtCore/qdebug.h>

#ifndef QT_NO_BEARERMANAGEMENT
#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QConnmanEnginePlugin : public QBearerEnginePlugin
{
public:
    QConnmanEnginePlugin();
    ~QConnmanEnginePlugin();

    QStringList keys() const;
    QBearerEngine *create(const QString &key) const;
};

QConnmanEnginePlugin::QConnmanEnginePlugin()
{
}

QConnmanEnginePlugin::~QConnmanEnginePlugin()
{
}

QStringList QConnmanEnginePlugin::keys() const
{
    return QStringList() << QLatin1String("connman");
}

QBearerEngine *QConnmanEnginePlugin::create(const QString &key) const
{
    if (key == QLatin1String("connman")) {
        QConnmanEngine *engine = new QConnmanEngine;
        if (engine->connmanAvailable())
            return engine;
        else
            delete engine;
    }
    return 0;
}

Q_EXPORT_STATIC_PLUGIN(QConnmanEnginePlugin)
Q_EXPORT_PLUGIN2(qconnmanbearer, QConnmanEnginePlugin)

QT_END_NAMESPACE

#endif
#endif // QT_NO_BEARERMANAGEMENT
