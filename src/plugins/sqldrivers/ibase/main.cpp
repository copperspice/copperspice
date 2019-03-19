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
#include "../../../sql/drivers/ibase/qsql_ibase.h"

QT_BEGIN_NAMESPACE

class QIBaseDriverPlugin : public QSqlDriverPlugin
{
public:
    QIBaseDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QIBaseDriverPlugin::QIBaseDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QIBaseDriverPlugin::create(const QString &name)
{
    if (name == "QIBASE") {
        QIBaseDriver* driver = new QIBaseDriver();
        return driver;
    }
    return 0;
}

QStringList QIBaseDriverPlugin::keys() const
{
    QStringList l;
    l.append("QIBASE");   
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QIBaseDriverPlugin)
Q_EXPORT_PLUGIN2(qsqlibase, QIBaseDriverPlugin)

QT_END_NAMESPACE
