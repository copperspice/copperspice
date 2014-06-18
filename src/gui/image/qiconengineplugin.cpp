/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qiconengineplugin.h"
#include "qiconengine.h"

QT_BEGIN_NAMESPACE

/*!
    \class QIconEnginePlugin
    \brief The QIconEnginePlugin class provides an abstract base for custom QIconEngine plugins.

    \ingroup plugins

    \bold {Use QIconEnginePluginV2 instead.}

    The icon engine plugin is a simple plugin interface that makes it easy to
    create custom icon engines that can be loaded dynamically into applications
    through QIcon. QIcon uses the file or resource name's suffix to determine
    what icon engine to use.

    Writing a icon engine plugin is achieved by subclassing this base class,
    reimplementing the pure virtual functions keys() and create(), and
    exporting the class with the Q_EXPORT_PLUGIN2() macro.

    \sa {How to Create Qt Plugins}
*/

/*!
    \fn QStringList QIconEnginePlugin::keys() const

    Returns a list of icon engine keys that this plugin supports. The keys correspond
    to the suffix of the file or resource name used when the plugin was created.
    Keys are case insensitive.

    \sa create()
*/

/*!
    \fn QIconEngine* QIconEnginePlugin::create(const QString& filename)

    Creates and returns a QIconEngine object for the icon with the given
    \a filename.

    \sa keys()
*/

/*!
    Constructs a icon engine plugin with the given \a parent. This is invoked
    automatically by the Q_EXPORT_PLUGIN2() macro.
*/
QIconEnginePlugin::QIconEnginePlugin(QObject *parent)
   : QObject(parent)
{
}

/*!
    Destroys the icon engine plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QIconEnginePlugin::~QIconEnginePlugin()
{
}

// version 2

/*!
    \class QIconEnginePluginV2
    \brief The QIconEnginePluginV2 class provides an abstract base for custom QIconEngineV2 plugins.

    \ingroup plugins
    \since 4.3

    Icon engine plugins produces \l{QIconEngine}s for \l{QIcon}s; an
    icon engine is used to render the icon. The keys that identifies
    the engines the plugin can create are suffixes of
    icon filenames; they are returned by keys(). The create() function
    receives the icon filename to return an engine for; it should
    return 0 if it cannot produce an engine for the file.

    Writing an icon engine plugin is achieved by inheriting
    QIconEnginePluginV2, reimplementing keys() and create(), and
    adding the Q_EXPORT_PLUGIN2() macro.

    You should ensure that you do not duplicate keys. Qt will query
    the plugins for icon engines in the order in which the plugins are
    found during plugin search (see the plugins \l{How to Create Qt
    Plugins}{overview document}).

    \sa {How to Create Qt Plugins}
*/

/*!
    \fn QStringList QIconEnginePluginV2::keys() const

    Returns a list of icon engine keys that this plugin supports. The keys correspond
    to the suffix of the file or resource name used when the plugin was created.
    Keys are case insensitive.

    \sa create()
*/

/*!
    \fn QIconEngineV2* QIconEnginePluginV2::create(const QString& filename = QString())

    Creates and returns a QIconEngine object for the icon with the given
    \a filename.

    \sa keys()
*/

/*!
    Constructs a icon engine plugin with the given \a parent. This is invoked
    automatically by the Q_EXPORT_PLUGIN2() macro.
*/
QIconEnginePluginV2::QIconEnginePluginV2(QObject *parent)
   : QObject(parent)
{
}

/*!
    Destroys the icon engine plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QIconEnginePluginV2::~QIconEnginePluginV2()
{
}

QT_END_NAMESPACE
