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



class QSQLite2DriverPlugin : public QSqlDriverPlugin
{
   CS_OBJECT(QSQLite2DriverPlugin)

   CS_PLUGIN_IID("com.copperspice.CS.SqlDriver")
   CS_PLUGIN_KEY("QSQLITE2")

public:
    QSQLite2DriverPlugin();

    QSqlDriver* create(const QString &) override;

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