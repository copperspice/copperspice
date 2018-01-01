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

#ifndef QT_NO_IM
#include "qmultiinputcontext.h"
#include "qmultiinputcontextplugin.h"
#include <qinputcontextplugin.h>
#include <qstringlist.h>

QT_BEGIN_NAMESPACE

QMultiInputContextPlugin::QMultiInputContextPlugin()
{
}

QMultiInputContextPlugin::~QMultiInputContextPlugin()
{
}

QStringList QMultiInputContextPlugin::keys() const
{
    // input method switcher should named with "imsw-" prefix to
    // prevent to be listed in ordinary input method list.
    return QStringList( QLatin1String("imsw-multi") );
}

QInputContext *QMultiInputContextPlugin::create( const QString &key )
{
    if (key != QLatin1String("imsw-multi"))
        return 0;
    return new QMultiInputContext;
}

QStringList QMultiInputContextPlugin::languages( const QString & )
{
    return QStringList();
}

QString QMultiInputContextPlugin::displayName( const QString &key )
{
    if (key != QLatin1String("imsw-multi"))
        return QString();
    return tr( "Multiple input method switcher" );
}

QString QMultiInputContextPlugin::description( const QString &key )
{
    if (key != QLatin1String("imsw-multi"))
        return QString();
    return tr( "Multiple input method switcher that uses the context menu of the text widgets" );
}


Q_EXPORT_STATIC_PLUGIN(QMultiInputContextPlugin)
Q_EXPORT_PLUGIN2(qimsw_multi, QMultiInputContextPlugin)

QT_END_NAMESPACE

#endif
