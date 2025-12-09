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

#include <qwayland_bradient_decoration.h>

#include <qcursor.h>
#include <qpalette.h>
#include <qtextoption.h>
#include <qwindowsysteminterface.h>

#include <qwayland_shellsurface_p.h>
#include <qwayland_toplevel_p.h>
#include <qwayland_window_p.h>

namespace QtWaylandClient {

#define BUTTON_SPACING 5
#define BUTTON_WIDTH   10

static const char *const qt_close_xpm[] = {
   "10 10 2 1",
   "# c #000000",
   ". c None",
   "..........",
   ".##....##.",
   "..##..##..",
   "...####...",
   "....##....",
   "...####...",
   "..##..##..",
   ".##....##.",
   "..........",
   ".........."
};

static const char *const qt_maximize_xpm[] = {
   "10 10 2 1",
   "# c #000000",
   ". c None",
   "#########.",
   "#########.",
   "#.......#.",
   "#.......#.",
   "#.......#.",
   "#.......#.",
   "#.......#.",
   "#.......#.",
   "#########.",
   ".........."
};

static const char *const qt_minimize_xpm[] = {
   "10 10 2 1",
   "# c #000000",
   ". c None",
   "..........",
   "..........",
   "..........",
   "..........",
   "..........",
   "..........",
   "..........",
   ".#######..",
   ".#######..",
   ".........."
};

static const char *const qt_normalizeup_xpm[] = {
   "10 10 2 1",
   "# c #000000",
   ". c None",
   "...######.",
   "...######.",
   "...#....#.",
   ".######.#.",
   ".######.#.",
   ".#....###.",
   ".#....#...",
   ".#....#...",
   ".######...",
   ".........."
};

QWaylandBradientDecoration::QWaylandBradientDecoration()
   : QWaylandAbstractDecoration(), m_factor(1), m_clicking(Button::None)
{
   QPalette palette;
   m_foregroundColor = palette.color(QPalette::Active, QPalette::WindowText);
   m_backgroundColor = palette.color(QPalette::Active, QPalette::Window);

   QTextOption option(Qt::AlignHCenter | Qt::AlignVCenter);
   option.setWrapMode(QTextOption::NoWrap);

   m_windowTitle.setTextOption(option);
}

bool QWaylandBradientDecoration::clickButton(Qt::MouseButtons b, Button btn)
{
   if (isLeftClicked(b)) {
      m_clicking = btn;

   } else if (isLeftReleased(b)) {
      if (m_clicking == btn) {
         m_clicking = None;
         return true;

      } else {
         m_clicking = None;
      }
   }

   return false;
}

QRectF QWaylandBradientDecoration::closeButtonRect() const
{
   auto buttonSize = BUTTON_WIDTH * m_factor;

   return QRectF(window()->frameGeometry().width() - buttonSize - ((BUTTON_SPACING * 2) * m_factor),
         (margins().top() - buttonSize) / 2, buttonSize, buttonSize);
}

bool QWaylandBradientDecoration::handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
      Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
   (void) global;

   // Figure out what area mouse is in
   if (closeButtonRect().contains(local)) {
      if (clickButton(b, Close)) {
         QWindowSystemInterface::handleCloseEvent(window());
      }

   } else if (maximizeButtonRect().contains(local)) {
      if (clickButton(b, Maximize)) {
         window()->setWindowState(waylandWindow()->isMaximized() ? Qt::WindowNoState : Qt::WindowMaximized);
      }

   } else if (minimizeButtonRect().contains(local)) {
      if (clickButton(b, Minimize)) {
         window()->setWindowState(Qt::WindowMinimized);
      }

   } else if (local.y() <= margins().top()) {
      processMouseTop(inputDevice, local, b, mods);

   } else if (local.y() > window()->height() + margins().top()) {
      processMouseBottom(inputDevice, local, b, mods);

   } else if (local.x() <= margins().left()) {
      processMouseLeft(inputDevice, local, b, mods);

   } else if (local.x() > window()->width() + margins().left()) {
      processMouseRight(inputDevice, local, b, mods);

   } else {
      waylandWindow()->restoreMouseCursor(inputDevice);
      setMouseButtons(b);
      return false;
   }

   setMouseButtons(b);

   return true;
}

bool QWaylandBradientDecoration::handleTouch(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
      Qt::TouchPointState state, Qt::KeyboardModifiers mods)
{
   (void) inputDevice;
   (void) global;
   (void) mods;

   bool handled = (state == Qt::TouchPointPressed);

   if (handled) {
      if (closeButtonRect().contains(local)) {
         QWindowSystemInterface::handleCloseEvent(window());

      } else if (maximizeButtonRect().contains(local)) {
         window()->setWindowState(waylandWindow()->isMaximized() ? Qt::WindowNoState : Qt::WindowMaximized);

      } else if (minimizeButtonRect().contains(local)) {
         window()->setWindowState(Qt::WindowMinimized);

      } else if (local.y() <= margins().top()) {
         waylandWindow()->shellSurface()->topLevel()->move(inputDevice);

      } else {
         handled = false;
      }
   }

   return handled;
}

QMargins QWaylandBradientDecoration::margins() const
{
   return QMargins(3 * m_factor, 30 * m_factor, 3 * m_factor, 3 * m_factor);
}

QRectF QWaylandBradientDecoration::minimizeButtonRect() const
{
   auto buttonSize = BUTTON_WIDTH * m_factor;

   return QRectF(window()->frameGeometry().width() - buttonSize * 3 - ((BUTTON_SPACING * 4) * m_factor),
         (margins().top() - buttonSize) / 2, buttonSize, buttonSize);
}

QRectF QWaylandBradientDecoration::maximizeButtonRect() const
{
   auto buttonSize = BUTTON_WIDTH * m_factor;

   return QRectF(window()->frameGeometry().width() - buttonSize * 2 - ((BUTTON_SPACING * 3) * m_factor),
         (margins().top() - buttonSize) / 2, buttonSize, buttonSize);
}

void QWaylandBradientDecoration::paint(QPaintDevice *device)
{
   // pending implementation
}

void QWaylandBradientDecoration::processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,
      Qt::KeyboardModifiers mods)
{
   (void) mods;

   if (local.y() <= margins().bottom()) {
      if (local.x() <= margins().left()) {
         //top left bit
         waylandWindow()->setMouseCursor(inputDevice, Qt::SizeFDiagCursor);
         startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_TOP_LEFT, b);

      } else if (local.x() > window()->width() + margins().left()) {
         //top right bit
         waylandWindow()->setMouseCursor(inputDevice, Qt::SizeBDiagCursor);
         startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_TOP_RIGHT, b);

      } else {
         //top reszie bit
         waylandWindow()->setMouseCursor(inputDevice, Qt::SplitVCursor);
         startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_TOP, b);
      }

   } else {
      waylandWindow()->restoreMouseCursor(inputDevice);
      startMove(inputDevice, b);
   }
}

void QWaylandBradientDecoration::processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local,
      Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
   (void) mods;

   if (local.x() <= margins().left()) {
      // bottom left bit
      waylandWindow()->setMouseCursor(inputDevice, Qt::SizeBDiagCursor);
      startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM_LEFT, b);

   } else if (local.x() > window()->width() + margins().left()) {
      // bottom right bit
      waylandWindow()->setMouseCursor(inputDevice, Qt::SizeFDiagCursor);
      startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT, b);

   } else {
      // bottom bit
      waylandWindow()->setMouseCursor(inputDevice, Qt::SplitVCursor);
      startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM, b);
   }
}

void QWaylandBradientDecoration::processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local,
      Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
   (void) local;
   (void) mods;

   waylandWindow()->setMouseCursor(inputDevice, Qt::SplitHCursor);
   startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_LEFT, b);
}

void QWaylandBradientDecoration::processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local,
      Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
   (void) local;
   (void) mods;

   waylandWindow()->setMouseCursor(inputDevice, Qt::SplitHCursor);
   startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_RIGHT, b);
}

}
