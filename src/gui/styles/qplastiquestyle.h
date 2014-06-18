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

#ifndef QPLASTIQUESTYLE_H
#define QPLASTIQUESTYLE_H

#include <QtGui/qwindowsstyle.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_STYLE_PLASTIQUE)

class QPlastiqueStylePrivate;

class Q_GUI_EXPORT QPlastiqueStyle : public QWindowsStyle
{
   CS_OBJECT(QPlastiqueStyle)
   Q_DECLARE_PRIVATE(QPlastiqueStyle)

 public:
   QPlastiqueStyle();
   ~QPlastiqueStyle();

   void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                      QPainter *painter, const QWidget *widget = 0) const;

   void drawControl(ControlElement element, const QStyleOption *option,
                    QPainter *painter, const QWidget *widget) const;

   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                           QPainter *painter, const QWidget *widget) const;

   QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                          const QSize &size, const QWidget *widget) const;

   QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;
   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const;

   int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0,
                 QStyleHintReturn *returnData = 0) const;

   SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                    const QPoint &pos, const QWidget *widget = 0) const;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget = 0) const;

   void polish(QWidget *widget);
   void polish(QApplication *app);
   void polish(QPalette &pal);
   void unpolish(QWidget *widget);
   void unpolish(QApplication *app);

   QPalette standardPalette() const;

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0,
                                    const QWidget *widget = 0) const;

   int layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
                                   Qt::Orientation orientation,
                                   const QStyleOption *option = 0, const QWidget *widget = 0) const;

   bool eventFilter(QObject *watched, QEvent *event);
   void timerEvent(QTimerEvent *event);

 private:
   Q_DISABLE_COPY(QPlastiqueStyle)
   void *reserved;
};

#endif // QT_NO_STYLE_PLASTIQUE

QT_END_NAMESPACE

#endif // QPLASTIQUESTYLE_H
