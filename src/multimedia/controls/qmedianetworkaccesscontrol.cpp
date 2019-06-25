/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qmedianetworkaccesscontrol.h>


/*!
    \class QMediaNetworkAccessControl
    \brief The QMediaNetworkAccessControl class allows the setting of the Network Access Point for media related activities.
    \inmodule QtMultimedia


    \ingroup multimedia_control

    The functionality provided by this control allows the
    setting of a Network Access Point.

    This control can be used to set a network access for various
    network related activities.  The exact nature is dependent on the underlying
    usage by the supported QMediaObject.
*/

/*!
  \internal
*/
QMediaNetworkAccessControl::QMediaNetworkAccessControl(QObject *parent) :
   QMediaControl(parent)
{
}

/*!
    Destroys a network access control.
*/
QMediaNetworkAccessControl::~QMediaNetworkAccessControl()
{
}

/*!
    \fn void QMediaNetworkAccessControl::setConfigurations(const QList<QNetworkConfiguration> &configurations)

    The \a configurations parameter contains a list of network configurations to be used for network access.

    It is assumed the list is given in highest to lowest preference order.
    By calling this function all previous configurations will be invalidated
    and replaced with the new list.
*/

/*!
    \fn QNetworkConfiguration QMediaNetworkAccessControl::currentConfiguration() const

    Returns the current active configuration in use.
    A default constructed QNetworkConfigration is returned if no user supplied configuration are in use.
*/


/*!
    \fn QMediaNetworkAccessControl::configurationChanged(const QNetworkConfiguration &configuration)
    This signal is emitted when the current active network configuration changes
    to \a configuration.
*/


