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

// Copyright (C) 2014 Eurogiciel, author: <philippe.coval@eurogiciel.fr>

#ifndef QWAYLAND_XDG_SHELL_H
#define QWAYLAND_XDG_SHELL_H

#include <qsize.h>

#include <qwayland-xdg-shell.h>
#include <wayland-client.h>

class QWindow;

namespace QtWaylandClient {

class QWaylandInputDevice;
class QWaylandWindow;
class QWaylandXdgSurface;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgShell : public QtWayland::xdg_wm_base
{
 public:
   QWaylandXdgShell(struct ::xdg_wm_base *shell);
   QWaylandXdgShell(struct ::wl_registry *registry, uint32_t id);

   virtual ~QWaylandXdgShell();

   QWaylandXdgSurface *createXdgSurface(QWaylandWindow *window);

 private:
   void xdg_wm_base_ping(uint32_t serial) override;
};

}

#endif
