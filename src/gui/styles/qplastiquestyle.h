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

#ifndef QPLASTIQUESTYLE_H
#define QPLASTIQUESTYLE_H

#include <QtGui/qwindowsstyle.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_STYLE_PLASTIQUE)

class QPlastiqueStylePrivate;

class Q_GUI_EXPORT QPlastiqueStyle : public QWindowsStyle
{
   GUI_CS_OBJECT(QPlastiqueStyle)
   Q_DECLARE_PRIVATE(QPlastiqueStyle)

 public:
   QPlastiqueStyle();
   ~QPlastiqueStyle();

   void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, 
                  const QWidget *widget = 0) const override;

   void drawControl(ControlElement element, const QStyleOption *option,QPainter *painter, const QWidget *widget) const override;

   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,QPainter *painter, 
                  const QWidget *widget) const override;

   QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
   QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override;
   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const override;

   int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0, 
                  QStyleHintReturn *returnData = 0) const override;

   SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &pos, 
                  const QWidget *widget = 0) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const override;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget = 0) const override;

   void polish(QWidget *widget) override;
   void polish(QApplication *app) override;
   void polish(QPalette &pal) override;
   void unpolish(QWidget *widget) override;
   void unpolish(QApplication *app) override;

   QPalette standardPalette() const override;

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0, const QWidget *widget = 0) const override; 

   int layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation,
         const QStyleOption *option = 0, const QWidget *widget = 0) const override;

   bool eventFilter(QObject *watched, QEvent *event) override;
   void timerEvent(QTimerEvent *event) override;

 private:
   Q_DISABLE_COPY(QPlastiqueStyle)
   void *reserved;
};

#endif // QT_NO_STYLE_PLASTIQUE

QT_END_NAMESPACE

#endif // QPLASTIQUESTYLE_H
