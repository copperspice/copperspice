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

#ifndef QWAYLAND_SERVERBUFFER_INTEGRATIONPLUGIN_H
#define QWAYLAND_SERVERBUFFER_INTEGRATIONPLUGIN_H

#include <qfactoryinterface.h>
#include <qobject.h>
#include <qplugin.h>

namespace QtWaylandClient {

class QWaylandServerBufferIntegration;

#define QWaylandServerBufferIntegrationFactoryInterface_ID "com.copperspice.CS.WaylandClient.QWaylandServerBufferIntegrationFactoryInterface"

class Q_WAYLAND_CLIENT_EXPORT QWaylandServerBufferIntegrationPlugin : public QObject
{
   CS_OBJECT(QWaylandServerBufferIntegrationPlugin)

 public:
   explicit QWaylandServerBufferIntegrationPlugin(QObject *parent = nullptr);
   ~QWaylandServerBufferIntegrationPlugin();

   virtual QWaylandServerBufferIntegration *create(const QString &key, const QStringList &paramList) = 0;
};

}

#endif
