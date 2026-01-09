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

#ifndef QWAYLAND_TOPLEVEL_H
#define QWAYLAND_TOPLEVEL_H

#include <qobject.h>
#include <qmargins.h>

#include <wayland-client.h>

class QWindow;

namespace QtWaylandClient {

class QWaylandInputDevice;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandTopLevel : public QObject
{
   CS_OBJECT(QWaylandTopLevel)

 public:
   QWaylandTopLevel() = default;

   virtual ~QWaylandTopLevel()
   { }

   virtual void applyConfigure() {
   }

   virtual void move(QWaylandInputDevice *) {
   }

   virtual void requestWindowStates(Qt::WindowStates) {
   }

   virtual void resize(QWaylandInputDevice *, enum wl_shell_surface_resize) {
   }

   virtual void setAppId(const QString &)  {
   }

   virtual void setTitle(const QString &) {
   }

   virtual void showWindowMenu(QWaylandInputDevice *) {
   }

   virtual bool wantsDecorations() const {
      return false;
   }
};

}

#endif
