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

#ifndef QWAYLAND_SCREEN_H
#define QWAYLAND_SCREEN_H

#include <qplatform_screen.h>
#include <qwayland-wayland.h>

namespace QtWaylandClient {

class QWaylandCursor;
class QWaylandDisplay;

class Q_WAYLAND_CLIENT_EXPORT QWaylandScreen : public QPlatformScreen, QtWayland::wl_output
{
 public:
   QWaylandScreen(QWaylandDisplay *waylandDisplay, int version, uint32_t id);
   ~QWaylandScreen();

   QPlatformCursor *cursor() const override;

   int depth() const override;
   QWaylandDisplay *display() const;
   qreal devicePixelRatio() const override;

   QImage::Format format() const override;
   QRect geometry() const override;

   void init();

   QDpi logicalDpi() const override;

   QString name() const override {
      return m_outputName;
   }

   Qt::ScreenOrientation orientation() const override;

   uint32_t outputId() const {
      return m_outputId;
   }

   ::wl_output *output() {
      return object();
   }

   QSizeF physicalSize() const override;

   qreal refreshRate() const override;

   int scale() const;

   QList<QPlatformScreen *> virtualSiblings() const override;

   QWaylandCursor *waylandCursor() const {
      return m_waylandCursor;
   };

   static QWaylandScreen *waylandScreenFromWindow(QWindow *window);

 private:
   void output_mode(uint32_t flags, int width, int height, int refresh) override;

   void output_geometry(int32_t x, int32_t y, int32_t width, int32_t height, int subpixel, const QString &make,
         const QString &model, int32_t transform) override;

   void output_scale(int32_t factor) override;
   void output_done() override;

   int m_outputId;
   int m_scale;
   int m_depth;
   int m_refreshRate;
   int m_transform;

   bool m_initialized;

   QWaylandDisplay *m_waylandDisplay;
   QRect m_geometry;

   QImage::Format m_format;
   QSize m_physicalSize;
   QString m_outputName;
   Qt::ScreenOrientation m_orientation;

   QWaylandCursor *m_waylandCursor;
};

}

#endif
