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

   void polish(QWidget *w);
   void unpolish(QWidget *w);

   void polish(QApplication *);
   void unpolish(QApplication *);

   void polish(QPalette &pal);

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const;
   void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const;
   QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const;

   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *w = 0) const;
   SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt, const QWidget *w = 0) const;

   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const;
   QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,const QSize &contentsSize, const QWidget *w = 0) const;

   int pixelMetric(PixelMetric pm, const QStyleOption *opt = 0, const QWidget *widget = 0) const;

   QPalette standardPalette() const;

   virtual int styleHint(StyleHint sh, const QStyleOption *opt = 0, const QWidget *w = 0, QStyleHintReturn *shret = 0) const;

   enum FocusRectPolicy { FocusEnabled, FocusDisabled, FocusDefault };
   static void setFocusRectPolicy(QWidget *w, FocusRectPolicy policy);
   static FocusRectPolicy focusRectPolicy(const QWidget *w);

   enum WidgetSizePolicy { SizeSmall, SizeLarge, SizeMini, SizeDefault };
   static void setWidgetSizePolicy(const QWidget *w, WidgetSizePolicy policy);
   static WidgetSizePolicy widgetSizePolicy(const QWidget *w);

   QPixmap standardPixmap(StandardPixmap sp, const QStyleOption *opt, const QWidget *widget = 0) const;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const;

   virtual void drawItemText(QPainter *p, const QRect &r, int flags, const QPalette &pal,
         bool enabled, const QString &text, QPalette::ColorRole textRole  = QPalette::NoRole) const;

   bool event(QEvent *e);

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0, const QWidget *widget = 0) const;

   int layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, 
         const QStyleOption *option = 0, const QWidget *widget = 0) const;

 private:
   Q_DISABLE_COPY(QMacStyle)

   QMacStylePrivate *d;

   friend bool qt_mac_buttonIsRenderedFlat(const QPushButton *pushButton, const QStyleOptionButton *option);
};

#endif

QT_END_NAMESPACE

#endif // QMACSTYLE_H
