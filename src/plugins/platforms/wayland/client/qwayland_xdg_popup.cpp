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

#include <qwayland_xdg_popup_p.h>

#include <qwayland_inputdevice_p.h>
#include <qwayland_window_p.h>
#include <qwayland_xdg_shell_p.h>
#include <qwayland_xdg_surface_p.h>

namespace QtWaylandClient {

QWaylandXdgPopup::QWaylandXdgPopup(QWaylandXdgSurface *xdgSurface, QWaylandXdgSurface *parent, QtWayland::xdg_positioner *positioner)
   : QtWayland::xdg_popup(xdgSurface->get_popup(parent->object(), positioner->object())),
     m_grabbing(false), m_xdgSurface(xdgSurface), m_parent(parent)
{
}

QWaylandXdgPopup::~QWaylandXdgPopup()
{
   if (isInitialized()) {
      destroy();
   }

   if (m_grabbing) {
      QWaylandXdgShell *obj = m_xdgSurface->getXdgShell();
      obj->setXdgPopup(m_parent->getXdgPopup());
   }
}

void QWaylandXdgPopup::grab(QWaylandInputDevice *seat, uint serial)
{
   QWaylandXdgShell *obj = m_xdgSurface->getXdgShell();
   obj->setXdgPopup(this);

   xdg_popup::grab(seat->wl_seat(), serial);
   m_grabbing = true;
}

// following methdod is called directly by wayland

void QWaylandXdgPopup::xdg_popup_popup_done()
{
   QWaylandWindow *wl_window = m_xdgSurface->window();
   wl_window->window()->close();
}

}
