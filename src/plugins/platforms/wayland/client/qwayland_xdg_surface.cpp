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
#include <qwayland_xdg_toplevel_p.h>
#include <qwindow_p.h>

namespace QtWaylandClient {

QWaylandXdgSurface::QWaylandXdgSurface(QWaylandXdgShell *shell, QWaylandWindow *window)
   : QWaylandShellSurface(window), QtWayland::xdg_surface(shell->get_xdg_surface(window->object())),
     m_configured(false), m_serial(0), m_window(window), m_shell(shell)
{
   QWaylandDisplay *display = m_window->display();

   Qt::WindowType type   = m_window->window()->type();
   auto *transientParent = m_window->transientParent();

   if (type == Qt::ToolTip && transientParent) {
      // pending implementation

   } else if (type == Qt::Popup && transientParent != nullptr && display->lastInputDevice()) {
      // pending implementation

   } else {
      if (transientParent != nullptr) {
         auto parentXdgSurface = static_cast<QWaylandXdgSurface *>(transientParent->shellSurface());

         if (parentXdgSurface != nullptr) {
            if (m_topLevel != nullptr) {
               m_topLevel->set_parent(parentXdgSurface->m_topLevel->object());
            }
         }
      }
   }
}

QWaylandXdgSurface::~QWaylandXdgSurface()
{
   destroy();
}

void QWaylandXdgSurface::applyConfigure()
{
   if (m_topLevel != nullptr) {
      m_topLevel->applyConfigure();
   }

   m_configured = true;

   ack_configure(m_serial);
   m_serial = 0;
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

void QWaylandXdgSurface::propagateSizeHints()
{
   if (m_topLevel != nullptr && m_window != nullptr) {
      m_window->commit();
   }
}

QWaylandXdgTopLevel *QWaylandXdgSurface::topLevel()
{
   if (m_topLevel != nullptr) {
      return m_topLevel.get();
   }

   m_topLevel = QMakeUnique<QWaylandXdgTopLevel>(this, m_window);

   return m_topLevel.get();
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
      if (m_topLevel != nullptr) {
         m_topLevel->applyConfigure();
      }

      ack_configure(m_serial);

      m_configured = true;
      m_serial = 0;

      m_exposeRegion = QRegion(QRect(QPoint(), m_window->geometry().size()));
   }

   if (! m_exposeRegion.isEmpty()) {
      m_window->handleExpose(m_exposeRegion);
      m_exposeRegion = QRegion();
   }
}

}
