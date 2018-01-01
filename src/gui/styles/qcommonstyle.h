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

#ifndef QCOMMONSTYLE_H
#define QCOMMONSTYLE_H

#include <QtGui/qstyle.h>

QT_BEGIN_NAMESPACE

class QCommonStylePrivate;

class Q_GUI_EXPORT QCommonStyle: public QStyle
{
   GUI_CS_OBJECT(QCommonStyle)

 public:
   QCommonStyle();
   ~QCommonStyle();

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const override;
   void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const override;
   QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const override;

   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, 
                  const QWidget *w = 0) const override;

   SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt, 
                  const QWidget *w = 0) const override;

   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const override;
   QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, 
                  const QWidget *widget = 0) const override;

   int pixelMetric(PixelMetric m, const QStyleOption *opt = 0, const QWidget *widget = 0) const override;

   int styleHint(StyleHint sh, const QStyleOption *opt = 0, const QWidget *w = 0, QStyleHintReturn *shret = 0) const override;
   QPixmap standardPixmap(StandardPixmap sp, const QStyleOption *opt = 0, const QWidget *widget = 0) const override;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const override;

   void polish(QPalette &) override;
   void polish(QApplication *app) override;
   void polish(QWidget *widget) override;
   void unpolish(QWidget *widget) override;
   void unpolish(QApplication *application) override;

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0, const QWidget *widget = 0) const override;
   QCommonStyle(QCommonStylePrivate &dd);

 private:
   Q_DECLARE_PRIVATE(QCommonStyle)
   Q_DISABLE_COPY(QCommonStyle)
};

QT_END_NAMESPACE

#endif // QCOMMONSTYLE_H
