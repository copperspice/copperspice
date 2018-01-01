/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <algorithm>

#include <qstyle.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qbitmap.h>
#include <qpixmapcache.h>
#include <qstyleoption.h>
#include <qstyle_p.h>

#ifndef QT_NO_DEBUG
#include <qdebug.h>
#endif

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif

#include <limits.h>

QT_BEGIN_NAMESPACE

static const int MaxBits = 8 * sizeof(QSizePolicy::ControlType);

static int unpackControlTypes(QSizePolicy::ControlTypes controls, QSizePolicy::ControlType *array)
{
   if (!controls) {
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
   : QObject(0), d_ptr(new QStylePrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QStyle);

   d->proxyStyle = this;
}

/*!
    \internal

    Constructs a style object.
*/
QStyle::QStyle(QStylePrivate &dd)
   : QObject(0), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   Q_D(QStyle);

   d->proxyStyle = this;
}

QStyle::~QStyle()
{
}

void QStyle::polish(QWidget * /* widget */)
{
}

void QStyle::unpolish(QWidget * /* widget */)
{
}

void QStyle::polish(QApplication * /* app */)
{
}

void QStyle::unpolish(QApplication * /* app */)
{
}

void QStyle::polish(QPalette & /* pal */)
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
   if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter) {
      y += h / 2 - pixmap.height() / 2;
   } else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom) {
      y += h - pixmap.height();
   }
   if ((alignment & Qt::AlignRight) == Qt::AlignRight) {
      x += w - pixmap.width();
   } else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter) {
      x += w / 2 - pixmap.width() / 2;
   } else if ((alignment & Qt::AlignLeft) != Qt::AlignLeft && QApplication::isRightToLeft()) {
      x += w - pixmap.width();
   }
   result = QRect(x, y, pixmap.width(), pixmap.height());
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
   QRect aligned = alignedRect(QApplication::layoutDirection(), QFlag(alignment), pixmap.size(), rect);
   QRect inter = aligned.intersected(rect);

   painter->drawPixmap(inter.x(), inter.y(), pixmap, inter.x() - aligned.x(), inter.y() - aligned.y(), inter.width(),
                       inter.height());
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
   if (!(alignment & Qt::AlignHorizontal_Mask)) {
      alignment |= Qt::AlignLeft;
   }
   if ((alignment & Qt::AlignAbsolute) == 0 && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
      if (direction == Qt::RightToLeft) {
         alignment ^= (Qt::AlignLeft | Qt::AlignRight);
      }
      alignment |= Qt::AlignAbsolute;
   }
   return alignment;
}

/*!
    Converts the given \a logicalValue to a pixel position. The \a min
    parameter maps to 0, \a max maps to \a span and other values are
    distributed evenly in-between.

    This function can handle the entire integer range without
    overflow, providing that \a span is less than 4096.

    By default, this function assumes that the maximum value is on the
    right for horizontal items and on the bottom for vertical items.
    Set the \a upsideDown parameter to true to reverse this behavior.

    \sa sliderValueFromPosition()
*/

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

/*!
    \fn int QStyle::sliderValueFromPosition(int min, int max, int position, int span, bool upsideDown)

    Converts the given pixel \a position to a logical value. 0 maps to
    the \a min parameter, \a span maps to \a max and other values are
    distributed evenly in-between.

    This function can handle the entire integer range without
    overflow.

    By default, this function assumes that the maximum value is on the
    right for horizontal items and on the bottom for vertical
    items. Set the \a upsideDown parameter to true to reverse this
    behavior.

    \sa sliderPositionFromValue()
*/

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

/*### \fn void QStyle::drawItem(QPainter *p, const QRect &r,
                              int flags, const QColorGroup &colorgroup, bool enabled,
                              const QString &text, int len = -1,
                              const QColor *penColor = 0) const

    Use one of the drawItem() overloads that takes a QPalette instead
    of a QColorGroup.
*/

/*### \fn void QStyle::drawItem(QPainter *p, const QRect &r,
                              int flags, const QColorGroup colorgroup, bool enabled,
                              const QPixmap &pixmap,
                              const QColor *penColor = 0) const

    Use one of the drawItem() overloads that takes a QPalette instead
    of a QColorGroup.
*/

/*### \fn void QStyle::drawItem(QPainter *p, const QRect &r,
                          int flags, const QColorGroup colorgroup, bool enabled,
                          const QPixmap *pixmap,
                          const QString &text, int len = -1,
                          const QColor *penColor = 0) const

    Use one of the drawItem() overloads that takes a QPalette instead
    of a QColorGroup.
*/

/*!
     Returns the style's standard palette.

    Note that on systems that support system colors, the style's
    standard palette is not used. In particular, the Windows XP,
    Vista, and Mac styles do not use the standard palette, but make
    use of native theme engines. With these styles, you should not set
    the palette with QApplication::setStandardPalette().

 */
QPalette QStyle::standardPalette() const
{
#ifdef Q_WS_X11
   QColor background;

   if (QX11Info::appDepth() > 8) {
      background = QColor(0xd4, 0xd0, 0xc8);   // win 2000 grey
   } else {
      background = QColor(192, 192, 192);
   }
#else
   QColor background(0xd4, 0xd0, 0xc8); // win 2000 grey
#endif

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

QIcon QStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
   QIcon result = this->standardIconImplementation(standardIcon, option, widget);
   return result;
}

QIcon QStyle::standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option,
      const QWidget *widget) const
{
   return QIcon(standardPixmap(standardIcon, option, widget));
}

int QStyle::layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
                          Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const
{
   int result = this->layoutSpacingImplementation(control1, control2, orientation, option, widget);
   return result;
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

int QStyle::layoutSpacingImplementation(QSizePolicy::ControlType /* control1 */,
                                        QSizePolicy::ControlType /* control2 */,
                                        Qt::Orientation /*orientation*/,
                                        const QStyleOption * /* option */,
                                        const QWidget * /* widget */) const
{
   return -1;
}

QT_BEGIN_INCLUDE_NAMESPACE
#include <QDebug>
QT_END_INCLUDE_NAMESPACE


QDebug operator<<(QDebug debug, QStyle::State state)
{
#if !defined(QT_NO_DEBUG)
   debug << "QStyle::State(";

   QStringList states;
   if (state & QStyle::State_Active) {
      states << QLatin1String("Active");
   }
   if (state & QStyle::State_AutoRaise) {
      states << QLatin1String("AutoRaise");
   }
   if (state & QStyle::State_Bottom) {
      states << QLatin1String("Bottom");
   }
   if (state & QStyle::State_Children) {
      states << QLatin1String("Children");
   }
   if (state & QStyle::State_DownArrow) {
      states << QLatin1String("DownArrow");
   }
   if (state & QStyle::State_Editing) {
      states << QLatin1String("Editing");
   }
   if (state & QStyle::State_Enabled) {
      states << QLatin1String("Enabled");
   }
   if (state & QStyle::State_FocusAtBorder) {
      states << QLatin1String("FocusAtBorder");
   }
   if (state & QStyle::State_HasFocus) {
      states << QLatin1String("HasFocus");
   }
   if (state & QStyle::State_Horizontal) {
      states << QLatin1String("Horizontal");
   }
   if (state & QStyle::State_Item) {
      states << QLatin1String("Item");
   }
   if (state & QStyle::State_KeyboardFocusChange) {
      states << QLatin1String("KeyboardFocusChange");
   }
   if (state & QStyle::State_MouseOver) {
      states << QLatin1String("MouseOver");
   }
   if (state & QStyle::State_NoChange) {
      states << QLatin1String("NoChange");
   }
   if (state & QStyle::State_Off) {
      states << QLatin1String("Off");
   }
   if (state & QStyle::State_On) {
      states << QLatin1String("On");
   }
   if (state & QStyle::State_Open) {
      states << QLatin1String("Open");
   }
   if (state & QStyle::State_Raised) {
      states << QLatin1String("Raised");
   }
   if (state & QStyle::State_ReadOnly) {
      states << QLatin1String("ReadOnly");
   }
   if (state & QStyle::State_Selected) {
      states << QLatin1String("Selected");
   }
   if (state & QStyle::State_Sibling) {
      states << QLatin1String("Sibling");
   }
   if (state & QStyle::State_Sunken) {
      states << QLatin1String("Sunken");
   }
   if (state & QStyle::State_Top) {
      states << QLatin1String("Top");
   }
   if (state & QStyle::State_UpArrow) {
      states << QLatin1String("UpArrow");
   }

   std::sort(states.begin(), states.end());

   debug << states.join(QLatin1String(" | "));
   debug << ')';
#else
   Q_UNUSED(state);
#endif

   return debug;
}

const QStyle *QStyle::proxy() const
{
   Q_D(const QStyle);
   return d->proxyStyle;
}

/* \internal

    This function sets the base style that style calls will be
    redirected to. Note that ownership is not transferred.
*/
void QStyle::setProxy(QStyle *style)
{
   Q_D(QStyle);
   d->proxyStyle = style;
}

QT_END_NAMESPACE
