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

#ifndef QGTKSTYLE_H
#define QGTKSTYLE_H

#include <QtGui/QCleanlooksStyle>
#include <QtGui/QPalette>
#include <QtGui/QFont>
#include <QtGui/QFileDialog>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_STYLE_GTK)

class QPainterPath;
class QGtkStylePrivate;

class Q_GUI_EXPORT QGtkStyle : public QCleanlooksStyle
{
   GUI_CS_OBJECT(QGtkStyle)
   Q_DECLARE_PRIVATE(QGtkStyle)

 public:
   QGtkStyle();
   QGtkStyle(QGtkStylePrivate &dd);

   ~QGtkStyle();

   QPalette standardPalette() const override;

   void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, 
                  const QWidget *widget) const override;

   void drawControl(ControlElement control, const QStyleOption *option, QPainter *painter, 
                  const QWidget *widget) const override;

   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, 
                  const QWidget *widget) const override;

   void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;

   void drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal, bool enabled, const QString &text, 
                  QPalette::ColorRole textRole) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const override;
   int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;

   QStyle::SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                  const QPoint &pt, const QWidget *w) const override;

   QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, 
                  const QWidget *widget) const override;

   QRect subElementRect(SubElement sr, const QStyleOption *opt, const QWidget *w) const override;
   QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const override;

   QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
   QPixmap standardPixmap(StandardPixmap sp, const QStyleOption *option, const QWidget *widget) const override;
   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const override;

   void polish(QWidget *widget) override;
   void polish(QApplication *app) override;
   void polish(QPalette &palette) override;

   void unpolish(QWidget *widget) override;
   void unpolish(QApplication *app) override;

   static bool getGConfBool(const QString &key, bool fallback = 0);
   static QString getGConfString(const QString &key, const QString &fallback = QString());

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option, 
                  const QWidget *widget = 0) const override;

};

#endif //!defined(QT_NO_STYLE_QGTK)

QT_END_NAMESPACE

#endif //QGTKSTYLE_H
