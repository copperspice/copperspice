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

#ifndef QSTYLEPAINTER_H
#define QSTYLEPAINTER_H

#include <qpainter.h>
#include <qstyle.h>
#include <qwidget.h>

class QStylePainter : public QPainter
{
 public:
   QStylePainter()
      : QPainter(), m_widget(nullptr), m_wstyle(nullptr)
   {
   }

   explicit QStylePainter(QWidget *widget) {
      begin(widget, widget);
   }

   QStylePainter(QPaintDevice *device, QWidget *widget) {
      begin(device, widget);
   }

   QStylePainter(const QStylePainter &) = delete;
   QStylePainter &operator=(const QStylePainter &) = delete;

   bool begin(QWidget *widget) {
      return begin(widget, widget);
   }

   bool begin(QPaintDevice *device, QWidget *widget) {
      Q_ASSERT_X(widget, "QStylePainter::QStylePainter", "Widget must be non-zero");

      m_widget = widget;
      m_wstyle = widget->style();

      return QPainter::begin(device);
   };

   inline void drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption &option);
   inline void drawControl(QStyle::ControlElement ce, const QStyleOption &option);
   inline void drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex &option);
   inline void drawItemText(const QRect &rect, int flags, const QPalette &palette, bool enabled,
      const QString &text, QPalette::ColorRole textRole = QPalette::NoRole);

   inline void drawItemPixmap(const QRect &rect, int flags, const QPixmap &pixmap);

   QStyle *style() const {
      return m_wstyle;
   }

 private:
   QWidget *m_widget;
   QStyle  *m_wstyle;
};

void QStylePainter::drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption &option)
{
   m_wstyle->drawPrimitive(pe, &option, this, m_widget);
}

void QStylePainter::drawControl(QStyle::ControlElement ce, const QStyleOption &option)
{
   m_wstyle->drawControl(ce, &option, this, m_widget);
}

void QStylePainter::drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex &option)
{
   m_wstyle->drawComplexControl(cc, &option, this, m_widget);
}

void QStylePainter::drawItemText(const QRect &rect, int flags, const QPalette &pal, bool enabled,
            const QString &text, QPalette::ColorRole textRole)
{
   m_wstyle->drawItemText(this, rect, flags, pal, enabled, text, textRole);
}

void QStylePainter::drawItemPixmap(const QRect &rect, int flags, const QPixmap &pixmap)
{
   m_wstyle->drawItemPixmap(this, rect, flags, pixmap);
}

#endif // QSTYLEPAINTER_H
