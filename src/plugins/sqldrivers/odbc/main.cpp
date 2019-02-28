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

#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "../../../sql/drivers/odbc/qsql_odbc.h"

class QODBCDriverPlugin : public QSqlDriverPlugin
{
   CS_OBJECT(QODBCDriverPlugin)

   CS_PLUGIN_IID(QSqlDriverInterface_ID)
   CS_PLUGIN_KEY("QODBC")
   // CS_PLUGIN_KEY("QODBC3")

public:
    QODBCDriverPlugin();

    QSqlDriver *create(const QString &) override;
};

CS_PLUGIN_REGISTER(QODBCDriverPlugin)

QODBCDriverPlugin::QODBCDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QODBCDriverPlugin::create(const QString &name)
{
    if (name == "QODBC" || name == "QODBC3") {
        QODBCDriver* driver = new QODBCDriver();
        return driver;
    }
    return 0;
}
