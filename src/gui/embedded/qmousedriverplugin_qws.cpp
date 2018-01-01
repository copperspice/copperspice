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

#include <qmousedriverplugin_qws.h>
#include <qmouse_qws.h>

QT_BEGIN_NAMESPACE

/*!
    \class QMouseDriverPlugin
    \ingroup plugins
    \ingroup qws

    \brief The QMouseDriverPlugin class is an abstract base class for
    mouse driver plugins in Qt for Embedded Linux.

    Note that this class is only available in \l{Qt for Embedded Linux}.

    \l{Qt for Embedded Linux} provides ready-made drivers for several mouse
    protocols, see the \l{Qt for Embedded Linux Pointer Handling}{pointer
    handling} documentation for details. Custom mouse drivers can be
    implemented by subclassing the QWSMouseHandler class and creating
    a mouse driver plugin.

    A mouse driver plugin can be created by subclassing
    QMouseDriverPlugin and reimplementing the pure virtual keys() and
    create() functions. By exporting the derived class using the
    Q_EXPORT_PLUGIN2() macro, The default implementation of the
    QMouseDriverFactory class will automatically detect the plugin and
    load the driver into the server application at run-time. See \l
    {How to Create Qt Plugins} for details.

    \sa QWSMouseHandler, QMouseDriverFactory
*/

/*!
    \fn QStringList QMouseDriverPlugin::keys() const

    Implement this function to return the list of valid keys, i.e. the
    mouse drivers supported by this plugin.

    \l{Qt for Embedded Linux} provides ready-made drivers for several mouse
    protocols, see the \l {Qt for Embedded Linux Pointer Handling}{pointer
    handling} documentation for details.

    \sa create()
*/

/*!
    Constructs a mouse driver plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QMouseDriverPlugin::QMouseDriverPlugin(QObject *parent)
   : QObject(parent)
{
}

/*!
    Destroys the mouse driver plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QMouseDriverPlugin::~QMouseDriverPlugin()
{
}

/*!
    \fn QScreen* QMouseDriverPlugin::create(const QString &key, const QString& device)

    Implement this function to create a driver matching the type
    specified by the given \a key and \a device parameters. Note that
    keys are case-insensitive.

    \sa keys()
*/

QT_END_NAMESPACE

