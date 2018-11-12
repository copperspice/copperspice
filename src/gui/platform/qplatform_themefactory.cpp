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

#include <qplatform_themefactory_p.h>
#include <qplatform_themeplugin.h>
#include <qdir.h>
#include <qfactoryloader_p.h>
#include <qmutex.h>
#include <qguiapplication.h>
#include <qdebug.h>

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QPlatformThemeFactoryInterface_iid, QLatin1String("/platformthemes"), Qt::CaseInsensitive))

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, directLoader,
    (QPlatformThemeFactoryInterface_iid, QLatin1String(""), Qt::CaseInsensitive))


QPlatformTheme *QPlatformThemeFactory::create(const QString& key, const QString &platformPluginPath)
{
    QStringList paramList = key.split(QLatin1Char(':'));
    const QString platform = paramList.takeFirst().toLower();

    // Try loading the plugin from platformPluginPath first:
    if (!platformPluginPath.isEmpty()) {
        QCoreApplication::addLibraryPath(platformPluginPath);
        if (QPlatformTheme *ret = qLoadPlugin1<QPlatformTheme, QPlatformThemePlugin>(directLoader(), platform, paramList))
            return ret;
    }

    if (QPlatformTheme *ret = qLoadPlugin1<QPlatformTheme, QPlatformThemePlugin>(loader(), platform, paramList))
           return ret;

    return 0;
}

/*!
    Returns the list of valid keys, i.e. the keys this factory can
    create styles for.

    \sa create()
*/
QStringList QPlatformThemeFactory::keys(const QString &platformPluginPath)
{
    QStringList list;

    if (!platformPluginPath.isEmpty()) {
        QCoreApplication::addLibraryPath(platformPluginPath);
        list += directLoader()->keyMap().values();
        if (!list.isEmpty()) {
            const QString postFix = QStringLiteral(" (from ")
                    + QDir::toNativeSeparators(platformPluginPath)
                    + QLatin1Char(')');
            const QStringList::iterator end = list.end();
            for (QStringList::iterator it = list.begin(); it != end; ++it)
                (*it).append(postFix);
        }
    }

    list += loader()->keyMap().values();
    return list;

}

