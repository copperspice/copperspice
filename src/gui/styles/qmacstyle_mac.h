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

#ifndef QMACSTYLE_MAC_H
#define QMACSTYLE_MAC_H

#include <QtGui/qwindowsstyle.h>

QT_BEGIN_NAMESPACE

#if defined(Q_OS_MAC) && !defined(QT_NO_STYLE_MAC)

class QPalette;

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_MAC
#else
#define Q_GUI_EXPORT_STYLE_MAC Q_GUI_EXPORT
#endif

class QPushButton;
class QStyleOptionButton;
class QMacStylePrivate;

class Q_GUI_EXPORT_STYLE_MAC QMacStyle : public QWindowsStyle
{
   GUI_CS_OBJECT(QMacStyle)

 public:
   QMacStyle();
   virtual ~QMacStyle();

   void polish(QWidget *w) override;
   void unpolish(QWidget *w) override;

   void polish(QApplication *) override;
   void unpolish(QApplication *) override;

   void polish(QPalette &pal) override;

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const override;
   void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const override;
   QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const override;

   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, 
                  const QWidget *w = 0) const override;
   SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt, 
                  const QWidget *w = 0) const override;

   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, 
                  const QWidget *w = 0) const override;

   QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,const QSize &contentsSize, 
                  const QWidget *w = 0) const override;

   int pixelMetric(PixelMetric pm, const QStyleOption *opt = 0, const QWidget *widget = 0) const override;

   QPalette standardPalette() const override;

   int styleHint(StyleHint sh, const QStyleOption *opt = 0, const QWidget *w = 0, 
                  QStyleHintReturn *shret = 0) const override;

   enum FocusRectPolicy { FocusEnabled, FocusDisabled, FocusDefault };
   static void setFocusRectPolicy(QWidget *w, FocusRectPolicy policy);
   static FocusRectPolicy focusRectPolicy(const QWidget *w);

   enum WidgetSizePolicy { SizeSmall, SizeLarge, SizeMini, SizeDefault };
   static void setWidgetSizePolicy(const QWidget *w, WidgetSizePolicy policy);
   static WidgetSizePolicy widgetSizePolicy(const QWidget *w);

    QPixmap standardPixmap(StandardPixmap sp, const QStyleOption *opt, const QWidget *widget = 0) const override;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const override;

   void drawItemText(QPainter *p, const QRect &r, int flags, const QPalette &pal,
         bool enabled, const QString &text, QPalette::ColorRole textRole  = QPalette::NoRole) const override;

   bool event(QEvent *e) override;

 protected:
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0, const QWidget *widget = 0) const override;

   int layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, 
         const QStyleOption *option = 0, const QWidget *widget = 0) const override;

 private:
   Q_DISABLE_COPY(QMacStyle)

   QMacStylePrivate *d;

   friend bool qt_mac_buttonIsRenderedFlat(const QPushButton *pushButton, const QStyleOptionButton *option);
};

#endif

QT_END_NAMESPACE

#endif