/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QFUSIONSTYLE_P_P_H
#define QFUSIONSTYLE_P_P_H

#include <qcommonstyle.h>
#include <qcommonstyle_p.h>
#include <qplatform_theme.h>
#include <qguiapplication_p.h>

#ifndef QT_NO_STYLE_FUSION

class QFusionStylePrivate : public QCommonStylePrivate
{
   Q_DECLARE_PUBLIC(QFusionStyle)

 public:
   QFusionStylePrivate();

   // Used for grip handles
   QColor lightShade() const {
      return QColor(255, 255, 255, 90);
   }
   QColor darkShade() const {
      return QColor(0, 0, 0, 60);
   }

   QColor topShadow() const {
      return QColor(0, 0, 0, 18);
   }

   QColor innerContrastLine() const {
      return QColor(255, 255, 255, 30);
   }

   // want a standard blue color used when the system palette is used
   bool isMacSystemPalette(const QPalette &pal) const {

#if defined(Q_OS_DARWIN)
      const QPalette *themePalette = QGuiApplicationPrivate::platformTheme()->palette();

      if (themePalette && themePalette->color(QPalette::Normal, QPalette::Highlight) ==
                  pal.color(QPalette::Normal, QPalette::Highlight) &&
         themePalette->color(QPalette::Normal, QPalette::HighlightedText) ==
                  pal.color(QPalette::Normal, QPalette::HighlightedText)) {

         return true;
      }
#else
   (void) pal;
#endif

      return false;
   }

   QColor highlight(const QPalette &pal) const {
      if (isMacSystemPalette(pal)) {
         return QColor(60, 140, 230);
      }
      return pal.color(QPalette::Highlight);
   }

   QColor highlightedText(const QPalette &pal) const {
      if (isMacSystemPalette(pal)) {
         return Qt::white;
      }
      return pal.color(QPalette::HighlightedText);
   }

   QColor outline(const QPalette &pal) const {
      if (pal.window().style() == Qt::TexturePattern) {
         return QColor(0, 0, 0, 160);
      }
      return pal.background().color().darker(140);
   }

   QColor highlightedOutline(const QPalette &pal) const {
      QColor highlightedOutline = highlight(pal).darker(125);
      if (highlightedOutline.value() > 160) {
         highlightedOutline.setHsl(highlightedOutline.hue(), highlightedOutline.saturation(), 160);
      }
      return highlightedOutline;
   }

   QColor tabFrameColor(const QPalette &pal) const {
      if (pal.window().style() == Qt::TexturePattern) {
         return QColor(255, 255, 255, 8);
      }
      return buttonColor(pal).lighter(104);
   }

   QColor buttonColor(const QPalette &pal) const {
      QColor buttonColor = pal.button().color();
      int val = qGray(buttonColor.rgb());
      buttonColor = buttonColor.lighter(100 + qMax(1, (180 - val) / 6));
      buttonColor.setHsv(buttonColor.hue(), buttonColor.saturation() * 0.75, buttonColor.value());
      return buttonColor;
   }

   enum {
      menuItemHMargin      =  3, // menu item hor text margin
      menuArrowHMargin     =  6, // menu arrow horizontal margin
      menuRightBorder      = 15, // right border on menus
      menuCheckMarkWidth   = 12  // checkmarks width on menus
   };
};


#endif // QT_NO_STYLE_FUSION

#endif //QFUSIONSTYLE_P_P_H
