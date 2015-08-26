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

#ifndef QWINDOWSXPSTYLE_H
#define QWINDOWSXPSTYLE_H

#include <QtGui/qwindowsstyle.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_STYLE_WINDOWSXP)

class QWindowsXPStylePrivate;

class Q_GUI_EXPORT QWindowsXPStyle : public QWindowsStyle
{
   GUI_CS_OBJECT(QWindowsXPStyle)

 public:
   QWindowsXPStyle();
   QWindowsXPStyle(QWindowsXPStylePrivate &dd);
   ~QWindowsXPStyle();

   void unpolish(QApplication *);
   void polish(QApplication *);
   void polish(QWidget *);
   void polish(QPalette &);
   void unpolish(QWidget *);

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *p, const QWidget *widget = 0) const;
   void drawControl(ControlElement element, const QStyleOption *option, QPainter *p, const QWidget *wwidget = 0) const;

   QRect subElementRect(SubElement r, const QStyleOption *option, const QWidget *widget = 0) const;
   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *option, SubControl sc, const QWidget *widget = 0) const;

   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *option, QPainter *p, const QWidget *widget = 0) const;

   QSize sizeFromContents(ContentsType ct, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget = 0) const;

   int pixelMetric(PixelMetric pm, const QStyleOption *option = 0, const QWidget *widget = 0) const;
   int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const;

   QPalette standardPalette() const;
   QPixmap standardPixmap(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget = 0) const;

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget = 0) const;

 private:
   Q_DISABLE_COPY(QWindowsXPStyle)
   Q_DECLARE_PRIVATE(QWindowsXPStyle)

   friend class QStyleFactory;

   void *reserved;
};

#endif // QT_NO_STYLE_WINDOWSXP

QT_END_NAMESPACE

#endif // QWINDOWSXPSTYLE_H
