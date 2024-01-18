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

#ifndef QFUSIONSTYLE_P_H
#define QFUSIONSTYLE_P_H

#include <qcommonstyle.h>

#if ! defined(QT_NO_STYLE_FUSION)

class QFusionStylePrivate;

class QFusionStyle : public QCommonStyle
{
   GUI_CS_OBJECT(QFusionStyle)
   Q_DECLARE_PRIVATE(QFusionStyle)

 public:
   QFusionStyle();
   ~QFusionStyle();

   QPalette standardPalette () const override;
   void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
      const QWidget *widget) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
      QPainter *painter, const QWidget *widget) const override;

   QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget = nullptr) const override;

   QSize sizeFromContents(ContentsType type, const QStyleOption *option,
      const QSize &size, const QWidget *widget) const override;

   SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
      const QPoint &point, const QWidget *widget = nullptr) const override;

   QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option,
      SubControl subControl, const QWidget *widget) const override;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;

   int styleHint(StyleHint hint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr,
      QStyleHintReturn *styleHintReturn = nullptr) const override;

   QRect itemPixmapRect(const QRect &rect, int flags, const QPixmap &pixmap) const override;
   QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr) const override;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
      const QWidget *widget = nullptr) const override;

   void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;

   void drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &palette, bool enabled,
      const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const override;

   void polish(QWidget *widget) override;
   void polish(QApplication *app) override;
   void polish(QPalette &palette) override;
   void unpolish(QWidget *widget) override;
   void unpolish(QApplication *app) override;

 protected:
   QFusionStyle(QFusionStylePrivate &dd);

};

#endif // QT_NO_STYLE_FUSION

#endif //QFUSIONSTYLE_P_H
