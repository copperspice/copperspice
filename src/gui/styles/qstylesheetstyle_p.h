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

#ifndef QSTYLESHEETSTYLE_P_H
#define QSTYLESHEETSTYLE_P_H

#include <QtGui/qwindowsstyle.h>

#ifndef QT_NO_STYLE_STYLESHEET

#include <qstyleoption.h>
#include <qhash.h>
#include <qevent.h>
#include <qvector.h>
#include <qapplication.h>
#include <qcssparser_p.h>
#include <qbrush.h>
#include <qrenderrule_p.h>

QT_BEGIN_NAMESPACE

class QAbstractScrollArea;
class QStyleSheetStylePrivate;
class QStyleOptionTitleBar;

class QStyleSheetStyle : public QWindowsStyle
{
   typedef QWindowsStyle ParentStyle;

   GUI_CS_OBJECT(QStyleSheetStyle)

 public:
   QStyleSheetStyle(QStyle *baseStyle);
   ~QStyleSheetStyle();

   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *w = 0) const override;

   void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,const QWidget *w = 0) const override;
   void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;

   void drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
                  bool enabled, const QString &text, QPalette::ColorRole textRole  = QPalette::NoRole) const override;

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,const QWidget *w = 0) const override;

   QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;
   SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt,
                  const QWidget *w = 0) const override;

   QRect itemPixmapRect(const QRect &rect, int alignment, const QPixmap &pixmap) const override;
   QRect itemTextRect(const QFontMetrics &metrics, const QRect &rect, int alignment, bool enabled,
                  const QString &text) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const override;

   QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize,
                  const QWidget *widget = 0) const override;

   QPalette standardPalette() const override;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option = 0, const QWidget *w = 0 ) const override;

   int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
         Qt::Orientation orientation, const QStyleOption *option = 0, const QWidget *widget = 0) const;

   int styleHint(StyleHint sh, const QStyleOption *opt = 0, const QWidget *w = 0, QStyleHintReturn *shret = 0) const override;
   QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const override;
   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const override;

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

   void updateStyleSheetFont(QWidget *w) const;
   void saveWidgetFont(QWidget *w, const QFont &font) const;
   void clearWidgetFont(QWidget *w) const;

   bool styleSheetPalette(const QWidget *w, const QStyleOption *opt, QPalette *pal);

   static int numinstances;

 protected :
   QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0,
                  const QWidget *widget = 0) const override;

   int layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
                  Qt::Orientation orientation, const QStyleOption *option = 0, const QWidget *widget = 0) const override;

   bool event(QEvent *e) override;

 private:
   int refcount;

   friend class QRenderRule;
   int nativeFrameWidth(const QWidget *);
   QRenderRule renderRule(const QWidget *, int, quint64 = 0) const;
   QRenderRule renderRule(const QWidget *, const QStyleOption *, int = 0) const;
   QSize defaultSize(const QWidget *, QSize, const QRect &, int) const;
   QRect positionRect(const QWidget *, const QRenderRule &, const QRenderRule &, int, const QRect &, Qt::LayoutDirection) const;
   QRect positionRect(const QWidget *w, const QRenderRule &rule2, int pe, const QRect &originRect, Qt::LayoutDirection dir) const;

   mutable QCss::Parser parser;

   void setPalette(QWidget *);
   void unsetPalette(QWidget *);
   void setProperties(QWidget *);
   void setGeometry(QWidget *);
   QVector<QCss::StyleRule> styleRules(const QWidget *w) const;
   bool hasStyleRule(const QWidget *w, int part) const;

   QHash<QStyle::SubControl, QRect> titleBarLayout(const QWidget *w, const QStyleOptionTitleBar *tb) const;

   QCss::StyleSheet getDefaultStyleSheet() const;

   static Qt::Alignment resolveAlignment(Qt::LayoutDirection, Qt::Alignment);
   static bool isNaturalChild(const QWidget *w);
   bool initWidget(const QWidget *w) const;

   Q_DISABLE_COPY(QStyleSheetStyle)
   Q_DECLARE_PRIVATE(QStyleSheetStyle)
};

class QStyleSheetStyleCaches : public QObject
{
   GUI_CS_OBJECT(QStyleSheetStyleCaches)

 public :
   GUI_CS_SLOT_1(Public, void widgetDestroyed(QObject *un_named_arg1))
   GUI_CS_SLOT_2(widgetDestroyed)
   GUI_CS_SLOT_1(Public, void styleDestroyed(QObject *un_named_arg1))
   GUI_CS_SLOT_2(styleDestroyed)

   QHash<const QWidget *, QVector<QCss::StyleRule> > styleRulesCache;
   QHash<const QWidget *, QHash<int, bool> > hasStyleRuleCache;

   typedef QHash<int, QHash<quint64, QRenderRule> > QRenderRules;

   QHash<const QWidget *, QRenderRules> renderRulesCache;
   QHash<const QWidget *, QPalette> customPaletteWidgets; // widgets whose palette we tampered
   QHash<const void *, QCss::StyleSheet> styleSheetCache; // parsed style sheets
   QSet<const QWidget *> autoFillDisabledWidgets;
};


QT_END_NAMESPACE
#endif // QT_NO_STYLE_STYLESHEET
#endif // QSTYLESHEETSTYLE_P_H
