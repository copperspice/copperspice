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

#include <qdecorationplugin_qws.h>
#include <qdecorationstyled_qws.h>

QT_BEGIN_NAMESPACE

class DecorationStyled : public QDecorationPlugin
{
public:
    DecorationStyled();

    QStringList keys() const;
    QDecoration *create(const QString&);
};

DecorationStyled::DecorationStyled() : QDecorationPlugin()
{
}

QStringList DecorationStyled::keys() const
{
    return (QStringList() << QLatin1String("Styled"));
}

QDecoration* DecorationStyled::create(const QString& s)
{
    if (s.toLower() != QLatin1String("styled"))
        return 0;

    qDebug("creatign styled decoration");

    return new QDecorationStyled;
}

Q_EXPORT_PLUGIN2(qdecorationstyled, DecorationStyled)

QT_END_NAMESPACE
