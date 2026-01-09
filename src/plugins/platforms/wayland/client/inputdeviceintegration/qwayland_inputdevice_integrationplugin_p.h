/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

// Copyright (C) 2014 LG Electronics Ltd

#ifndef QWAYLAND_INPUTDEVICE_INTEGRATIONPLUGIN_H
#define QWAYLAND_INPUTDEVICE_INTEGRATIONPLUGIN_H

#include <qfactoryinterface.h>
#include <qobject.h>
#include <qplugin.h>

namespace QtWaylandClient {

class QWaylandInputDeviceIntegration;

#define QWaylandInputDeviceIntegrationFactoryInterface_ID "com.copperspice.CS.WaylandClient.QWaylandInputDeviceIntegrationFactoryInterface"

class Q_WAYLAND_CLIENT_EXPORT QWaylandInputDeviceIntegrationPlugin : public QObject
{
   CS_OBJECT(QWaylandInputDeviceIntegrationPlugin)

 public:
   explicit QWaylandInputDeviceIntegrationPlugin(QObject *parent = nullptr);
   ~QWaylandInputDeviceIntegrationPlugin();

   virtual QWaylandInputDeviceIntegration *create(const QString &key, const QStringList &paramList) = 0;
};

}

#endif
