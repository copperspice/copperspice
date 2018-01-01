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

#include <qgenericplugin_qpa.h>

QT_BEGIN_NAMESPACE

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

