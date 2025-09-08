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

#ifndef QWAYLAND_SHELLSURFACE_H
#define QWAYLAND_SHELLSURFACE_H

#include <qobject.h>
#include <qrect.h>

namespace QtWaylandClient {

class QWaylandPopup;
class QWaylandTopLevel;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShellSurface : public QObject
{
   CS_OBJECT(QWaylandShellSurface)

 public:
   explicit QWaylandShellSurface(QWaylandWindow *window);

   virtual ~QWaylandShellSurface()
   { }

   virtual void applyConfigure() {
   }

   virtual bool handleExpose(const QRegion &)  {
      return false;
   }

   virtual bool isExposed() const {
      return true;
   }

   virtual QWaylandPopup *popup() {
      return nullptr;
   }

   virtual void propagateSizeHints() {
   }

   virtual void setWindowGeometry(QRect) {
   }

   virtual QWaylandTopLevel *topLevel() {
      return nullptr;
   }

   inline QWaylandWindow *window() {
      return m_window;
   }

 private:
   QWaylandWindow *m_window;
};

}

#endif
