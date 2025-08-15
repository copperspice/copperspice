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

#include <qwayland_abstract_decoration_p.h>

#include <qimage.h>
#include <qwindow.h>

#include <qwayland_inputdevice_p.h>
#include <qwayland_screen_p.h>
#include <qwayland_shellsurface_p.h>
#include <qwayland_toplevel_p.h>
#include <qwayland_window_p.h>

namespace QtWaylandClient {

class QWaylandAbstractDecorationPrivate
{
 public:
   QWaylandAbstractDecorationPrivate();
   ~QWaylandAbstractDecorationPrivate();

   QWindow *m_window;
   QWaylandWindow *m_wayland_window;

   bool m_isDirty;
   QImage m_decorationContentImage;

   Qt::MouseButtons m_mouseButtons;

 protected:
   QWaylandAbstractDecoration *q_ptr;

 private:
   Q_DECLARE_PUBLIC(QWaylandAbstractDecoration)
};

QWaylandAbstractDecorationPrivate::QWaylandAbstractDecorationPrivate()
   : m_window(nullptr), m_wayland_window(nullptr), m_isDirty(true), m_decorationContentImage(nullptr), m_mouseButtons(Qt::NoButton)
{
}

QWaylandAbstractDecorationPrivate::~QWaylandAbstractDecorationPrivate()
{
}

QWaylandAbstractDecoration::QWaylandAbstractDecoration()
   : d_ptr(new QWaylandAbstractDecorationPrivate)
{
   d_ptr->q_ptr = this;
}

QWaylandAbstractDecoration::~QWaylandAbstractDecoration()
{
}

static QRegion marginsRegion(const QSize &size, const QMargins &margins)
{
   QRegion r;

   const int widthWithMargins = margins.left() + size.width() + margins.right();

   r += QRect(0, 0, widthWithMargins, margins.top());
   r += QRect(0, size.height()+margins.top(), widthWithMargins, margins.bottom());
   r += QRect(0, margins.top(), margins.left(), size.height());
   r += QRect(size.width()+margins.left(), margins.top(), margins.right(), size.height());

   return r;
}

void QWaylandAbstractDecoration::setWaylandWindow(QWaylandWindow *window)
{
   Q_D(QWaylandAbstractDecoration);

   // double initialization is probably not great
   Q_ASSERT(! d->m_window && ! d->m_wayland_window);

   d->m_window = window->window();
   d->m_wayland_window = window;
}

const QImage &QWaylandAbstractDecoration::contentImage()
{
   Q_D(QWaylandAbstractDecoration);

   if (d->m_isDirty) {
      // Update the decoration backingstore

      // pending implementation
   }

   return d->m_decorationContentImage;
}

void QWaylandAbstractDecoration::update()
{
   Q_D(QWaylandAbstractDecoration);
   d->m_isDirty = true;
}

void QWaylandAbstractDecoration::setMouseButtons(Qt::MouseButtons mb)
{
   Q_D(QWaylandAbstractDecoration);
   d->m_mouseButtons = mb;
}

void QWaylandAbstractDecoration::startResize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize resize, Qt::MouseButtons buttons)
{
   Q_D(QWaylandAbstractDecoration);

   if (isLeftClicked(buttons)) {
      d->m_wayland_window->topLevel()->resize(inputDevice, resize);
      inputDevice->removeMouseButtonFromState(Qt::LeftButton);
   }
}

void QWaylandAbstractDecoration::startMove(QWaylandInputDevice *inputDevice, Qt::MouseButtons buttons)
{
   Q_D(QWaylandAbstractDecoration);

   if (isLeftClicked(buttons)) {
      d->m_wayland_window->topLevel()->move(inputDevice);
      inputDevice->removeMouseButtonFromState(Qt::LeftButton);
   }
}

bool QWaylandAbstractDecoration::isLeftClicked(Qt::MouseButtons newMouseButtonState)
{
   Q_D(QWaylandAbstractDecoration);

   if (! (d->m_mouseButtons & Qt::LeftButton) && (newMouseButtonState & Qt::LeftButton)) {
      return true;
   }

   return false;
}

bool QWaylandAbstractDecoration::isLeftReleased(Qt::MouseButtons newMouseButtonState)
{
   Q_D(QWaylandAbstractDecoration);

   if ((d->m_mouseButtons & Qt::LeftButton) && ! (newMouseButtonState & Qt::LeftButton)) {
      return true;
   }

   return false;
}

bool QWaylandAbstractDecoration::isDirty() const
{
   Q_D(const QWaylandAbstractDecoration);
   return d->m_isDirty;
}

QWindow *QWaylandAbstractDecoration::window() const
{
   Q_D(const QWaylandAbstractDecoration);
   return d->m_window;
}

QWaylandWindow *QWaylandAbstractDecoration::waylandWindow() const
{
   Q_D(const QWaylandAbstractDecoration);
   return d->m_wayland_window;
}

}
