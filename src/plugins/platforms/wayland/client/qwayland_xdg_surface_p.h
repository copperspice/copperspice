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

#ifndef QWAYLAND_XDG_SURFACE_H
#define QWAYLAND_XDG_SURFACE_H

#include <qrect.h>
#include <qregion.h>
#include <quniquepointer.h>

#include <qwayland-xdg-shell.h>

#include <qwayland_shellsurface_p.h>
#include <qwayland_xdg_toplevel_p.h>

class QWindow;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandXdgShell;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgSurface : public QWaylandShellSurface, public QtWayland::xdg_surface
{
   CS_OBJECT(QWaylandXdgSurface)

 public:
   QWaylandXdgSurface(QWaylandXdgShell *shell, QWaylandWindow *window);
   ~QWaylandXdgSurface() override;

   void applyConfigure() override;

   bool handleExpose(const QRegion &exposeRegion) override;

   QWaylandXdgShell *getXdgShell() const {
      return m_shell;
   }

   bool isExposed() const override;

   bool isTopLevel() const {
      return m_topLevel != nullptr;
   }

   void propagateSizeHints() override;

   QWaylandXdgTopLevel *topLevel() override;

   void setWindowGeometry(QRect rect) override;

 protected:
   void xdg_surface_configure(uint32_t serial) override;

 private:
   bool m_configured;
   uint32_t m_serial;

   QRegion m_exposeRegion;

   QWaylandWindow *m_window;
   QWaylandXdgShell *m_shell;

   QUniquePointer<QWaylandXdgTopLevel> m_topLevel;
};

}

#endif
