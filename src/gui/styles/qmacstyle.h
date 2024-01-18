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

#ifndef QMACSTYLE_H
#define QMACSTYLE_H

#include <qcommonstyle.h>

#if defined(Q_OS_DARWIN) && ! defined(QT_NO_STYLE_MAC)

class QPalette;
class QPushButton;
class QStyleOptionButton;
class QMacStylePrivate;

class QMacStyle : public QCommonStyle
{
   GUI_CS_OBJECT(QMacStyle)

 public:
   enum FocusRectPolicy { FocusEnabled, FocusDisabled, FocusDefault };
   enum WidgetSizePolicy { SizeSmall, SizeLarge, SizeMini, SizeDefault };

   QMacStyle();

   QMacStyle(const QMacStyle &) = delete;
   QMacStyle &operator=(const QMacStyle &) = delete;

   virtual ~QMacStyle();

   void polish(QWidget *widget) override;
   void unpolish(QWidget *widget) override;

   void polish(QApplication *app) override;
   void unpolish(QApplication *app) override;

   void polish(QPalette &palette) override;

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *painter,
         const QWidget *widget = nullptr) const override;

   void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
         const QWidget *widget = nullptr) const override;

   QRect subElementRect(SubElement subElement, const QStyleOption *option, const QWidget *widget = nullptr) const override;

   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
         const QWidget *widget = nullptr) const override;

   SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &point,
         const QWidget *widget = nullptr) const override;

   QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl,
         const QWidget *widget = nullptr) const override;

   QSize sizeFromContents(ContentsType ct, const QStyleOption *option, const QSize &contentsSize,
         const QWidget *widget = nullptr) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

   QPalette standardPalette() const override;

   int styleHint(StyleHint styleHint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr,
         QStyleHintReturn *styleHintReturn = nullptr) const override;

   static void setFocusRectPolicy(QWidget *widget, FocusRectPolicy policy);
   static FocusRectPolicy focusRectPolicy(const QWidget *widget);

   static void setWidgetSizePolicy(const QWidget *widget, WidgetSizePolicy policy);
   static WidgetSizePolicy widgetSizePolicy(const QWidget *widget, const QStyleOption *option = nullptr);

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
      const QWidget *widget = nullptr) const override;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;

   void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &palette,
      bool enabled, const QString &text, QPalette::ColorRole textRole  = QPalette::NoRole) const override;

   bool event(QEvent *event) override;

   QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr) const override;

   int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation,
      const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

 private:
   Q_DECLARE_PRIVATE(QMacStyle)
   friend bool qt_mac_buttonIsRenderedFlat(const QPushButton *pushButton, const QStyleOptionButton *option);
};

#endif

#endif