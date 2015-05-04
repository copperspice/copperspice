/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QCLEANLOOKSSTYLE_H
#define QCLEANLOOKSSTYLE_H

#include <QtGui/qwindowsstyle.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_STYLE_CLEANLOOKS)

class QCleanlooksStylePrivate;

class Q_GUI_EXPORT QCleanlooksStyle : public QWindowsStyle
{
   CS_OBJECT(QCleanlooksStyle)
   Q_DECLARE_PRIVATE(QCleanlooksStyle)

 public:
   QCleanlooksStyle();
   ~QCleanlooksStyle();

   QPalette standardPalette () const;
   void drawPrimitive(PrimitiveElement elem, const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
   void drawControl(ControlElement ce, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
   int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;

   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;
   QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const; 
   QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const;

   SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt, const QWidget *w = 0) const;
   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const;
   int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const;

   QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const;
   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget = 0) const;
   void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const;

   void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled,
         const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;

   void polish(QWidget *widget);
   void polish(QApplication *app);
   void polish(QPalette &pal);
   void unpolish(QWidget *widget);
   void unpolish(QApplication *app);

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget = 0) const;

   QCleanlooksStyle(QCleanlooksStylePrivate &dd);
};

#endif // QT_NO_STYLE_CLEANLOOKS

QT_END_NAMESPACE

#endif // QCLEANLOOKSSTYLE_H
