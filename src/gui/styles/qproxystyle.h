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

#ifndef QPROXYSTYLE_H
#define QPROXYSTYLE_H

#include <qcommonstyle.h>

#if ! defined(QT_NO_STYLE_PROXY)

class QProxyStylePrivate;

class Q_GUI_EXPORT QProxyStyle : public QCommonStyle
{
   GUI_CS_OBJECT(QProxyStyle)

 public:
   QProxyStyle(QStyle *style = nullptr);
   QProxyStyle(const QString &key);

   QProxyStyle(const QProxyStyle &) = delete;
   QProxyStyle &operator=(const QProxyStyle &) = delete;

   ~QProxyStyle();

   QStyle *baseStyle() const;
   void setBaseStyle(QStyle *style);

   void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
   void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;

   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled,
      const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const override;

   void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;

   QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;

   QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override;
   QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl sc, const QWidget *widget) const override;
   QRect itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const override;
   QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const override;

   SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &pos,
      const QWidget *widget = nullptr) const override;

   int styleHint(StyleHint hint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr,
      QStyleHintReturn *styleHintReturn = nullptr) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

   int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
      Qt::Orientation orientation, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

   QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr,
         const QWidget *widget = nullptr) const override;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
         const QWidget *widget = nullptr) const override;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;
   QPalette standardPalette() const override;

   void polish(QWidget *widget) override;
   void polish(QPalette &pal) override;
   void polish(QApplication *app) override;

   void unpolish(QWidget *widget) override;
   void unpolish(QApplication *app) override;

 protected:
   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QProxyStyle)
};

#endif // QT_NO_STYLE_PROXY

#endif
