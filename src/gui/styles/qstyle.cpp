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

#include <qstyle.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qbitmap.h>
#include <qpixmapcache.h>
#include <qstyleoption.h>
#include <qstyle_p.h>

#include <qguiapplication_p.h>

#include <algorithm>
#include <limits.h>

static constexpr const int MaxBits = 8 * sizeof(QSizePolicy::ControlType);

static int unpackControlTypes(QSizePolicy::ControlTypes controls, QSizePolicy::ControlType *array)
{
   if (! controls) {
      return 0;
   }

   // optimization: exactly one bit is set
   if ((controls & (controls - 1)) == 0) {
      array[0] = QSizePolicy::ControlType(uint(controls));
      return 1;
   }

   int count = 0;
   for (int i = 0; i < MaxBits; ++i) {
      if (uint bit = (controls & (0x1 << i))) {
         array[count++] = QSizePolicy::ControlType(bit);
      }
   }
   return count;
}

QStyle::QStyle()
   : QObject(nullptr), d_ptr(new QStylePrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QStyle);

   d->proxyStyle = this;
}

QStyle::QStyle(QStylePrivate &dd)
   : QObject(nullptr), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   Q_D(QStyle);

   d->proxyStyle = this;
}

QStyle::~QStyle()
{
}

void QStyle::polish(QWidget *)
{
}

void QStyle::unpolish(QWidget *)
{
}

void QStyle::polish(QApplication *)
{
}

void QStyle::unpolish(QApplication *)
{
}

void QStyle::polish(QPalette &)
{
}

QRect QStyle::itemTextRect(const QFontMetrics &metrics, const QRect &rect, int alignment, bool enabled,
   const QString &text) const
{
   QRect result;
   int x, y, w, h;
   rect.getRect(&x, &y, &w, &h);

   if (!text.isEmpty()) {
      result = metrics.boundingRect(x, y, w, h, alignment, text);
      if (!enabled && proxy()->styleHint(SH_EtchDisabledText)) {
         result.setWidth(result.width() + 1);
         result.setHeight(result.height() + 1);
      }
   } else {
      result = QRect(x, y, w, h);
   }
   return result;
}

QRect QStyle::itemPixmapRect(const QRect &rect, int alignment, const QPixmap &pixmap) const
{
   QRect result;
   int x, y, w, h;

   rect.getRect(&x, &y, &w, &h);
   const int pixmapWidth  = pixmap.width() / pixmap.devicePixelRatio();
   const int pixmapHeight = pixmap.height() / pixmap.devicePixelRatio();
   if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter) {
      y += h / 2 - pixmapHeight / 2;

   } else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom) {
      y += h - pixmapHeight;
   }

   if ((alignment & Qt::AlignRight) == Qt::AlignRight) {
      x += w - pixmapWidth;

   } else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter) {
      x += w / 2 - pixmapWidth / 2;

   } else if ((alignment & Qt::AlignLeft) != Qt::AlignLeft && QApplication::isRightToLeft()) {
      x += w - pixmapWidth;
   }

   result = QRect(x, y, pixmapWidth, pixmapHeight);
   return result;
}

void QStyle::drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
   bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
   if (text.isEmpty()) {
      return;
   }

   QPen savedPen;
   if (textRole != QPalette::NoRole) {
      savedPen = painter->pen();
      painter->setPen(QPen(pal.brush(textRole), savedPen.widthF()));
   }
   if (!enabled) {
      if (proxy()->styleHint(SH_DitherDisabledText)) {
         QRect br;
         painter->drawText(rect, alignment, text, &br);
         painter->fillRect(br, QBrush(painter->background().color(), Qt::Dense5Pattern));
         return;
      } else if (proxy()->styleHint(SH_EtchDisabledText)) {
         QPen pen = painter->pen();
         painter->setPen(pal.light().color());
         painter->drawText(rect.adjusted(1, 1, 1, 1), alignment, text);
         painter->setPen(pen);
      }
   }

   painter->drawText(rect, alignment, text);
   if (textRole != QPalette::NoRole) {
      painter->setPen(savedPen);
   }
}

void QStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
   qreal scale = pixmap.devicePixelRatio();

   QRect aligned = alignedRect(QApplication::layoutDirection(), QFlag(alignment), pixmap.size() / scale, rect);
   QRect inter   = aligned.intersected(rect);

   painter->drawPixmap(inter.x(), inter.y(), pixmap, inter.x() - aligned.x(), inter.y() - aligned.y(), inter.width() * scale,
      inter.height() *scale);
}

QRect QStyle::visualRect(Qt::LayoutDirection direction, const QRect &boundingRect, const QRect &logicalRect)
{
   if (direction == Qt::LeftToRight) {
      return logicalRect;
   }

   QRect rect = logicalRect;
   rect.translate(2 * (boundingRect.right() - logicalRect.right()) +
      logicalRect.width() - boundingRect.width(), 0);

   return rect;
}

QPoint QStyle::visualPos(Qt::LayoutDirection direction, const QRect &boundingRect, const QPoint &logicalPos)
{
   if (direction == Qt::LeftToRight) {
      return logicalPos;
   }
   return QPoint(boundingRect.right() - logicalPos.x(), logicalPos.y());
}

QRect QStyle::alignedRect(Qt::LayoutDirection direction, Qt::Alignment alignment, const QSize &size,
   const QRect &rectangle)
{
   alignment = visualAlignment(direction, alignment);
   int x = rectangle.x();
   int y = rectangle.y();
   int w = size.width();
   int h = size.height();

   if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter) {
      y += rectangle.size().height() / 2 - h / 2;
   } else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom) {
      y += rectangle.size().height() - h;
   }

   if ((alignment & Qt::AlignRight) == Qt::AlignRight) {
      x += rectangle.size().width() - w;
   } else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter) {
      x += rectangle.size().width() / 2 - w / 2;
   }

   return QRect(x, y, w, h);
}

Qt::Alignment QStyle::visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment)
{
   return QGuiApplicationPrivate::visualAlignment(direction, alignment);
}



int QStyle::sliderPositionFromValue(int min, int max, int logicalValue, int span, bool upsideDown)
{
   if (span <= 0 || logicalValue < min || max <= min) {
      return 0;
   }
   if (logicalValue > max) {
      return upsideDown ? span : min;
   }

   uint range = max - min;
   uint p = upsideDown ? max - logicalValue : logicalValue - min;

   if (range > (uint)INT_MAX / 4096) {
      double dpos = (double(p)) / (double(range) / span);
      return int(dpos);
   } else if (range > (uint)span) {
      return (2 * p * span + range) / (2 * range);
   } else {
      uint div = span / range;
      uint mod = span % range;
      return p * div + (2 * p * mod + range) / (2 * range);
   }
   // equiv. to (p * span) / range + 0.5
   // no overflow because of this implicit assumption:
   // span <= 4096
}


int QStyle::sliderValueFromPosition(int min, int max, int pos, int span, bool upsideDown)
{
   if (span <= 0 || pos <= 0) {
      return upsideDown ? max : min;
   }

   if (pos >= span) {
      return upsideDown ? min : max;
   }

   uint range = max - min;

   if ((uint)span > range) {
      int tmp = (2 * pos * range + span) / (2 * span);
      return upsideDown ? max - tmp : tmp + min;
   } else {
      uint div = range / span;
      uint mod = range % span;
      int tmp = pos * div + (2 * pos * mod + span) / (2 * span);
      return upsideDown ? max - tmp : tmp + min;
   }
   // equiv. to min + (pos*range)/span + 0.5
   // no overflow because of this implicit assumption:
   // pos <= span < sqrt(INT_MAX+0.0625)+0.25 ~ sqrt(INT_MAX)
}

QPalette QStyle::standardPalette() const
{
   QColor background = QColor(0xd4, 0xd0, 0xc8); // win 2000 grey

   QColor light(background.lighter());
   QColor dark(background.darker());
   QColor mid(Qt::gray);

   QPalette palette(Qt::black, background, light, dark, mid, Qt::black, Qt::white);
   palette.setBrush(QPalette::Disabled, QPalette::WindowText, dark);
   palette.setBrush(QPalette::Disabled, QPalette::Text, dark);
   palette.setBrush(QPalette::Disabled, QPalette::ButtonText, dark);
   palette.setBrush(QPalette::Disabled, QPalette::Base, background);
   return palette;
}


int QStyle::combinedLayoutSpacing(QSizePolicy::ControlTypes controls1,
   QSizePolicy::ControlTypes controls2, Qt::Orientation orientation,
   QStyleOption *option, QWidget *widget) const
{
   QSizePolicy::ControlType array1[MaxBits];
   QSizePolicy::ControlType array2[MaxBits];
   int count1 = unpackControlTypes(controls1, array1);
   int count2 = unpackControlTypes(controls2, array2);
   int result = -1;

   for (int i = 0; i < count1; ++i) {
      for (int j = 0; j < count2; ++j) {
         int spacing = layoutSpacing(array1[i], array2[j], orientation, option, widget);
         result = qMax(spacing, result);
      }
   }
   return result;
}

QDebug operator<<(QDebug debug, QStyle::State state)
{
   return operator<< <QStyle::StateFlag>(debug, state);
}

const QStyle *QStyle::proxy() const
{
   Q_D(const QStyle);
   return d->proxyStyle;
}

void QStyle::setProxy(QStyle *style)
{
   Q_D(QStyle);
   d->proxyStyle = style;
}

