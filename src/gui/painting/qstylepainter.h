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

#ifndef QSTYLEPAINTER_H
#define QSTYLEPAINTER_H

#include <QtGui/qpainter.h>
#include <QtGui/qstyle.h>
#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QStylePainter : public QPainter
{

 public:
   inline QStylePainter() : QPainter(), widget(0), wstyle(0) {}
   inline explicit QStylePainter(QWidget *w) {
      begin(w, w);
   }

   inline QStylePainter(QPaintDevice *pd, QWidget *w) {
      begin(pd, w);
   }

   inline bool begin(QWidget *w) {
      return begin(w, w);
   }

   inline bool begin(QPaintDevice *pd, QWidget *w) {
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
   inline QStyle *style() const {
      return wstyle;
   }

 private:
   QWidget *widget;
   QStyle *wstyle;
   Q_DISABLE_COPY(QStylePainter)
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

QT_END_NAMESPACE

#endif // QSTYLEPAINTER_H
