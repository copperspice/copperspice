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

#include "qgenericpluginfactory.h"

#include "qguiapplication.h"
#include "qfactoryloader_p.h"
#include "qgenericplugin.h"
#include "qdebug.h"

QT_BEGIN_NAMESPACE

#if !defined(Q_OS_WIN32) || defined(QT_SHARED)
#ifndef QT_NO_LIBRARY

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QGenericPluginFactoryInterface_iid,
     QLatin1String("/generic"), Qt::CaseInsensitive))

#endif //QT_NO_LIBRARY
#endif //QT_SHARED

/*!
    \class QGenericPluginFactory
    \ingroup plugins
    \inmodule QtGui

    \brief The QGenericPluginFactory class creates plugin drivers.

    \sa QGenericPlugin
*/

/*!
    Creates the driver specified by \a key, using the given \a specification.

    Note that the keys are case-insensitive.

    \sa keys()
*/
QObject *QGenericPluginFactory::create(const QString& key, const QString &specification)
{
#if (!defined(Q_OS_WIN32) || defined(QT_SHARED)) && !defined(QT_NO_LIBRARY)
    const QString driver = key.toLower();
    if (QObject *object = qLoadPlugin1<QObject, QGenericPlugin>(loader(), driver, specification))
        return object;
#else // (!Q_OS_WIN32 || QT_SHARED) && !QT_NO_LIBRARY
    Q_UNUSED(key)
    Q_UNUSED(specification)
#endif
    return 0;
}

/*!
    Returns the list of valid keys, i.e. the available mouse drivers.

    \sa create()
*/
QStringList QGenericPluginFactory::keys()
{
    QStringList list;

#if !defined(Q_OS_WIN32) || defined(QT_SHARED)
#ifndef QT_NO_LIBRARY
    typedef QMultiMap<int, QString> PluginKeyMap;
    typedef PluginKeyMap::const_iterator PluginKeyMapConstIterator;

    const PluginKeyMap keyMap = loader()->keyMap();
    const PluginKeyMapConstIterator cend = keyMap.constEnd();
    for (PluginKeyMapConstIterator it = keyMap.constBegin(); it != cend; ++it)
        if (!list.contains(it.value()))
            list += it.value();
#endif //QT_NO_LIBRARY
#endif
    return list;
}

QT_END_NAMESPACE
