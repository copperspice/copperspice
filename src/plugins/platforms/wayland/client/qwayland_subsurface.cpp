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

#include <qwayland_subsurface_p.h>

#include <qwayland_window_p.h>

namespace QtWaylandClient {

QWaylandSubSurface::QWaylandSubSurface(QWaylandWindow *window, QWaylandWindow *parent, ::wl_subsurface *sub_surface)
   : QtWayland::wl_subsurface(sub_surface), m_window(window), m_parent(parent)
{
   m_parent->m_children.append(this);
   set_desync();
}

QWaylandSubSurface::~QWaylandSubSurface()
{
   m_parent->m_children.removeOne(this);
   destroy();
}

}
