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
#include "../../../../src/sql/drivers/sqlite/qsql_sqlite.h"

QT_BEGIN_NAMESPACE

class QSQLiteDriverPlugin : public QSqlDriverPlugin
{
public:
    QSQLiteDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QSQLiteDriverPlugin::QSQLiteDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QSQLiteDriverPlugin::create(const QString &name)
{
    if (name == "QSQLITE") {
        QSQLiteDriver* driver = new QSQLiteDriver();
        return driver;
    }
    return 0;
}

QStringList QSQLiteDriverPlugin::keys() const
{
    QStringList l;
    l.append("QSQLITE");
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QSQLiteDriverPlugin)
Q_EXPORT_PLUGIN2(qsqlite, QSQLiteDriverPlugin)

QT_END_NAMESPACE
