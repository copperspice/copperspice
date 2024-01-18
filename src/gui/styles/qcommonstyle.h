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

#ifndef QCOMMONSTYLE_H
#define QCOMMONSTYLE_H

#include <qstyle.h>

class QCommonStylePrivate;

class Q_GUI_EXPORT QCommonStyle : public QStyle
{
   GUI_CS_OBJECT(QCommonStyle)

 public:
   QCommonStyle();

   QCommonStyle(const QCommonStyle &) = delete;
   QCommonStyle &operator=(const QCommonStyle &) = delete;

   ~QCommonStyle();

   void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget = nullptr) const override;

   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &point,
      const QWidget *widget = nullptr) const override;

   QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl,
      const QWidget *widget = nullptr) const override;

   QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size,
      const QWidget *widget = nullptr) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const
      QWidget *widget = nullptr) const override;

   int styleHint(StyleHint styleHint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr,
      QStyleHintReturn *styleHintReturn = nullptr) const override;

   QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr) const override;

   QPixmap standardPixmap(StandardPixmap pixmap, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr) const override;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;
   int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
      Qt::Orientation orientation, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

   void polish(QPalette &palette) override;
   void polish(QApplication *application) override;
   void polish(QWidget *widget) override;
   void unpolish(QWidget *widget) override;
   void unpolish(QApplication *application) override;

 protected:
   QCommonStyle(QCommonStylePrivate &dd);

 private:
   Q_DECLARE_PRIVATE(QCommonStyle)

#ifndef QT_NO_ANIMATION
   void _q_removeAnimation(QObject *obj);
#endif
};

#endif