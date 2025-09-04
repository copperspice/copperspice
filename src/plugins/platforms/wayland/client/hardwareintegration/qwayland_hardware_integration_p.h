/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QWAYLAND_HARDWARE_INTEGRATION_H
#define QWAYLAND_HARDWARE_INTEGRATION_H

#include <qwayland-hardware-integration.h>

namespace QtWaylandClient {

class QWaylandDisplay;

class Q_WAYLAND_CLIENT_EXPORT QWaylandHardwareIntegration : public QtWayland::qt_hardware_integration
{
 public:
   QWaylandHardwareIntegration(struct ::wl_registry *registry, int id);

   QString clientBufferIntegration();
   QString serverBufferIntegration();

 protected:
   void hardware_integration_client_backend(const QString &name) override;
   void hardware_integration_server_backend(const QString &name) override;

 private:
   QString m_client_buffer;
   QString m_server_buffer;
};

}

#endif
