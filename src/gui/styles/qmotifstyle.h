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

#ifndef QMOTIFSTYLE_H
#define QMOTIFSTYLE_H

#include <QtGui/qcommonstyle.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_STYLE_MOTIF)

class QPalette;
class QFocusFrame;
class QMotifStylePrivate;

class Q_GUI_EXPORT QMotifStyle : public QCommonStyle
{
   GUI_CS_OBJECT(QMotifStyle)

 public:
   explicit QMotifStyle(bool useHighlightCols = false);
   virtual ~QMotifStyle();

   void setUseHighlightColors(bool);
   bool useHighlightColors() const;

   void polish(QPalette &) override;
   void polish(QWidget *) override;
   void unpolish(QWidget *) override;
   void polish(QApplication *) override;
   void unpolish(QApplication *) override;

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const override;
   void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const override;

   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, 
                  const QWidget *w = 0) const override;

   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, 
                  const QWidget *widget = 0) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const override;

   QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, 
                  const QWidget *widget = 0) const override;

   QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const override;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget = 0) const override;

   int styleHint(StyleHint hint, const QStyleOption *opt = 0, const QWidget *widget = 0, 
                  QStyleHintReturn *returnData = 0) const override;

   bool event(QEvent *) override;
   QPalette standardPalette() const override;

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0, const QWidget *widget = 0) const override;

   QPointer<QFocusFrame> focus;
   QMotifStyle(QMotifStylePrivate &dd, bool useHighlightCols = false);
   void timerEvent(QTimerEvent *event) override;
   bool eventFilter(QObject *o, QEvent *e) override;

 private:
   Q_DECLARE_PRIVATE(QMotifStyle)
   Q_DISABLE_COPY(QMotifStyle)

   bool highlightCols;
};

#endif

QT_END_NAMESPACE

#endif // QMOTIFSTYLE_H
