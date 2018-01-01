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
#include "../../../../src/sql/drivers/sqlite2/qsql_sqlite2.h"

QT_BEGIN_NAMESPACE

class QSQLite2DriverPlugin : public QSqlDriverPlugin
{
public:
    QSQLite2DriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QSQLite2DriverPlugin::QSQLite2DriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QSQLite2DriverPlugin::create(const QString &name)
{
    if (name == "QSQLITE2") {
        QSQLite2Driver* driver = new QSQLite2Driver();
        return driver;
    }
    return 0;
}

QStringList QSQLite2DriverPlugin::keys() const
{
    QStringList l;
    l.append("QSQLITE2");
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QSQLite2DriverPlugin)
Q_EXPORT_PLUGIN2(qsqlite2, QSQLite2DriverPlugin)

QT_END_NAMESPACE
