/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QPROXYSTYLE_H
#define QPROXYSTYLE_H

#include <QtGui/QCommonStyle>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_STYLE_PROXY)

class QProxyStylePrivate;

class Q_GUI_EXPORT QProxyStyle : public QCommonStyle
{
   GUI_CS_OBJECT(QProxyStyle)

 public:
   QProxyStyle(QStyle *baseStyle = 0);
   ~QProxyStyle();

   QStyle *baseStyle() const;
   void setBaseStyle(QStyle *style);

   void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const override; 
   void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const override;

   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, 
                  const QWidget *widget = 0) const override;

   void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled,
                  const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const override;

   void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;

   QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;

   QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override;
   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const override;
   QRect itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const override;
   QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const override;

   SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &pos,
         const QWidget *widget = 0) const override;

   int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0,
         QStyleHintReturn *returnData = 0) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const override;
   
   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget = 0) const override;
   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const override;
   QPalette standardPalette() const override;

   void polish(QWidget *widget) override;
   void polish(QPalette &pal) override;
   void polish(QApplication *app) override;

   void unpolish(QWidget *widget) override;
   void unpolish(QApplication *app) override;

 protected:
   bool event(QEvent *e) override;

   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const override;

   int layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation,
         const QStyleOption *option = 0, const QWidget *widget = 0) const override;

 private:
   Q_DISABLE_COPY(QProxyStyle)
   Q_DECLARE_PRIVATE(QProxyStyle)
};

#endif // QT_NO_STYLE_PROXY

QT_END_NAMESPACE

#endif // QPROXYSTYLE_H
