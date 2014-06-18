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

#include "qgenericplugin_qpa.h"

QT_BEGIN_NAMESPACE

/*!
    \class QGenericPlugin
    \ingroup plugins
    \ingroup qpa
    \since 4.8

    \brief The QGenericPlugin class is an abstract base class for
    window-system related plugins in Qt QPA.

    Note that this class is only available in Qt QPA.

    A mouse plugin can be created by subclassing
    QGenericPlugin and reimplementing the pure virtual keys() and
    create() functions. By exporting the derived class using the
    Q_EXPORT_PLUGIN2() macro, The default implementation of the
    QGenericPluginFactory class will automatically detect the plugin and
    load the driver into the server application at run-time. See \l
    {How to Create Qt Plugins} for details.

    \sa QGenericPluginFactory
*/

/*!
    \fn QStringList QGenericPlugin::keys() const

    Implement this function to return the list of valid keys, i.e. the
    drivers supported by this plugin.

    \sa create()
*/

/*!
    Constructs a plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QGenericPlugin::QGenericPlugin(QObject *parent)
   : QObject(parent)
{
}

/*!
    Destroys the plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QGenericPlugin::~QGenericPlugin()
{
}

/*!
    \fn QObject* QGenericPlugin::create(const QString &key, const QString& specification)

    Implement this function to create a driver matching the type
    specified by the given \a key and \a specification parameters. Note that
    keys are case-insensitive.

    \sa keys()
*/

QT_END_NAMESPACE

