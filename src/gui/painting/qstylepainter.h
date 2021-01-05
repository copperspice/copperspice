/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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
      : QPainter(), widget(nullptr), wstyle(nullptr)
   {
   }

   explicit QStylePainter(QWidget *w) {
      begin(w, w);
   }

   QStylePainter(QPaintDevice *pd, QWidget *w) {
      begin(pd, w);
   }

   QStylePainter(const QStylePainter &) = delete;
   QStylePainter &operator=(const QStylePainter &) = delete;

   bool begin(QWidget *w) {
      return begin(w, w);
   }

   bool begin(QPaintDevice *pd, QWidget *w) {
      Q_ASSERT_X(w, "QStylePainter::QStylePainter", "Widget must be non-zero");
      widget = w;
      wstyle = w->style();
      return QPainter::begin(pd);
   };

   inline void drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption &opt);
   inline void drawControl(QStyle::ControlElement ce, const QStyleOption &opt);
   inline void drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex &opt);
   inline void drawItemText(const QRect &r, int flags, const QPalette &pal, bool enabled,
      const QString &text, QPalette::ColorRole textRole = QPalette::NoRole);
   inline void drawItemPixmap(const QRect &r, int flags, const QPixmap &pixmap);

   QStyle *style() const {
      return wstyle;
   }

 private:
   QWidget *widget;
   QStyle *wstyle;
};

void QStylePainter::drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption &opt)
{
   wstyle->drawPrimitive(pe, &opt, this, widget);
}

void QStylePainter::drawControl(QStyle::ControlElement ce, const QStyleOption &opt)
{
   wstyle->drawControl(ce, &opt, this, widget);
}

void QStylePainter::drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex &opt)
{
   wstyle->drawComplexControl(cc, &opt, this, widget);
}

void QStylePainter::drawItemText(const QRect &r, int flags, const QPalette &pal, bool enabled,
   const QString &text, QPalette::ColorRole textRole)
{
   wstyle->drawItemText(this, r, flags, pal, enabled, text, textRole);
}

void QStylePainter::drawItemPixmap(const QRect &r, int flags, const QPixmap &pixmap)
{
   wstyle->drawItemPixmap(this, r, flags, pixmap);
}


#endif // QSTYLEPAINTER_H
