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
   : QtWayland::xdg_toplevel(surface->get_toplevel()), m_active(false), m_wasActive(false),
     m_surface(surface), m_window(window)

{
}

QWaylandXdgTopLevel::~QWaylandXdgTopLevel()
{
   if (m_applied.states & Qt::WindowActive) {
      QWaylandWindow *wl_window = m_surface->window();
      wl_window->display()->handleWindowDeactivated(wl_window);
   }

   if (isInitialized()) {
      destroy();
   }
}

void QWaylandXdgTopLevel::applyConfigure()
{
   QWaylandWindow *wl_window = m_surface->window();

   if (! (m_applied.states & (Qt::WindowMaximized|Qt::WindowFullScreen))) {
      m_normalSize = wl_window->window()->frameGeometry().size();
   }

   if ((m_pending.states & Qt::WindowActive) && ! (m_applied.states & Qt::WindowActive)) {
      wl_window->display()->handleWindowActivated(wl_window);
   }

   if (! (m_pending.states & Qt::WindowActive) && (m_applied.states & Qt::WindowActive)) {
      wl_window->display()->handleWindowDeactivated(wl_window);
   }

   Qt::WindowStates statesWithoutActive = m_pending.states & ~Qt::WindowActive;

   wl_window->handleWindowStatesChanged(statesWithoutActive);

   if (m_pending.size.isEmpty()) {
      bool normalPending = ! (m_pending.states & (Qt::WindowMaximized|Qt::WindowFullScreen));

      if (normalPending && ! m_normalSize.isEmpty()) {
         wl_window->resizeApplyConfigure(m_normalSize);
      }

   } else {
      wl_window->resizeApplyConfigure(m_pending.size);
   }

   QSize windowGeometrySize = wl_window->window()->frameGeometry().size();

   QPoint pos = wl_window->window()->framePosition();
   m_surface->set_window_geometry(pos.x(), pos.y(), windowGeometrySize.width(), windowGeometrySize.height());

   m_applied = m_pending;
}

void QWaylandXdgTopLevel::move(QWaylandInputDevice *inputDevice)
{
   move(inputDevice->wl_seat(), inputDevice->serial());
}

void QWaylandXdgTopLevel::requestWindowStates(Qt::WindowStates states)
{
   Qt::WindowStates changedStates = m_applied.states ^ states;

   if (changedStates & Qt::WindowMaximized) {
      if (states & Qt::WindowMaximized) {
         set_maximized();
      } else {
         unset_maximized();
      }
   }

   if (changedStates & Qt::WindowFullScreen) {
      if (states & Qt::WindowFullScreen) {
         set_fullscreen(nullptr);
      } else {
         unset_fullscreen();
      }
   }

   if (states & Qt::WindowMinimized) {
      set_minimized();
      m_surface->window()->handleWindowStatesChanged(states & ~Qt::WindowMinimized);
   }
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

bool QWaylandXdgTopLevel::wantsDecorations() const
{
   return ! (m_pending.states & Qt::WindowFullScreen);
}

// following two methods are called directly by wayland
void QWaylandXdgTopLevel::xdg_toplevel_configure(int32_t width, int32_t height, wl_array *states)
{
   m_pending.size   = QSize(width, height);
   m_pending.states = Qt::WindowNoState;

   uint32_t *state  = static_cast<uint32_t *>(states->data);
   size_t numStates = states->size / sizeof(uint32_t);

   for (size_t i = 0; i < numStates; ++i) {
      switch (state[i]) {

         case XDG_TOPLEVEL_STATE_ACTIVATED:
            m_pending.states |= Qt::WindowActive;
            break;

         case XDG_TOPLEVEL_STATE_FULLSCREEN:
            m_pending.states |= Qt::WindowFullScreen;
            break;

         case XDG_TOPLEVEL_STATE_MAXIMIZED:
            m_pending.states |= Qt::WindowMaximized;
            break;

         default:
            break;
      }
   }
}

void QWaylandXdgTopLevel::xdg_toplevel_close()
{
   m_window->window()->close();
}

}
