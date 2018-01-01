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
#include "../../../sql/drivers/oci/qsql_oci.h"

QT_BEGIN_NAMESPACE

class QOCIDriverPlugin : public QSqlDriverPlugin
{
public:
    QOCIDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QOCIDriverPlugin::QOCIDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QOCIDriverPlugin::create(const QString &name)
{
    if (name == "QOCI") {
        QOCIDriver* driver = new QOCIDriver();
        return driver;
    }
    return 0;
}

QStringList QOCIDriverPlugin::keys() const
{
    QStringList l;   
    l.append("QOCI");
    return l;
}

Q_EXPORT_STATIC_PLUGIN(QOCIDriverPlugin)
Q_EXPORT_PLUGIN2(qsqloci, QOCIDriverPlugin)

QT_END_NAMESPACE
