/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qstyle.h>
#include <qproxystyle_p.h>
#include <qapplication_p.h>
#include <qproxystyle.h>
#include <qstylefactory.h>
#include <qstyle_p.h>

#if !defined(QT_NO_STYLE_PROXY) || defined(QT_PLUGIN)

void QProxyStylePrivate::ensureBaseStyle() const
{
   Q_Q(const QProxyStyle);

   if (baseStyle) {
      return;
   }

   if (!baseStyle && !QApplicationPrivate::styleOverride.isEmpty()) {
      baseStyle = QStyleFactory::create(QApplicationPrivate::styleOverride);

      if (baseStyle) {
         // If baseStyle is an instance of the same proxyStyle
         // we destroy it and fall back to the desktop style

         if (baseStyle->metaObject()->className() == q->metaObject()->className()) {
            delete baseStyle;
            baseStyle = 0;
         }
      }
   }

   if (!baseStyle) { // Use application desktop style
      baseStyle = QStyleFactory::create(QApplicationPrivate::desktopStyleKey());
   }

   if (!baseStyle) { // Fallback to windows style
      baseStyle = QStyleFactory::create(QLatin1String("windows"));
   }

   baseStyle->setProxy(const_cast<QProxyStyle *>(q));
   baseStyle->setParent(const_cast<QProxyStyle *>(q)); // Take ownership
}

QProxyStyle::QProxyStyle(QStyle *style)
   : QCommonStyle(*new QProxyStylePrivate())
{
   Q_D(QProxyStyle);

   if (style) {
      d->baseStyle = style;

      style->setProxy(this);
      style->setParent(this); // Take ownership

   }
}

QProxyStyle::QProxyStyle(const QString &key)
   : QCommonStyle(*new QProxyStylePrivate())
{
   Q_D(QProxyStyle);

   QStyle *style = QStyleFactory::create(key);
   if (style) {
      d->baseStyle = style;
      style->setProxy(this);
      style->setParent(this); // Take ownership
   }
}

QProxyStyle::~QProxyStyle()
{
}

QStyle *QProxyStyle::baseStyle() const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle;
}

/*!
    Sets the base style that should be proxied.

    Ownership of \a style is transferred to QProxyStyle.

    If style is zero, a desktop-dependant style will be
    assigned automatically.
*/
void QProxyStyle::setBaseStyle(QStyle *style)
{
   Q_D (QProxyStyle);

   if (d->baseStyle && d->baseStyle->parent() == this) {
      d->baseStyle->deleteLater();
   }

   d->baseStyle = style;

   if (d->baseStyle) {
      d->baseStyle->setProxy(this);
      d->baseStyle->setParent(this);
   }
}

/*! \reimp
 */
void QProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
   const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   d->baseStyle->drawPrimitive(element, option, painter, widget);
}

/*!
  \reimp
 */
void QProxyStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
   const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   d->baseStyle->drawControl(element, option, painter, widget);
}

/*! \reimp
 */
void QProxyStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
   const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   d->baseStyle->drawComplexControl(control, option, painter, widget);
}

/*! \reimp
 */
void QProxyStyle::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled,
   const QString &text, QPalette::ColorRole textRole) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   d->baseStyle->drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

/*! \reimp
 */
void QProxyStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   d->baseStyle->drawItemPixmap(painter, rect, alignment, pixmap);
}

/*! \reimp
 */
QSize QProxyStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size,
   const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->sizeFromContents(type, option, size, widget);
}

/*! \reimp
 */
QRect QProxyStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->subElementRect(element, option, widget);
}

/*! \reimp
 */
QRect QProxyStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *option, SubControl sc,
   const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->subControlRect(cc, option, sc, widget);
}

/*! \reimp
 */
QRect QProxyStyle::itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled,
   const QString &text) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->itemTextRect(fm, r, flags, enabled, text);
}

/*! \reimp
 */
QRect QProxyStyle::itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->itemPixmapRect(r, flags, pixmap);
}

/*! \reimp
 */
QStyle::SubControl QProxyStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
   const QPoint &pos, const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->hitTestComplexControl(control, option, pos, widget);
}

/*! \reimp
 */
int QProxyStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
   QStyleHintReturn *returnData) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->styleHint(hint, option, widget, returnData);
}

/*! \reimp
 */
int QProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->pixelMetric(metric, option, widget);
}

/*! \reimp
 */
QPixmap QProxyStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->standardPixmap(standardPixmap, opt, widget);
}

/*! \reimp
 */
QPixmap QProxyStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->generatedIconPixmap(iconMode, pixmap, opt);
}

/*! \reimp
 */
QPalette QProxyStyle::standardPalette() const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->standardPalette();
}

/*! \reimp
 */
void QProxyStyle::polish(QWidget *widget)
{
   Q_D (QProxyStyle);
   d->ensureBaseStyle();
   d->baseStyle->polish(widget);
}

/*! \reimp
 */
void QProxyStyle::polish(QPalette &pal)
{
   Q_D (QProxyStyle);
   d->ensureBaseStyle();
   d->baseStyle->polish(pal);
}

/*! \reimp
 */
void QProxyStyle::polish(QApplication *app)
{
   Q_D (QProxyStyle);
   d->ensureBaseStyle();
   d->baseStyle->polish(app);
}

/*! \reimp
 */
void QProxyStyle::unpolish(QWidget *widget)
{
   Q_D (QProxyStyle);
   d->ensureBaseStyle();
   d->baseStyle->unpolish(widget);
}

/*! \reimp
 */
void QProxyStyle::unpolish(QApplication *app)
{
   Q_D (QProxyStyle);
   d->ensureBaseStyle();
   d->baseStyle->unpolish(app);
}

/*! \reimp
 */
bool QProxyStyle::event(QEvent *e)
{
   Q_D (QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->event(e);
}

QIcon QProxyStyle::standardIcon(StandardPixmap standardIcon,
   const QStyleOption *option, const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->standardIcon(standardIcon, option, widget);
}

int QProxyStyle::layoutSpacing(QSizePolicy::ControlType control1,
   QSizePolicy::ControlType control2,
   Qt::Orientation orientation,
   const QStyleOption *option,
   const QWidget *widget) const
{
   Q_D (const QProxyStyle);
   d->ensureBaseStyle();
   return d->baseStyle->layoutSpacing(control1, control2, orientation, option, widget);
}

#endif // QT_NO_STYLE_PROXY
