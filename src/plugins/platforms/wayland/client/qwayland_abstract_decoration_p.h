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

// Copyright (C) 2014 Robin Burchell <robin.burchell@viroteck.net>

#ifndef QWAYLAND_ABSTRACT_DECORATION_H
#define QWAYLAND_ABSTRACT_DECORATION_H

#include <qapplication.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qimage.h>
#include <qmargins.h>
#include <qpointf.h>
#include <qstatictext.h>

#include <wayland-client.h>

class QEvent;
class QPaintDevice;
class QPainter;
class QWindow;

namespace QtWaylandClient {

class QWaylandInputDevice;
class QWaylandScreen;
class QWaylandWindow;

class QWaylandAbstractDecorationPrivate;

class Q_WAYLAND_CLIENT_EXPORT QWaylandAbstractDecoration : public QObject
{
   CS_OBJECT(QWaylandAbstractDecoration)

 public:
   QWaylandAbstractDecoration();
   virtual ~QWaylandAbstractDecoration();

   void setWaylandWindow(QWaylandWindow *window);
   QWaylandWindow *waylandWindow() const;

   void update();
   bool isDirty() const;

   virtual QMargins margins() const = 0;
   QWindow *window() const;
   const QImage &contentImage();

   virtual bool handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
         Qt::MouseButtons b, Qt::KeyboardModifiers mods) = 0;

   virtual bool handleTouch(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
         Qt::TouchPointState state, Qt::KeyboardModifiers mods) = 0;

 protected:
   virtual void paint(QPaintDevice *device) = 0;

   void setMouseButtons(Qt::MouseButtons mb);

   void startResize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize resize, Qt::MouseButtons buttons);
   void startMove(QWaylandInputDevice *inputDevice, Qt::MouseButtons buttons);

   bool isLeftClicked(Qt::MouseButtons newMouseButtonState);
   bool isLeftReleased(Qt::MouseButtons newMouseButtonState);

   QScopedPointer<QWaylandAbstractDecorationPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QWaylandAbstractDecoration)
};

}

#endif
