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

#include <qwayland_xdg_toplevel_p.h>

#include <qpoint.h>
#include <qwindow.h>

#include <qwayland_abstract_decoration_p.h>
#include <qwayland_display_p.h>
#include <qwayland_inputdevice_p.h>
#include <qwayland_window_p.h>
#include <qwayland_xdg_shell_p.h>
#include <qwayland_xdg_surface_p.h>

namespace QtWaylandClient {

QWaylandXdgTopLevel::QWaylandXdgTopLevel(QWaylandXdgSurface *surface, QWaylandWindow *window)
   : QtWayland::xdg_toplevel(surface->get_toplevel()), m_surface(surface), m_window(window)
{
}

QWaylandXdgTopLevel::~QWaylandXdgTopLevel()
{
   if (isInitialized()) {
      destroy();
   }
}

void QWaylandXdgTopLevel::move(QWaylandInputDevice *inputDevice)
{
   move(inputDevice->wl_seat(), inputDevice->serial());
}

void QWaylandXdgTopLevel::resize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize edges)
{
   enum resize_edge data = bit_cast<enum resize_edge>(edges);
   resize(inputDevice, data);
}

void QWaylandXdgTopLevel::resize(QWaylandInputDevice *inputDevice, enum resize_edge edges)
{
   resize(inputDevice->wl_seat(), inputDevice->serial(), edges);
}

void QWaylandXdgTopLevel::setAppId(const QString &appId)
{
   set_app_id(appId);
}

void QWaylandXdgTopLevel::setTitle(const QString &title)
{
   set_title(title);
}

void QWaylandXdgTopLevel::showWindowMenu(QWaylandInputDevice *inputDevice)
{
   QPointF menuLocation = inputDevice->cursorPosition().toPoint();
   show_window_menu(inputDevice->wl_seat(), inputDevice->serial(), menuLocation.x(), menuLocation.y());
}

// following two methods are called directly by wayland
void QWaylandXdgTopLevel::xdg_toplevel_configure(int32_t width, int32_t height, wl_array *states)
{
   // pending implementation
}

void QWaylandXdgTopLevel::xdg_toplevel_close()
{
   m_window->window()->close();
}

}
