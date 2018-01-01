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

#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "../../../sql/drivers/db2/qsql_db2.h"

QT_BEGIN_NAMESPACE

class QDB2DriverPlugin : public QSqlDriverPlugin
{
public:
    QDB2DriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QDB2DriverPlugin::QDB2DriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QDB2DriverPlugin::create(const QString &name)
{
    if (name == "QDB2") {
        QDB2Driver* driver = new QDB2Driver();
        return driver;
    }
    return 0;
}

QStringList QDB2DriverPlugin::keys() const
{
    QStringList l;
    l.append("QDB2");
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QDB2DriverPlugin)
Q_EXPORT_PLUGIN2(qsqldb2, QDB2DriverPlugin)

QT_END_NAMESPACE
