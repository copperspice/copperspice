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

#include <qwayland_xdg_surface_p.h>

#include <qwayland_abstract_decoration_p.h>
#include <qwayland_display_p.h>
#include <qwayland_screen_p.h>
#include <qwayland_window_p.h>
#include <qwayland_xdg_shell_p.h>
#include <qwindow_p.h>

namespace QtWaylandClient {

QWaylandXdgSurface::QWaylandXdgSurface(QWaylandXdgShell *shell, QWaylandWindow *window)
   : QWaylandShellSurface(window), QtWayland::xdg_surface(shell->get_xdg_surface(window->object())),
     m_configured(false), m_serial(0), m_window(window), m_shell(shell)
{

   // pending implementation

}

QWaylandXdgSurface::~QWaylandXdgSurface()
{
   destroy();
}

void QWaylandXdgSurface::applyConfigure()
{

   // pending implementation

}

bool QWaylandXdgSurface::handleExpose(const QRegion &exposeRegion)
{
   if (! isExposed() && ! exposeRegion.isEmpty()) {
      m_exposeRegion = exposeRegion;
      return true;
   }

   return false;
}

bool QWaylandXdgSurface::isExposed() const
{
   return m_configured || m_serial != 0;
}

void QWaylandXdgSurface::setWindowGeometry(QRect rect)
{
   set_window_geometry(rect.x(), rect.y(), rect.width(), rect.height());
}

// following is called directly by wayland
void QWaylandXdgSurface::xdg_surface_configure(uint32_t serial)
{
   m_serial = serial;

   if (m_configured) {
      m_window->applyConfigureLater();

   } else {
      // pending implementation
   }

   if (! m_exposeRegion.isEmpty()) {
      m_window->handleExpose(m_exposeRegion);
      m_exposeRegion = QRegion();
   }
}

}
