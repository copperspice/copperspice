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

#include <qwayland_screen_p.h>

#include <qapplication.h>
#include <qplatform_window.h>
#include <qwindowsysteminterface.h>

#include <qwayland_cursor_p.h>
#include <qwayland_display_p.h>
#include <qwayland_window_p.h>

namespace QtWaylandClient {

QWaylandScreen::QWaylandScreen(QWaylandDisplay *waylandDisplay, int version, uint32_t id)
   : QtWayland::wl_output(waylandDisplay->wl_registry(), id, qMin(version, 2)),
     m_outputId(id), m_scale(1), m_depth(32), m_refreshRate(60000), m_transform(-1), m_initialized(false),
     m_waylandDisplay(waylandDisplay), m_format(QImage::Format_ARGB32_Premultiplied),
     m_outputName(QString("Screen%1").formatArg(id)), m_orientation(Qt::PrimaryOrientation),
     m_waylandCursor(nullptr)
{
}

QWaylandScreen::~QWaylandScreen()
{
   delete m_waylandCursor;
}

void QWaylandScreen::init()
{
   m_waylandCursor = new QWaylandCursor(this);

   m_initialized = true;
   m_waylandDisplay->handleScreenInitialized(this);
}

QPlatformCursor *QWaylandScreen::cursor() const
{
   return m_waylandCursor;
}

int QWaylandScreen::depth() const
{
   return m_depth;
}

qreal QWaylandScreen::devicePixelRatio() const
{
   return qreal(m_scale);
}

QWaylandDisplay *QWaylandScreen::display() const
{
   return m_waylandDisplay;
}

QImage::Format QWaylandScreen::format() const
{
   return m_format;
}

QRect QWaylandScreen::geometry() const
{
   // Scale geometry for QScreen, puts the window and screen
   // geometry in the same coordinate system

   return QRect(m_geometry.topLeft(), m_geometry.size() / m_scale);
}

QDpi QWaylandScreen::logicalDpi() const
{
   static int force_dpi = qgetenv("QT_WAYLAND_FORCE_DPI").toInt();

   if (force_dpi > 0) {
      return QDpi(force_dpi, force_dpi);
   }

   return QPlatformScreen::logicalDpi();
}

Qt::ScreenOrientation QWaylandScreen::orientation() const
{
   return m_orientation;
}

void QWaylandScreen::output_geometry(int32_t x, int32_t y, int32_t width, int32_t height,
      int subpixel, const QString &make, const QString &model, int32_t transform)
{
   (void) subpixel;
   (void) make;

   m_transform = transform;

   if (! model.isEmpty()) {
      m_outputName = model;
   }

   m_physicalSize = QSize(width, height);
   m_geometry.moveTopLeft(QPoint(x, y));
}

void QWaylandScreen::output_mode(uint32_t flags, int width, int height, int refresh)
{
   if (! (flags & WL_OUTPUT_MODE_CURRENT)) {
      return;
   }

   QSize size(width, height);

   if (size != m_geometry.size()) {
      m_geometry.setSize(size);
   }

   if (refresh != m_refreshRate) {
      m_refreshRate = refresh;
   }
}

void QWaylandScreen::output_scale(int32_t factor)
{
   m_scale = factor;
}

QSizeF QWaylandScreen::physicalSize() const
{
   if (m_physicalSize.isEmpty()) {
      return QPlatformScreen::physicalSize();
   } else {
      return m_physicalSize;
   }
}

qreal QWaylandScreen::refreshRate() const
{
   return m_refreshRate / 1000.0f;
}

int QWaylandScreen::scale() const
{
   return m_scale;
}

QWaylandScreen *QWaylandScreen::waylandScreenFromWindow(QWindow *window)
{
   QPlatformScreen *platformScreen = QPlatformScreen::platformScreenForWindow(window);
   return static_cast<QWaylandScreen *> (platformScreen);
}

void QWaylandScreen::output_done()
{
   if (! m_initialized) {
      init();
   }

   if (m_transform >= 0) {
      bool isPortrait = m_geometry.height() > m_geometry.width();

      switch (m_transform) {
         case WL_OUTPUT_TRANSFORM_NORMAL:
            m_orientation = isPortrait ? Qt::PortraitOrientation : Qt::LandscapeOrientation;
            break;

         case WL_OUTPUT_TRANSFORM_90:
            m_orientation = isPortrait ? Qt::InvertedLandscapeOrientation : Qt::PortraitOrientation;
            break;

         case WL_OUTPUT_TRANSFORM_180:
            m_orientation = isPortrait ? Qt::InvertedPortraitOrientation : Qt::InvertedLandscapeOrientation;
            break;

         case WL_OUTPUT_TRANSFORM_270:
            m_orientation = isPortrait ? Qt::LandscapeOrientation : Qt::InvertedPortraitOrientation;
            break;

         // Ignore these ones for now
         case WL_OUTPUT_TRANSFORM_FLIPPED:
         case WL_OUTPUT_TRANSFORM_FLIPPED_90:
         case WL_OUTPUT_TRANSFORM_FLIPPED_180:
         case WL_OUTPUT_TRANSFORM_FLIPPED_270:
            break;
      }

      QWindowSystemInterface::handleScreenOrientationChange(screen(), m_orientation);
      m_transform = -1;
   }

   QWindowSystemInterface::handleScreenRefreshRateChange(screen(), refreshRate());
   QWindowSystemInterface::handleScreenGeometryChange(screen(), m_geometry, m_geometry);
}

QList<QPlatformScreen *> QWaylandScreen::virtualSiblings() const
{
   QList<QPlatformScreen *> retval;

   const QList<QWaylandScreen *> screens = m_waylandDisplay->screens();

   for (QWaylandScreen *item : screens) {
      if (item->screen()) {
         retval.append(item);
      }
   }

   return retval;
}

}
