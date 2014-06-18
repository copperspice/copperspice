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

#include "qstyleplugin.h"
#include "qstyle.h"

QT_BEGIN_NAMESPACE

/*!
    \class QStylePlugin
    \brief The QStylePlugin class provides an abstract base for custom QStyle plugins.

    \ingroup plugins

    QStylePlugin is a simple plugin interface that makes it easy
    to create custom styles that can be loaded dynamically into
    applications using the QStyleFactory class.

    Writing a style plugin is achieved by subclassing this base class,
    reimplementing the pure virtual keys() and create() functions, and
    exporting the class using the Q_EXPORT_PLUGIN2() macro. See \l
    {How to Create Qt Plugins} for details.

    \sa QStyleFactory, QStyle
*/

/*!
    \fn QStringList QStylePlugin::keys() const

    Returns the list of style keys this plugin supports.

    These keys are usually the class names of the custom styles that
    are implemented in the plugin.

    \sa create()
*/

/*!
    \fn QStyle *QStylePlugin::create(const QString& key)

    Creates and returns a QStyle object for the given style \a key.
    If a plugin cannot create a style, it should return 0 instead.

    The style key is usually the class name of the required
    style. Note that the keys are case insensitive. For example:

    \snippet doc/src/snippets/qstyleplugin/main.cpp 0
    \codeline
    \snippet doc/src/snippets/qstyleplugin/main.cpp 1
    \snippet doc/src/snippets/qstyleplugin/main.cpp 2

    \sa keys()
*/

/*!
    Constructs a style plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QStylePlugin::QStylePlugin(QObject *parent)
   : QObject(parent)
{
}

/*!
    Destroys the style plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QStylePlugin::~QStylePlugin()
{
}

QT_END_NAMESPACE
