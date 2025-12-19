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

#ifndef QWAYLAND_XDG_TOPLEVEL_H
#define QWAYLAND_XDG_TOPLEVEL_H

#include <qsize.h>

#include <qwayland-xdg-shell.h>
#include <wayland-client.h>

#include <qwayland_toplevel_p.h>

class QWindow;

namespace QtWaylandClient {

class QWaylandInputDevice;
class QWaylandWindow;
class QWaylandXdgSurface;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgTopLevel : public QWaylandTopLevel, public QtWayland::xdg_toplevel
{
   CS_OBJECT(QWaylandXdgTopLevel)

 public:
   QWaylandXdgTopLevel(QWaylandXdgSurface *surface, QWaylandWindow *window);

   virtual ~QWaylandXdgTopLevel();

   void applyConfigure() override;

   using QtWayland::xdg_toplevel::move;
   void move(QWaylandInputDevice *inputDevice) override;

   void requestWindowStates(Qt::WindowStates states) override;

   using QtWayland::xdg_toplevel::resize;
   void resize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize edges) override;
   void resize(QWaylandInputDevice *inputDevice, enum resize_edge edges);

   void setAppId(const QString &appId) override;
   void setTitle(const QString &title) override;

   void showWindowMenu(QWaylandInputDevice *inputDevice) override;

   bool wantsDecorations() const override;

 protected:
   void xdg_toplevel_configure(int32_t width, int32_t height, wl_array *states) override;
   void xdg_toplevel_close() override;

 private:
   bool m_active;
   bool m_wasActive;

   QWaylandXdgSurface *m_surface;
   QWaylandWindow *m_window;

   QSize m_normalSize;

   struct StateData {
      QSize size = {0, 0};
      Qt::WindowStates states = Qt::WindowNoState;
    };

   StateData m_pending;
   StateData m_applied;
};

}

#endif
