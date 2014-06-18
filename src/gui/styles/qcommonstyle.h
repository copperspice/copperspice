/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
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
   CS_OBJECT(QCommonStyle)

 public:
   QCommonStyle();
   ~QCommonStyle();

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                      const QWidget *w = 0) const;
   void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                    const QWidget *w = 0) const;
   QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const;
   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                           const QWidget *w = 0) const;
   SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                    const QPoint &pt, const QWidget *w = 0) const;
   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                        const QWidget *w = 0) const;
   QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                          const QSize &contentsSize, const QWidget *widget = 0) const;

   int pixelMetric(PixelMetric m, const QStyleOption *opt = 0, const QWidget *widget = 0) const;

   int styleHint(StyleHint sh, const QStyleOption *opt = 0, const QWidget *w = 0,
                 QStyleHintReturn *shret = 0) const;

   QPixmap standardPixmap(StandardPixmap sp, const QStyleOption *opt = 0,
                          const QWidget *widget = 0) const;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const;

   void polish(QPalette &);
   void polish(QApplication *app);
   void polish(QWidget *widget);
   void unpolish(QWidget *widget);
   void unpolish(QApplication *application);

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0,
                                    const QWidget *widget = 0) const;
   QCommonStyle(QCommonStylePrivate &dd);

 private:
   Q_DECLARE_PRIVATE(QCommonStyle)
   Q_DISABLE_COPY(QCommonStyle)
};

QT_END_NAMESPACE

#endif // QCOMMONSTYLE_H
