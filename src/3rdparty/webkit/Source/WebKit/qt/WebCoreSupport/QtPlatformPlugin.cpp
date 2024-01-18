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

#include "config.h"
#include "QtPlatformPlugin.h"

#include "qwebkitplatformplugin.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

namespace WebCore {

bool QtPlatformPlugin::load(const QString& file)
{
    m_loader.setFileName(file);
    if (!m_loader.load())
        return false;

    QObject* obj = m_loader.instance();
    if (obj) {
        m_plugin = qobject_cast<QWebKitPlatformPlugin*>(obj);
        if (m_plugin)
            return true;
    }

    m_loader.unload();
    return false;
}

bool QtPlatformPlugin::load()
{
    const QLatin1String suffix("/webkit/");
    const QStringList paths = QCoreApplication::libraryPaths();

    for (int i = 0; i < paths.count(); ++i) {
        const QDir dir(paths[i] + suffix);
        if (!dir.exists())
            continue;
        const QStringList files = dir.entryList(QDir::Files);
        for (int j = 0; j < files.count(); ++j) {
            if (load(dir.absoluteFilePath(files.at(j))))
                return true;
        }
    }
    return false;
}

QtPlatformPlugin::~QtPlatformPlugin()
{
    m_loader.unload();
}

bool QtPlatformPlugin::loadStaticallyLinkedPlugin()
{
    QObjectList objs = QPluginLoader::staticInstances();
    for (int i = 0; i < objs.size(); ++i) {
        m_plugin = qobject_cast<QWebKitPlatformPlugin*>(objs[i]);
        if (m_plugin)
            return true;
    }
    return false;
}

QWebKitPlatformPlugin* QtPlatformPlugin::plugin()
{
    if (m_loaded) {
        return m_plugin;
    }

    m_loaded = true;

    if (loadStaticallyLinkedPlugin()) {
        return m_plugin;
   }

    // Plugin path is stored in a static variable to avoid searching for the plugin more then once.
    static QString pluginPath;

    if (pluginPath.isEmpty()) {
        if (load()) {
            pluginPath = m_loader.fileName();
        }

    } else {
        load(pluginPath);

    }

    return m_plugin;
}

QWebSelectMethod* QtPlatformPlugin::createSelectInputMethod()
{
    QWebKitPlatformPlugin* p = plugin();
    return p ? static_cast<QWebSelectMethod*>(p->createExtension(QWebKitPlatformPlugin::MultipleSelections)) : 0;
}


QWebNotificationPresenter* QtPlatformPlugin::createNotificationPresenter()
{
    QWebKitPlatformPlugin* p = plugin();
    return p ? static_cast<QWebNotificationPresenter*>(p->createExtension(QWebKitPlatformPlugin::Notifications)) : 0;
}

QWebHapticFeedbackPlayer* QtPlatformPlugin::createHapticFeedbackPlayer()
{
    QWebKitPlatformPlugin* p = plugin();
    return p ? static_cast<QWebHapticFeedbackPlayer*>(p->createExtension(QWebKitPlatformPlugin::Haptics)) : 0;
}

QWebTouchModifier* QtPlatformPlugin::createTouchModifier()
{
    QWebKitPlatformPlugin* p = plugin();
    return p ? static_cast<QWebTouchModifier*>(p->createExtension(QWebKitPlatformPlugin::TouchInteraction)) : 0;
}

#if ENABLE(VIDEO) && USE(QT_MULTIMEDIA)
QWebFullScreenVideoHandler* QtPlatformPlugin::createFullScreenVideoHandler()
{
    QWebKitPlatformPlugin* p = plugin();
    return p ? static_cast<QWebFullScreenVideoHandler*>(p->createExtension(QWebKitPlatformPlugin::FullScreenVideoPlayer)) : 0;
}
#endif

}
