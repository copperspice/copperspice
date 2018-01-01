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

#define Q_UUIDIMPL
#include <qsqldriverplugin.h>
#include <qstringlist.h>
#ifdef Q_OS_WIN32    // We assume that MS SQL Server is used. Set Q_USE_SYBASE to force Sybase.
// Conflicting declarations of LPCBYTE in sqlfront.h and winscard.h
#define _WINSCARD_H_
#include <windows.h>
#endif
#include "../../../sql/drivers/tds/qsql_tds.h"

QT_BEGIN_NAMESPACE


class QTDSDriverPlugin : public QSqlDriverPlugin
{
public:
    QTDSDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QTDSDriverPlugin::QTDSDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QTDSDriverPlugin::create(const QString &name)
{
    if (name == "QTDS") {
        QTDSDriver* driver = new QTDSDriver();
        return driver;
    }
    return 0;
}

QStringList QTDSDriverPlugin::keys() const
{
    QStringList l;
    l.append("QTDS");
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QTDSDriverPlugin)
Q_EXPORT_PLUGIN2(qsqltds, QTDSDriverPlugin)

QT_END_NAMESPACE
