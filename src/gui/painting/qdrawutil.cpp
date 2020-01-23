/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qdrawutil.h>
#include <qbitmap.h>
#include <qpixmapcache.h>

#include <qpainter.h>
#include <qpalette.h>
#include <qpaintengineex_p.h>
#include <qvarlengtharray.h>
#include <qmath.h>
#include <qhexstring_p.h>

void qDrawShadeLine(QPainter *p, int x1, int y1, int x2, int y2, const QPalette &pal,
   bool sunken, int lineWidth, int midLineWidth)
{
   if (!(p && lineWidth >= 0 && midLineWidth >= 0))        {
      qWarning("qDrawShadeLine: Invalid parameters");
      return;
   }

   int tlw = lineWidth * 2 + midLineWidth;        // total line width
   QPen oldPen = p->pen();                        // save pen

   if (sunken) {
      p->setPen(pal.color(QPalette::Dark));
   } else {
      p->setPen(pal.light().color());
   }

   QPolygon a;
   int i;
   if (y1 == y2) {                                // horizontal line
      int y = y1 - tlw / 2;
      if (x1 > x2) {                             // swap x1 and x2
         int t = x1;
         x1 = x2;
         x2 = t;
      }
      x2--;
      for (i = 0; i < lineWidth; i++) {            // draw top shadow
         a.setPoints(3, x1 + i, y + tlw - 1 - i,
            x1 + i, y + i,
            x2 - i, y + i);
         p->drawPolyline(a);
      }
      if (midLineWidth > 0) {
         p->setPen(pal.mid().color());
         for (i = 0; i < midLineWidth; i++)    // draw lines in the middle
            p->drawLine(x1 + lineWidth, y + lineWidth + i,
               x2 - lineWidth, y + lineWidth + i);
      }
      if (sunken) {
         p->setPen(pal.light().color());
      } else {
         p->setPen(pal.dark().color());
      }
      for (i = 0; i < lineWidth; i++) {            // draw bottom shadow
         a.setPoints(3, x1 + i, y + tlw - i - 1,
            x2 - i, y + tlw - i - 1,
            x2 - i, y + i + 1);
         p->drawPolyline(a);
      }
   } else if (x1 == x2) {                      // vertical line
      int x = x1 - tlw / 2;
      if (y1 > y2) {                        // swap y1 and y2
         int t = y1;
         y1 = y2;
         y2 = t;
      }
      y2--;
      for (i = 0; i < lineWidth; i++) {            // draw left shadow
         a.setPoints(3, x + i, y2,
            x + i, y1 + i,
            x + tlw - 1, y1 + i);
         p->drawPolyline(a);
      }
      if (midLineWidth > 0) {
         p->setPen(pal.mid().color());

         for (i = 0; i < midLineWidth; i++) {
            // draw lines in the middle
            p->drawLine(x + lineWidth + i, y1 + lineWidth, x + lineWidth + i, y2);
         }
      }

      if (sunken) {
         p->setPen(pal.light().color());
      } else {
         p->setPen(pal.dark().color());
      }

      for (i = 0; i < lineWidth; i++) {            // draw right shadow
         a.setPoints(3, x + lineWidth, y2 - i,
            x + tlw - i - 1, y2 - i,
            x + tlw - i - 1, y1 + lineWidth);
         p->drawPolyline(a);
      }
   }
   p->setPen(oldPen);
}


void qDrawShadeRect(QPainter *p, int x, int y, int w, int h,
   const QPalette &pal, bool sunken, int lineWidth, int midLineWidth, const QBrush *fill)
{
   if (w == 0 || h == 0) {
      return;
   }

   if (! (w > 0 && h > 0 && lineWidth >= 0 && midLineWidth >= 0)) {
      qWarning("qDrawShadeRect: Invalid parameters");
      return;
   }

   QPen oldPen = p->pen();
   if (sunken) {
      p->setPen(pal.dark().color());
   } else {
      p->setPen(pal.light().color());
   }

   int x1 = x, y1 = y, x2 = x + w - 1, y2 = y + h - 1;

   if (lineWidth == 1 && midLineWidth == 0) {// standard shade rectangle
      p->drawRect(x1, y1, w - 2, h - 2);
      if (sunken) {
         p->setPen(pal.light().color());
      } else {
         p->setPen(pal.dark().color());
      }
      QLineF lines[4] = { QLineF(x1 + 1, y1 + 1, x2 - 2, y1 + 1),
            QLineF(x1 + 1, y1 + 2, x1 + 1, y2 - 2),
            QLineF(x1, y2, x2, y2),
            QLineF(x2, y1, x2, y2 - 1)
         };
      p->drawLines(lines, 4);              // draw bottom/right lines
   } else {                                        // more complicated
      int m = lineWidth + midLineWidth;
      int i, j = 0, k = m;
      for (i = 0; i < lineWidth; i++) {            // draw top shadow
         QLineF lines[4] = { QLineF(x1 + i, y2 - i, x1 + i, y1 + i),
               QLineF(x1 + i, y1 + i, x2 - i, y1 + i),
               QLineF(x1 + k, y2 - k, x2 - k, y2 - k),
               QLineF(x2 - k, y2 - k, x2 - k, y1 + k)
            };
         p->drawLines(lines, 4);
         k++;
      }
      p->setPen(pal.mid().color());
      j = lineWidth * 2;

      for (i = 0; i < midLineWidth; i++) {    // draw lines in the middle
         p->drawRect(x1 + lineWidth + i, y1 + lineWidth + i, w - j - 1, h - j - 1);
         j += 2;
      }

      if (sunken) {
         p->setPen(pal.light().color());
      } else {
         p->setPen(pal.dark().color());
      }

      k = m;
      for (i = 0; i < lineWidth; i++) {            // draw bottom shadow
         QLineF lines[4] = { QLineF(x1 + 1 + i, y2 - i, x2 - i, y2 - i),
               QLineF(x2 - i, y2 - i, x2 - i, y1 + i + 1),
               QLineF(x1 + k, y2 - k, x1 + k, y1 + k),
               QLineF(x1 + k, y1 + k, x2 - k, y1 + k)
            };
         p->drawLines(lines, 4);
         k++;
      }
   }

   if (fill) {
      QBrush oldBrush = p->brush();
      int tlw = lineWidth + midLineWidth;
      p->setPen(Qt::NoPen);
      p->setBrush(*fill);
      p->drawRect(x + tlw, y + tlw, w - 2 * tlw, h - 2 * tlw);
      p->setBrush(oldBrush);
   }

   p->setPen(oldPen);                        // restore pen
}

void qDrawShadePanel(QPainter *p, int x, int y, int w, int h,
   const QPalette &pal, bool sunken, int lineWidth, const QBrush *fill)
{
   if (w == 0 || h == 0) {
      return;
   }

   if (! (w > 0 && h > 0 && lineWidth >= 0)) {
      qWarning("qDrawShadePanel: Invalid parameters");
   }

   QColor shade = pal.dark().color();
   QColor light = pal.light().color();
   if (fill) {
      if (fill->color() == shade) {
         shade = pal.shadow().color();
      }
      if (fill->color() == light) {
         light = pal.midlight().color();
      }
   }

   QPen oldPen = p->pen();                        // save pen
   QVector<QLineF> lines;
   lines.reserve(2 * lineWidth);

   if (sunken) {
      p->setPen(shade);
   } else {
      p->setPen(light);
   }

   int x1, y1, x2, y2;
   int i;
   x1 = x;
   y1 = y2 = y;
   x2 = x + w - 2;
   for (i = 0; i < lineWidth; i++) {            // top shadow
      lines << QLineF(x1, y1++, x2--, y2++);
   }
   x2 = x1;
   y1 = y + h - 2;
   for (i = 0; i < lineWidth; i++) {            // left shado
      lines << QLineF(x1++, y1, x2++, y2--);
   }
   p->drawLines(lines);
   lines.clear();

   if (sunken) {
      p->setPen(light);
   } else {
      p->setPen(shade);
   }

   x1 = x;
   y1 = y2 = y + h - 1;
   x2 = x + w - 1;
   for (i = 0; i < lineWidth; i++) {            // bottom shadow
      lines << QLineF(x1++, y1--, x2, y2--);
   }
   x1 = x2;
   y1 = y;
   y2 = y + h - lineWidth - 1;
   for (i = 0; i < lineWidth; i++) {            // right shadow
      lines << QLineF(x1--, y1++, x2--, y2);
   }
   p->drawLines(lines);
   if (fill) {                              // fill with fill color
      p->fillRect(x + lineWidth, y + lineWidth, w - lineWidth * 2, h - lineWidth * 2, *fill);
   }
   p->setPen(oldPen);                        // restore pen
}


/*!
  \internal
  This function draws a rectangle with two pixel line width.
  It is called from qDrawWinButton() and qDrawWinPanel().

  c1..c4 and fill are used:

    1 1 1 1 1 2
    1 3 3 3 4 2
    1 3 F F 4 2
    1 3 F F 4 2
    1 4 4 4 4 2
    2 2 2 2 2 2
*/

static void qDrawWinShades(QPainter *p,
   int x, int y, int w, int h,
   const QColor &c1, const QColor &c2,
   const QColor &c3, const QColor &c4,
   const QBrush *fill)
{
   if (w < 2 || h < 2) {                      // can't do anything with that
      return;
   }

   QPen oldPen = p->pen();
   QPoint a[3] = { QPoint(x, y + h - 2), QPoint(x, y), QPoint(x + w - 2, y) };
   p->setPen(c1);
   p->drawPolyline(a, 3);
   QPoint b[3] = { QPoint(x, y + h - 1), QPoint(x + w - 1, y + h - 1), QPoint(x + w - 1, y) };
   p->setPen(c2);
   p->drawPolyline(b, 3);
   if (w > 4 && h > 4) {
      QPoint c[3] = { QPoint(x + 1, y + h - 3), QPoint(x + 1, y + 1), QPoint(x + w - 3, y + 1) };
      p->setPen(c3);
      p->drawPolyline(c, 3);
      QPoint d[3] = { QPoint(x + 1, y + h - 2), QPoint(x + w - 2, y + h - 2), QPoint(x + w - 2, y + 1) };
      p->setPen(c4);
      p->drawPolyline(d, 3);

      if (fill) {
         p->fillRect(QRect(x + 2, y + 2, w - 4, h - 4), *fill);
      }
   }
   p->setPen(oldPen);
}


void qDrawWinButton(QPainter *p, int x, int y, int w, int h,
   const QPalette &pal, bool sunken,
   const QBrush *fill)
{
   if (sunken)
      qDrawWinShades(p, x, y, w, h,
         pal.shadow().color(), pal.light().color(), pal.dark().color(),
         pal.button().color(), fill);
   else
      qDrawWinShades(p, x, y, w, h,
         pal.light().color(), pal.shadow().color(), pal.button().color(),
         pal.dark().color(), fill);
}

void qDrawWinPanel(QPainter *p, int x, int y, int w, int h,
   const QPalette &pal, bool        sunken,
   const QBrush *fill)
{
   if (sunken)
      qDrawWinShades(p, x, y, w, h,
         pal.dark().color(), pal.light().color(), pal.shadow().color(),
         pal.midlight().color(), fill);
   else
      qDrawWinShades(p, x, y, w, h,
         pal.light().color(), pal.shadow().color(), pal.midlight().color(),
         pal.dark().color(), fill);
}

void qDrawPlainRect(QPainter *p, int x, int y, int w, int h, const QColor &c,
   int lineWidth, const QBrush *fill)
{
   if (w == 0 || h == 0) {
      return;
   }

   if (!(w > 0 && h > 0 && lineWidth >= 0)) {
      qWarning("qDrawPlainRect: Invalid parameters");
   }
   QPen   oldPen   = p->pen();
   QBrush oldBrush = p->brush();
   p->setPen(c);
   p->setBrush(Qt::NoBrush);

   for (int i = 0; i < lineWidth; i++) {
      p->drawRect(x + i, y + i, w - i * 2 - 1, h - i * 2 - 1);
   }

   if (fill) {                                // fill with fill color
      p->setPen(Qt::NoPen);
      p->setBrush(*fill);
      p->drawRect(x + lineWidth, y + lineWidth, w - lineWidth * 2, h - lineWidth * 2);
   }
   p->setPen(oldPen);
   p->setBrush(oldBrush);
}


void qDrawShadeLine(QPainter *p, const QPoint &p1, const QPoint &p2,
   const QPalette &pal, bool sunken,
   int lineWidth, int midLineWidth)
{
   qDrawShadeLine(p, p1.x(), p1.y(), p2.x(), p2.y(), pal, sunken,
      lineWidth, midLineWidth);
}


void qDrawShadeRect(QPainter *p, const QRect &r,
   const QPalette &pal, bool sunken,
   int lineWidth, int midLineWidth,
   const QBrush *fill)
{
   qDrawShadeRect(p, r.x(), r.y(), r.width(), r.height(), pal, sunken,
      lineWidth, midLineWidth, fill);
}


void qDrawShadePanel(QPainter *p, const QRect &r,
   const QPalette &pal, bool sunken,
   int lineWidth, const QBrush *fill)
{
   qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(), pal, sunken,
      lineWidth, fill);
}


void qDrawWinButton(QPainter *p, const QRect &r,
   const QPalette &pal, bool sunken, const QBrush *fill)
{
   qDrawWinButton(p, r.x(), r.y(), r.width(), r.height(), pal, sunken, fill);
}

void qDrawWinPanel(QPainter *p, const QRect &r,
   const QPalette &pal, bool sunken, const QBrush *fill)
{
   qDrawWinPanel(p, r.x(), r.y(), r.width(), r.height(), pal, sunken, fill);
}

void qDrawPlainRect(QPainter *p, const QRect &r, const QColor &c,
   int lineWidth, const QBrush *fill)
{
   qDrawPlainRect(p, r.x(), r.y(), r.width(), r.height(), c,
      lineWidth, fill);
}


typedef QVarLengthArray<QPainter::PixmapFragment, 16> QPixmapFragmentsArray;



void qDrawBorderPixmap(QPainter *painter, const QRect &targetRect, const QMargins &targetMargins,
   const QPixmap &pixmap, const QRect &sourceRect, const QMargins &sourceMargins,
   const QTileRules &rules, QDrawBorderPixmap::DrawingHints hints)
{
   QPainter::PixmapFragment d;
   d.opacity = 1.0;
   d.rotation = 0.0;

   QPixmapFragmentsArray opaqueData;
   QPixmapFragmentsArray translucentData;

   // source center
   const int sourceCenterTop = sourceRect.top() + sourceMargins.top();
   const int sourceCenterLeft = sourceRect.left() + sourceMargins.left();
   const int sourceCenterBottom = sourceRect.bottom() - sourceMargins.bottom() + 1;
   const int sourceCenterRight = sourceRect.right() - sourceMargins.right() + 1;
   const int sourceCenterWidth = sourceCenterRight - sourceCenterLeft;
   const int sourceCenterHeight = sourceCenterBottom - sourceCenterTop;

   // target center
   const int targetCenterTop = targetRect.top() + targetMargins.top();
   const int targetCenterLeft = targetRect.left() + targetMargins.left();
   const int targetCenterBottom = targetRect.bottom() - targetMargins.bottom() + 1;
   const int targetCenterRight = targetRect.right() - targetMargins.right() + 1;
   const int targetCenterWidth = targetCenterRight - targetCenterLeft;
   const int targetCenterHeight = targetCenterBottom - targetCenterTop;

   QVarLengthArray<qreal, 16> xTarget; // x-coordinates of target rectangles
   QVarLengthArray<qreal, 16> yTarget; // y-coordinates of target rectangles

   int columns = 3;
   int rows = 3;
   if (rules.horizontal != Qt::StretchTile && sourceCenterWidth != 0) {
      columns = qMax(3, 2 + qCeil(targetCenterWidth / qreal(sourceCenterWidth)));
   }

   if (rules.vertical != Qt::StretchTile && sourceCenterHeight != 0) {
      rows = qMax(3, 2 + qCeil(targetCenterHeight / qreal(sourceCenterHeight)));
   }

   xTarget.resize(columns + 1);
   yTarget.resize(rows + 1);

   bool oldAA = painter->testRenderHint(QPainter::Antialiasing);
   if (painter->paintEngine()->type() != QPaintEngine::OpenGL
      && painter->paintEngine()->type() != QPaintEngine::OpenGL2
      && oldAA && painter->combinedTransform().type() != QTransform::TxNone) {
      painter->setRenderHint(QPainter::Antialiasing, false);
   }

   xTarget[0] = targetRect.left();
   xTarget[1] = targetCenterLeft;
   xTarget[columns - 1] = targetCenterRight;
   xTarget[columns] = targetRect.left() + targetRect.width();

   yTarget[0] = targetRect.top();
   yTarget[1] = targetCenterTop;
   yTarget[rows - 1] = targetCenterBottom;
   yTarget[rows] = targetRect.top() + targetRect.height();

   qreal dx = targetCenterWidth;
   qreal dy = targetCenterHeight;

   switch (rules.horizontal) {
      case Qt::StretchTile:
         dx = targetCenterWidth;
         break;
      case Qt::RepeatTile:
         dx = sourceCenterWidth;
         break;
      case Qt::RoundTile:
         dx = targetCenterWidth / qreal(columns - 2);
         break;
   }

   for (int i = 2; i < columns - 1; ++i) {
      xTarget[i] = xTarget[i - 1] + dx;
   }

   switch (rules.vertical) {
      case Qt::StretchTile:
         dy = targetCenterHeight;
         break;
      case Qt::RepeatTile:
         dy = sourceCenterHeight;
         break;
      case Qt::RoundTile:
         dy = targetCenterHeight / qreal(rows - 2);
         break;
   }

   for (int i = 2; i < rows - 1; ++i) {
      yTarget[i] = yTarget[i - 1] + dy;
   }

   // corners
   if (targetMargins.top() > 0 && targetMargins.left() > 0 && sourceMargins.top() > 0 && sourceMargins.left() > 0) { // top left
      d.x = (0.5 * (xTarget[1] + xTarget[0]));
      d.y = (0.5 * (yTarget[1] + yTarget[0]));
      d.sourceLeft = sourceRect.left();
      d.sourceTop = sourceRect.top();
      d.width = sourceMargins.left();
      d.height = sourceMargins.top();
      d.scaleX = qreal(xTarget[1] - xTarget[0]) / d.width;
      d.scaleY = qreal(yTarget[1] - yTarget[0]) / d.height;
      if (hints & QDrawBorderPixmap::OpaqueTopLeft) {
         opaqueData.append(d);
      } else {
         translucentData.append(d);
      }
   }

   if (targetMargins.top() > 0 && targetMargins.right() > 0 && sourceMargins.top() > 0 && sourceMargins.right() > 0) { // top right
      d.x = (0.5 * (xTarget[columns] + xTarget[columns - 1]));
      d.y = (0.5 * (yTarget[1] + yTarget[0]));
      d.sourceLeft = sourceCenterRight;
      d.sourceTop = sourceRect.top();
      d.width = sourceMargins.right();
      d.height = sourceMargins.top();
      d.scaleX = qreal(xTarget[columns] - xTarget[columns - 1]) / d.width;
      d.scaleY = qreal(yTarget[1] - yTarget[0]) / d.height;
      if (hints & QDrawBorderPixmap::OpaqueTopRight) {
         opaqueData.append(d);
      } else {
         translucentData.append(d);
      }
   }

   if (targetMargins.bottom() > 0 && targetMargins.left() > 0 && sourceMargins.bottom() > 0 && sourceMargins.left() > 0) { // bottom left
      d.x = (0.5 * (xTarget[1] + xTarget[0]));
      d.y = (0.5 * (yTarget[rows] + yTarget[rows - 1]));
      d.sourceLeft = sourceRect.left();
      d.sourceTop = sourceCenterBottom;
      d.width = sourceMargins.left();
      d.height = sourceMargins.bottom();
      d.scaleX = qreal(xTarget[1] - xTarget[0]) / d.width;
      d.scaleY = qreal(yTarget[rows] - yTarget[rows - 1]) / d.height;
      if (hints & QDrawBorderPixmap::OpaqueBottomLeft) {
         opaqueData.append(d);
      } else {
         translucentData.append(d);
      }
   }
   if (targetMargins.bottom() > 0 && targetMargins.right() > 0 && sourceMargins.bottom() > 0 &&
      sourceMargins.right() > 0) { // bottom right
      d.x = (0.5 * (xTarget[columns] + xTarget[columns - 1]));
      d.y = (0.5 * (yTarget[rows] + yTarget[rows - 1]));
      d.sourceLeft = sourceCenterRight;
      d.sourceTop = sourceCenterBottom;
      d.width = sourceMargins.right();
      d.height = sourceMargins.bottom();
      d.scaleX = qreal(xTarget[columns] - xTarget[columns - 1]) / d.width;
      d.scaleY = qreal(yTarget[rows] - yTarget[rows - 1]) / d.height;
      if (hints & QDrawBorderPixmap::OpaqueBottomRight) {
         opaqueData.append(d);
      } else {
         translucentData.append(d);
      }
   }

   // horizontal edges
   if (targetCenterWidth > 0 && sourceCenterWidth > 0) {
      if (targetMargins.top() > 0 && sourceMargins.top() > 0) { // top
         QPixmapFragmentsArray &data = hints & QDrawBorderPixmap::OpaqueTop ? opaqueData : translucentData;
         d.sourceLeft = sourceCenterLeft;
         d.sourceTop = sourceRect.top();
         d.width = sourceCenterWidth;
         d.height = sourceMargins.top();
         d.y = (0.5 * (yTarget[1] + yTarget[0]));
         d.scaleX = dx / d.width;
         d.scaleY = qreal(yTarget[1] - yTarget[0]) / d.height;
         for (int i = 1; i < columns - 1; ++i) {
            d.x = (0.5 * (xTarget[i + 1] + xTarget[i]));
            data.append(d);
         }
         if (rules.horizontal == Qt::RepeatTile) {
            data[data.size() - 1].width = ((xTarget[columns - 1] - xTarget[columns - 2]) / d.scaleX);
         }
      }
      if (targetMargins.bottom() > 0 && sourceMargins.bottom() > 0) { // bottom
         QPixmapFragmentsArray &data = hints & QDrawBorderPixmap::OpaqueBottom ? opaqueData : translucentData;
         d.sourceLeft = sourceCenterLeft;
         d.sourceTop = sourceCenterBottom;
         d.width = sourceCenterWidth;
         d.height = sourceMargins.bottom();
         d.y = (0.5 * (yTarget[rows] + yTarget[rows - 1]));
         d.scaleX = dx / d.width;
         d.scaleY = qreal(yTarget[rows] - yTarget[rows - 1]) / d.height;
         for (int i = 1; i < columns - 1; ++i) {
            d.x = (0.5 * (xTarget[i + 1] + xTarget[i]));
            data.append(d);
         }
         if (rules.horizontal == Qt::RepeatTile) {
            data[data.size() - 1].width = ((xTarget[columns - 1] - xTarget[columns - 2]) / d.scaleX);
         }
      }
   }

   // vertical edges
   if (targetCenterHeight > 0 && sourceCenterHeight > 0) {
      if (targetMargins.left() > 0 && sourceMargins.left() > 0) { // left
         QPixmapFragmentsArray &data = hints & QDrawBorderPixmap::OpaqueLeft ? opaqueData : translucentData;
         d.sourceLeft = sourceRect.left();
         d.sourceTop = sourceCenterTop;
         d.width = sourceMargins.left();
         d.height = sourceCenterHeight;
         d.x = (0.5 * (xTarget[1] + xTarget[0]));
         d.scaleX = qreal(xTarget[1] - xTarget[0]) / d.width;
         d.scaleY = dy / d.height;
         for (int i = 1; i < rows - 1; ++i) {
            d.y = (0.5 * (yTarget[i + 1] + yTarget[i]));
            data.append(d);
         }
         if (rules.vertical == Qt::RepeatTile) {
            data[data.size() - 1].height = ((yTarget[rows - 1] - yTarget[rows - 2]) / d.scaleY);
         }
      }
      if (targetMargins.right() > 0 && sourceMargins.right() > 0) { // right
         QPixmapFragmentsArray &data = hints & QDrawBorderPixmap::OpaqueRight ? opaqueData : translucentData;
         d.sourceLeft = sourceCenterRight;
         d.sourceTop = sourceCenterTop;
         d.width = sourceMargins.right();
         d.height = sourceCenterHeight;
         d.x = (0.5 * (xTarget[columns] + xTarget[columns - 1]));
         d.scaleX = qreal(xTarget[columns] - xTarget[columns - 1]) / d.width;
         d.scaleY = dy / d.height;
         for (int i = 1; i < rows - 1; ++i) {
            d.y = (0.5 * (yTarget[i + 1] + yTarget[i]));
            data.append(d);
         }
         if (rules.vertical == Qt::RepeatTile) {
            data[data.size() - 1].height = ((yTarget[rows - 1] - yTarget[rows - 2]) / d.scaleY);
         }
      }
   }

   // center
   if (targetCenterWidth > 0 && targetCenterHeight > 0 && sourceCenterWidth > 0 && sourceCenterHeight > 0) {
      QPixmapFragmentsArray &data = hints & QDrawBorderPixmap::OpaqueCenter ? opaqueData : translucentData;
      d.sourceLeft = sourceCenterLeft;
      d.sourceTop = sourceCenterTop;
      d.width = sourceCenterWidth;
      d.height = sourceCenterHeight;
      d.scaleX = dx / d.width;
      d.scaleY = dy / d.height;

      qreal repeatWidth = (xTarget[columns - 1] - xTarget[columns - 2]) / d.scaleX;
      qreal repeatHeight = (yTarget[rows - 1] - yTarget[rows - 2]) / d.scaleY;

      for (int j = 1; j < rows - 1; ++j) {
         d.y = (0.5 * (yTarget[j + 1] + yTarget[j]));
         for (int i = 1; i < columns - 1; ++i) {
            d.x = (0.5 * (xTarget[i + 1] + xTarget[i]));
            data.append(d);
         }
         if (rules.horizontal == Qt::RepeatTile) {
            data[data.size() - 1].width = repeatWidth;
         }
      }
      if (rules.vertical == Qt::RepeatTile) {
         for (int i = 1; i < columns - 1; ++i) {
            data[data.size() - i].height = repeatHeight;
         }
      }
   }

   if (opaqueData.size()) {
      painter->drawPixmapFragments(opaqueData.data(), opaqueData.size(), pixmap, QPainter::OpaqueHint);
   }
   if (translucentData.size()) {
      painter->drawPixmapFragments(translucentData.data(), translucentData.size(), pixmap);
   }

   if (oldAA) {
      painter->setRenderHint(QPainter::Antialiasing, true);
   }
}

