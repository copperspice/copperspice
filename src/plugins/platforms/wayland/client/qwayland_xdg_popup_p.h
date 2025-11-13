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

#ifndef QWAYLAND_XDG_POPUP_P_H
#define QWAYLAND_XDG_POPUP_P_H

#include <qwayland-xdg-shell.h>
#include <wayland-client.h>

#include <qwayland_popup_p.h>

class QWindow;

namespace QtWaylandClient {

class QWaylandInputDevice;
class QWaylandWindow;
class QWaylandXdgSurface;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgPopup : public QWaylandPopup, public QtWayland::xdg_popup
{
   CS_OBJECT(QWaylandXdgPopup)

 public:
   QWaylandXdgPopup(QWaylandXdgSurface *xdgSurface, QWaylandXdgSurface *parent, QtWayland::xdg_positioner *positioner);
   virtual ~QWaylandXdgPopup();

   QWaylandXdgSurface *getXdgSurface() const {
      return m_xdgSurface;
   }

   void grab(QWaylandInputDevice *seat, uint serial);

 protected:
   void xdg_popup_popup_done() override;

 private:
   bool m_grabbing;

   QWaylandXdgSurface *m_xdgSurface;
   QWaylandXdgSurface *m_parent;
};

}

#endif
