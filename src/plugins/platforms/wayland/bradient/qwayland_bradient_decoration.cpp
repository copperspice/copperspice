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
#include <qlineargradient.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qsizef.h>
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
   m_factor = std::lround((waylandWindow()->screen()->logicalDpi().first)/96.0);

   QRect surfaceRect(QPoint(), window()->frameGeometry().size());

   QRect clips[] = {
      QRect(0, 0, surfaceRect.width(), margins().top()),
      QRect(0, surfaceRect.height() - margins().bottom(), surfaceRect.width(), margins().bottom()),
      QRect(0, margins().top(), margins().left(), surfaceRect.height() - margins().top() - margins().bottom()),
      QRect(surfaceRect.width() - margins().right(), margins().top(), margins().left(),
      surfaceRect.height() - margins().top() - margins().bottom())
   };

   QRect top = clips[0];

   QPainter p(device);
   p.setRenderHint(QPainter::Antialiasing);

   // Title bar
   QPoint gradCenter(top.center() + QPoint(30, 60));
   QLinearGradient grad(top.topLeft(), top.bottomLeft());

   QColor base(m_backgroundColor);
   grad.setColorAt(0, base.lighter(100));
   grad.setColorAt(1, base.darker(120));

   QPainterPath roundedRect;
   roundedRect.addRoundedRect(surfaceRect, 6, 6);

   for (int i = 0; i < 4; ++i) {
      p.save();
      p.setClipRect(clips[i]);
      p.fillPath(roundedRect, grad);
      p.restore();
   }

   // Window icon
   QIcon icon = waylandWindow()->windowIcon();

   if (! icon.isNull()) {
      QPixmap pixmap = icon.pixmap(QSize(128, 128));
      QPixmap scaled = pixmap.scaled(22, 22, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

      QRectF iconRect(0, 0, 22, 22);
      p.drawPixmap(iconRect.adjusted(margins().left() + BUTTON_SPACING, 4,
         margins().left() + BUTTON_SPACING, 4), scaled, iconRect);
   }

   // Window title
   QString windowTitleText = window()->title();

   if (! windowTitleText.isEmpty()) {
      if (m_windowTitle.text() != windowTitleText) {
         m_windowTitle.setText(windowTitleText);
         m_windowTitle.prepare();
      }

      QRect titleBar = top;
      titleBar.setLeft(margins().left() + BUTTON_SPACING + (icon.isNull() ? 0 : 22 + BUTTON_SPACING));
      titleBar.setRight(minimizeButtonRect().left() - BUTTON_SPACING);

      p.save();
      p.setClipRect(titleBar);
      p.setPen(m_foregroundColor);

      QSizeF size = m_windowTitle.size();
      int dx = (top.width() - size.width()) / 2;
      int dy = (top.height() - size.height()) / 2;

      QFont font = p.font();
      font.setBold(true);
      p.setFont(font);

      QPoint windowTitlePoint(top.topLeft().x() + dx, top.topLeft().y() + dy);
      p.drawStaticText(windowTitlePoint, m_windowTitle);
      p.restore();
   }

   p.save();

   // close button
   QPixmap closePixmap(qt_close_xpm);
   p.drawPixmap(closeButtonRect(), closePixmap, closePixmap.rect());

   // maximize button
   QPixmap maximizePixmap(waylandWindow()->isMaximized() ? qt_normalizeup_xpm : qt_maximize_xpm);
   p.drawPixmap(maximizeButtonRect(), maximizePixmap, maximizePixmap.rect());

   // minimize button
   QPixmap minimizePixmap(qt_minimize_xpm);
   p.drawPixmap(minimizeButtonRect(), minimizePixmap, minimizePixmap.rect());

   p.restore();
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
