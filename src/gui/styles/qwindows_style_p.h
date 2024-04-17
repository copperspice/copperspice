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

#ifndef QWINDOWSSTYLE_P_H
#define QWINDOWSSTYLE_P_H

#include <qcommonstyle.h>
#include <qcommonstyle_p.h>

#ifndef QT_NO_STYLE_WINDOWS

#include <qlist.h>

class QTime;
class QWindowsStylePrivate;

class QWindowsStyle : public QCommonStyle
{
   GUI_CS_OBJECT(QWindowsStyle)

 public:
   QWindowsStyle();

   QWindowsStyle(const QWindowsStyle &) = delete;
   QWindowsStyle &operator=(const QWindowsStyle &) = delete;

   ~QWindowsStyle();

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   QRect subElementRect(SubElement subElement, const QStyleOption *option, const QWidget *widget = nullptr) const override;

   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   QSize sizeFromContents(ContentsType ct, const QStyleOption *option, const QSize &contentsSize,
      const QWidget *widget = nullptr) const override;

   int pixelMetric(PixelMetric pm, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

   int styleHint(StyleHint hint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr,
      QStyleHintReturn *styleHintReturn = nullptr) const override;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option, const QWidget *widget = nullptr) const override;

   QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr) const override;


   void polish(QApplication *app) override;
   void polish(QWidget *widget) override;
   void polish(QPalette &palette) override;

   void unpolish(QApplication *app) override;
   void unpolish(QWidget *widget) override;

 protected:
   bool eventFilter(QObject *object, QEvent *event) override;
   QWindowsStyle(QWindowsStylePrivate &dd);

 private:
   Q_DECLARE_PRIVATE(QWindowsStyle)
};

class QWindowsStylePrivate : public QCommonStylePrivate
{
   Q_DECLARE_PUBLIC(QWindowsStyle)

 public:
   static constexpr const int InvalidMetric = -23576;
   QWindowsStylePrivate();

   static int pixelMetricFromSystemDp(QStyle::PixelMetric pm, const QStyleOption *option = nullptr,
         const QWidget *widget = nullptr);

   static int fixedPixelMetric(QStyle::PixelMetric pm);

   static qreal devicePixelRatio(const QWidget *widget = nullptr) {
      return widget ? widget->devicePixelRatioF() : QWindowsStylePrivate::appDevicePixelRatio();
   }

   static qreal nativeMetricScaleFactor(const QWidget *widget = nullptr);

   bool hasSeenAlt(const QWidget *widget) const;
   bool altDown() const {
      return alt_down;
   }

   bool alt_down;
   QList<const QWidget *> seenAlt;
   int menuBarTimer;

   QColor inactiveCaptionText;
   QColor activeCaptionColor;
   QColor activeGradientCaptionColor;
   QColor inactiveCaptionColor;
   QColor inactiveGradientCaptionColor;

   enum {
      windowsItemFrame        =  2, // menu item frame width
      windowsSepHeight        =  9, // separator item height
      windowsItemHMargin      =  3, // menu item hor text margin
      windowsItemVMargin      =  2, // menu item ver text margin
      windowsArrowHMargin     =  6, // arrow horizontal margin
      windowsRightBorder      = 15, // right border on windows
      windowsCheckMarkWidth   = 12  // checkmarks width on windows
   };
 private:
   static qreal appDevicePixelRatio();
};

#endif // QT_NO_STYLE_WINDOWS

#endif
