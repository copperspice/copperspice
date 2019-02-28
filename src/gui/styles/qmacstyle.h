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

#ifndef QMACSTYLE_H
#define QMACSTYLE_H

#include <qcommonstyle.h>

#if defined(Q_OS_MAC) && ! defined(QT_NO_STYLE_MAC)

class QPalette;
class QPushButton;
class QStyleOptionButton;
class QMacStylePrivate;

class QMacStyle : public QCommonStyle
{
   GUI_CS_OBJECT(QMacStyle)

   Q_DECLARE_PRIVATE(QMacStyle)

 public:
   enum FocusRectPolicy { FocusEnabled, FocusDisabled, FocusDefault };
   enum WidgetSizePolicy { SizeSmall, SizeLarge, SizeMini, SizeDefault };

   QMacStyle();
   virtual ~QMacStyle();

   void polish(QWidget *w) override;
   void unpolish(QWidget *w) override;

   void polish(QApplication *) override;
   void unpolish(QApplication *) override;

   void polish(QPalette &pal) override;

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
         const QWidget *w = 0) const override;

   void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
         const QWidget *w = 0) const override;

   QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const override;

   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
         const QWidget *w = 0) const override;

   SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt,
         const QWidget *w = 0) const override;

   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
         const QWidget *w = 0) const override;

   QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize,
         const QWidget *w = 0) const override;

   int pixelMetric(PixelMetric pm, const QStyleOption *opt = 0, const QWidget *widget = 0) const override;

   QPalette standardPalette() const override;

   int styleHint(StyleHint sh, const QStyleOption *opt = 0, const QWidget *w = 0,
      QStyleHintReturn *shret = 0) const override;

   static void setFocusRectPolicy(QWidget *w, FocusRectPolicy policy);
   static FocusRectPolicy focusRectPolicy(const QWidget *w);

   static void setWidgetSizePolicy(const QWidget *w, WidgetSizePolicy policy);
   static WidgetSizePolicy widgetSizePolicy(const QWidget *w, const QStyleOption *opt = nullptr);

   QPixmap standardPixmap(StandardPixmap sp, const QStyleOption *opt, const QWidget *widget = 0) const override;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const override;

   void drawItemText(QPainter *p, const QRect &r, int flags, const QPalette &pal,
      bool enabled, const QString &text, QPalette::ColorRole textRole  = QPalette::NoRole) const override;

   bool event(QEvent *e) override;

   QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *opt = 0, const QWidget *widget = 0) const override;

   int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation,
      const QStyleOption *option = 0, const QWidget *widget = 0) const override;

 private:
   Q_DISABLE_COPY(QMacStyle)

   friend bool qt_mac_buttonIsRenderedFlat(const QPushButton *pushButton, const QStyleOptionButton *option);
};

#endif

#endif