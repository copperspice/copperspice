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

#ifndef QSTYLESHEETSTYLE_P_H
#define QSTYLESHEETSTYLE_P_H

#include <qwindows_style_p.h>

#ifndef QT_NO_STYLE_STYLESHEET

#include <qstyleoption.h>
#include <qbrush.h>
#include <qevent.h>
#include <qhash.h>
#include <qvector.h>
#include <qapplication.h>

#include <qcssparser_p.h>
#include <qrenderrule_p.h>

class QAbstractScrollArea;
class QStyleSheetStylePrivate;
class QStyleOptionTitleBar;

class QStyleSheetStyle : public QWindowsStyle
{
   GUI_CS_OBJECT(QStyleSheetStyle)

   using ParentStyle = QWindowsStyle;

 public:
   QStyleSheetStyle(QStyle *baseStyle);

   QStyleSheetStyle(const QStyleSheetStyle &) = delete;
   QStyleSheetStyle &operator=(const QStyleSheetStyle &) = delete;

   ~QStyleSheetStyle();

   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   void drawControl(ControlElement element, const QStyleOption *option, QPainter *p,
      const QWidget *widget = nullptr) const override;

   void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;

   void drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &palette,
      bool enabled, const QString &text, QPalette::ColorRole textRole  = QPalette::NoRole) const override;

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *opiont, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;
   SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &point,
      const QWidget *widget = nullptr) const override;

   QRect itemPixmapRect(const QRect &rect, int alignment, const QPixmap &pixmap) const override;
   QRect itemTextRect(const QFontMetrics &metrics, const QRect &rect, int alignment, bool enabled,
      const QString &text) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

   QSize sizeFromContents(ContentsType ct, const QStyleOption *option, const QSize &contentsSize,
      const QWidget *widget = nullptr) const override;

   QPalette standardPalette() const override;

   QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *opt = nullptr,
      const QWidget *widget = nullptr) const override;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr ) const override;

   int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
      Qt::Orientation orientation, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

   int styleHint(StyleHint styleHint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr,
            QStyleHintReturn *styleHintReturn = nullptr) const

 override;
   QRect subElementRect(SubElement rect, const QStyleOption *option, const QWidget *widget = nullptr) const override;
   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *option, SubControl subControl,
      const QWidget *widget = nullptr) const override;

   void polish(QWidget *widget) override;
   void polish(QApplication *app) override;
   void polish(QPalette &pal) override;

   void unpolish(QWidget *widget) override;
   void unpolish(QApplication *app) override;

   // These functions are called from QApplication/QWidget. Be careful.
   QStyle *baseStyle() const;
   void repolish(QWidget *widget);
   void repolish(QApplication *app);

   QStyle *base;
   void ref() {
      ++refcount;
   }

   void deref() {
      Q_ASSERT(refcount > 0);
      if (!--refcount) {
         delete this;
      }
   }

   void updateStyleSheetFont(QWidget *widget) const;
   void saveWidgetFont(QWidget *widget, const QFont &font) const;
   void clearWidgetFont(QWidget *widget) const;

   bool styleSheetPalette(const QWidget *widget, const QStyleOption *option, QPalette *palette);

   static int numinstances;

 protected:
   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QStyleSheetStyle)

   int refcount;

   int nativeFrameWidth(const QWidget *obj);
   QRenderRule renderRule(const QObject *obj, int, quint64 = 0) const;
   QRenderRule renderRule(const QObject *obj, const QStyleOption *, int = 0) const;

   QSize defaultSize(const QWidget *obj, QSize, const QRect &, int) const;

   QRect positionRect(const QWidget *w, const QRenderRule &, const QRenderRule &, int, const QRect &, Qt::LayoutDirection) const;
   QRect positionRect(const QWidget *w, const QRenderRule &rule2, int pe, const QRect &originRect, Qt::LayoutDirection dir) const;

   mutable QCss::Parser parser;

   void setPalette(QWidget *);
   void unsetPalette(QWidget *);
   void setProperties(QWidget *);
   void setGeometry(QWidget *);

   QVector<QCss::StyleRule> styleRules(const QObject *obj) const;
   bool hasStyleRule(const QObject *obj, int part) const;

   QHash<QStyle::SubControl, QRect> titleBarLayout(const QWidget *w, const QStyleOptionTitleBar *tb) const;

   QCss::StyleSheet getDefaultStyleSheet() const;

   static Qt::Alignment resolveAlignment(Qt::LayoutDirection, Qt::Alignment);
   static bool isNaturalChild(const QObject *obj);
   bool initObject(const QObject *obj) const;

   friend class QRenderRule;
};

class QStyleSheetStyleCaches : public QObject
{
   GUI_CS_OBJECT(QStyleSheetStyleCaches)

 public:
   using QRenderRules = QHash<int, QHash<quint64, QRenderRule>>;

   GUI_CS_SLOT_1(Public, void objectDestroyed(QObject *obj))
   GUI_CS_SLOT_2(objectDestroyed)

   GUI_CS_SLOT_1(Public, void styleDestroyed(QObject *obj))
   GUI_CS_SLOT_2(styleDestroyed)

   QHash<const QObject *, QVector<QCss::StyleRule>> styleRulesCache;
   QHash<const QObject *, QHash<int, bool>> hasStyleRuleCache;

   QHash<const QObject *, QRenderRules> renderRulesCache;
   QHash<const QWidget *, QPalette> customPaletteWidgets;   // widgets whose palette we tampered

   QHash<const void *, QCss::StyleSheet> styleSheetCache;      // parsed style sheets
   QSet<const QWidget *> autoFillDisabledWidgets;
};

#endif // QT_NO_STYLE_STYLESHEET
#endif // QSTYLESHEETSTYLE_P_H
