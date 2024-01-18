/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include <qshapedpixmapdndwindow_p.h>

#include <qpainter.h>
#include <qcursor.h>
#include <qguiapplication.h>
#include <qpalette.h>
#include <qbitmap.h>

QShapedPixmapWindow::QShapedPixmapWindow(QScreen *screen)
   : m_useCompositing(true)
{
   setScreen(screen);
   QSurfaceFormat format;
   format.setAlphaBufferSize(8);
   setFormat(format);
   setFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint
      | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus);
}

QShapedPixmapWindow::~QShapedPixmapWindow()
{
}

void QShapedPixmapWindow::setPixmap(const QPixmap &pixmap)
{
   m_pixmap = pixmap;
   if (!m_useCompositing) {
      const QBitmap mask = m_pixmap.mask();
      if (!mask.isNull()) {
         if (!handle()) {
            create();
         }
         setMask(mask);
      }
   }
}

void QShapedPixmapWindow::setHotspot(const QPoint &hotspot)
{
   m_hotSpot = hotspot;
}

void QShapedPixmapWindow::paintEvent(QPaintEvent *)
{
   if (!m_pixmap.isNull()) {
      const QRect rect(QPoint(0, 0), size());
      QPainter painter(this);
      if (m_useCompositing) {
         painter.setCompositionMode(QPainter::CompositionMode_Source);
      } else {
         painter.fillRect(rect, QGuiApplication::palette().base());
      }
      painter.drawPixmap(rect, m_pixmap);
   }
}

void QShapedPixmapWindow::updateGeometry(const QPoint &pos)
{
   QSize size(1, 1);
   if (!m_pixmap.isNull()) {
      size = qFuzzyCompare(m_pixmap.devicePixelRatio(), qreal(1.0))
         ? m_pixmap.size()
         : (QSizeF(m_pixmap.size()) / m_pixmap.devicePixelRatio()).toSize();
   }
   setGeometry(QRect(pos - m_hotSpot, size));
}


