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

#ifndef QCDESTYLE_H
#define QCDESTYLE_H

#include <QtGui/qmotifstyle.h>

QT_BEGIN_NAMESPACE

#if ! defined(QT_NO_STYLE_CDE)

class Q_GUI_EXPORT QCDEStyle : public QMotifStyle
{
   GUI_CS_OBJECT(QCDEStyle)

 public:
   explicit QCDEStyle(bool useHighlightCols = false);
   virtual ~QCDEStyle();

   int pixelMetric(PixelMetric metric, const QStyleOption *option = 0,  const QWidget *widget = 0) const override;
   void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const override;
   void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const override;
   QPalette standardPalette() const override;

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0, const QWidget *widget = 0) const override;

};

#endif // QT_NO_STYLE_CDE

QT_END_NAMESPACE

#endif
