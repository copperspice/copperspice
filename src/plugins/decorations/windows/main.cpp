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
#include <qdecorationwindows_qws.h>

QT_BEGIN_NAMESPACE

class DecorationWindows : public QDecorationPlugin
{
public:
    DecorationWindows();

    QStringList keys() const;
    QDecoration *create(const QString&);
};

DecorationWindows::DecorationWindows()
    : QDecorationPlugin()
{
}

QStringList DecorationWindows::keys() const
{
    return (QStringList() << QLatin1String("Windows"));
}

QDecoration* DecorationWindows::create(const QString& s)
{
    if (s.toLower() == QLatin1String("windows"))
        return new QDecorationWindows();

    return 0;
}

Q_EXPORT_PLUGIN2(qdecorationwindows, DecorationWindows)

QT_END_NAMESPACE
