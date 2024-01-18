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

#include <qgenericplugin.h>
#include "qtslib.h"

class QTsLibPlugin : public QGenericPlugin
{
public:
    QTsLibPlugin();

    QStringList keys() const;
    QObject* create(const QString &key, const QString &specification);
};

QTsLibPlugin::QTsLibPlugin()
    : QGenericPlugin()
{
}

QStringList QTsLibPlugin::keys() const
{
    return (QStringList() << "Tslib" << "TslibRaw");
}

QObject* QTsLibPlugin::create(const QString &key, const QString &specification)
{
    if (! key.compare("Tslib", Qt::CaseInsensitive) || ! key.compare("TslibRaw", Qt::CaseInsensitive))
        return new QTsLibMouseHandler(key, specification);

    return 0;
}

Q_EXPORT_PLUGIN2(qtslibplugin, QTsLibPlugin)

