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

#ifndef QWAYLAND_SUBSURFACE_H
#define QWAYLAND_SUBSURFACE_H

#include <qglobal.h>

#include <qwayland-wayland.h>
#include <wayland-client.h>

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandSubSurface : public QtWayland::wl_subsurface
{
 public:
   QWaylandSubSurface(QWaylandWindow *window, QWaylandWindow *parent, ::wl_subsurface *subsurface);
   ~QWaylandSubSurface();

   QWaylandWindow *window() const {
      return m_window;
   }

   QWaylandWindow *parent() const {
      return m_parent;
   }

 private:
   QWaylandWindow *m_window;
   QWaylandWindow *m_parent;
};

}

#endif
