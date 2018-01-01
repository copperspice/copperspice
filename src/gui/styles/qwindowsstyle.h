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

#ifndef QWINDOWSSTYLE_H
#define QWINDOWSSTYLE_H

#include <QtGui/qcommonstyle.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_STYLE_WINDOWS)

class QWindowsStylePrivate;

class Q_GUI_EXPORT QWindowsStyle : public QCommonStyle
{
   GUI_CS_OBJECT(QWindowsStyle)

 public:
   QWindowsStyle();
   ~QWindowsStyle();

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const override;
   void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const override;
   QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const override;

   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, 
                  const QWidget *w = 0) const override;

   QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, 
                  const QWidget *widget = 0) const override;

   int pixelMetric(PixelMetric pm, const QStyleOption *option = 0, const QWidget *widget = 0) const override;  

   int styleHint(StyleHint hint, const QStyleOption *opt = 0, const QWidget *widget = 0, 
                  QStyleHintReturn *returnData = 0) const override;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget = 0) const override;

   void polish(QApplication *) override;  
   void polish(QWidget *) override;
   void polish(QPalette &) override;
   void unpolish(QWidget *) override;
   void unpolish(QApplication *) override;
   
 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget = 0) const override;

   bool eventFilter(QObject *o, QEvent *e) override;
   void timerEvent(QTimerEvent *event) override;
   QWindowsStyle(QWindowsStylePrivate &dd);

 private:
   Q_DISABLE_COPY(QWindowsStyle)
   Q_DECLARE_PRIVATE(QWindowsStyle)
   void *reserved;
};

#endif // QT_NO_STYLE_WINDOWS

QT_END_NAMESPACE

#endif // QWINDOWSSTYLE_H
