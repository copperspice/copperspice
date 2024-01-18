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

#include <qdeclarativerectangle_p.h>
#include <qdeclarativerectangle_p_p.h>
#include <QPainter>
#include <QStringBuilder>
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

/*!
    \internal
    \class QDeclarativePen
    \brief The QDeclarativePen class provides a pen used for drawing rectangle borders on a QDeclarativeView.

    By default, the pen is invalid and nothing is drawn. You must either set a color (then the default
    width is 1) or a width (then the default color is black).

    A width of 1 indicates is a single-pixel line on the border of the item being painted.

    Example:
    \qml
    Rectangle {
        border.width: 2
        border.color: "red"
    }
    \endqml
*/

void QDeclarativePen::setColor(const QColor &c)
{
   _color = c;
   _valid = (_color.alpha() && _width >= 1) ? true : false;
   emit penChanged();
}

void QDeclarativePen::setWidth(int w)
{
   if (_width == w && _valid) {
      return;
   }

   _width = w;
   _valid = (_color.alpha() && _width >= 1) ? true : false;
   emit penChanged();
}


/*!
    \qmlclass GradientStop QDeclarativeGradientStop
    \ingroup qml-basic-visual-elements
    \since 4.7
    \brief The GradientStop item defines the color at a position in a Gradient.

    \sa Gradient
*/

/*!
    \qmlproperty real GradientStop::position
    \qmlproperty color GradientStop::color

    The position and color properties describe the color used at a given
    position in a gradient, as represented by a gradient stop.

    The default position is 0.0; the default color is black.

    \sa Gradient
*/

void QDeclarativeGradientStop::updateGradient()
{
   if (QDeclarativeGradient *grad = qobject_cast<QDeclarativeGradient *>(parent())) {
      grad->doUpdate();
   }
}

/*!
    \qmlclass Gradient QDeclarativeGradient
    \ingroup qml-basic-visual-elements
    \since 4.7
    \brief The Gradient item defines a gradient fill.

    A gradient is defined by two or more colors, which will be blended seamlessly.

    The colors are specified as a set of GradientStop child items, each of
    which defines a position on the gradient from 0.0 to 1.0 and a color.
    The position of each GradientStop is defined by setting its
    \l{GradientStop::}{position} property; its color is defined using its
    \l{GradientStop::}{color} property.

    A gradient without any gradient stops is rendered as a solid white fill.

    Note that this item is not a visual representation of a gradient. To display a
    gradient, use a visual element (like \l Rectangle) which supports the use
    of gradients.

    \section1 Example Usage

    \div {class="float-right"}
    \inlineimage qml-gradient.png
    \enddiv

    The following example declares a \l Rectangle item with a gradient starting
    with red, blending to yellow at one third of the height of the rectangle,
    and ending with green:

    \snippet doc/src/snippets/declarative/gradient.qml code

    \clearfloat
    \section1 Performance and Limitations

    Calculating gradients can be computationally expensive compared to the use
    of solid color fills or images. Consider using gradients for static items
    in a user interface.

    In Qt 4.7, only vertical, linear gradients can be applied to items. If you
    need to apply different orientations of gradients, a combination of rotation
    and clipping will need to be applied to the relevant items. This can
    introduce additional performance requirements for your application.

    The use of animations involving gradient stops may not give the desired
    result. An alternative way to animate gradients is to use pre-generated
    images or SVG drawings containing gradients.

    \sa GradientStop
*/

/*!
    \qmlproperty list<GradientStop> Gradient::stops
    This property holds the gradient stops describing the gradient.

    By default, this property contains an empty list.

    To set the gradient stops, define them as children of the Gradient element.
*/

const QGradient *QDeclarativeGradient::gradient() const
{
   if (!m_gradient && !m_stops.isEmpty()) {
      m_gradient = new QLinearGradient(0, 0, 0, 1.0);
      for (int i = 0; i < m_stops.count(); ++i) {
         const QDeclarativeGradientStop *stop = m_stops.at(i);
         m_gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
         m_gradient->setColorAt(stop->position(), stop->color());
      }
   }

   return m_gradient;
}

void QDeclarativeGradient::doUpdate()
{
   delete m_gradient;
   m_gradient = 0;
   emit updated();
}


/*!
    \qmlclass Rectangle QDeclarativeRectangle
    \ingroup qml-basic-visual-elements
    \since 4.7
    \brief The Rectangle item provides a filled rectangle with an optional border.
    \inherits Item

    Rectangle items are used to fill areas with solid color or gradients, and are
    often used to hold other items.

    \section1 Appearance

    Each Rectangle item is painted using either a solid fill color, specified using
    the \l color property, or a gradient, defined using a Gradient element and set
    using the \l gradient property. If both a color and a gradient are specified,
    the gradient is used.

    You can add an optional border to a rectangle with its own color and thickness
    by settting the \l border.color and \l border.width properties.

    You can also create rounded rectangles using the \l radius property. Since this
    introduces curved edges to the corners of a rectangle, it may be appropriate to
    set the \l smooth property to improve its appearance.

    \section1 Example Usage

    \div {class="float-right"}
    \inlineimage declarative-rect.png
    \enddiv

    The following example shows the effects of some of the common properties on a
    Rectangle item, which in this case is used to create a square:

    \snippet doc/src/snippets/declarative/rectangle/rectangle.qml document

    \clearfloat
    \section1 Performance

    Using the \l smooth property improves the appearance of a rounded rectangle at
    the cost of rendering performance. You should consider unsetting this property
    for rectangles in motion, and only set it when they are stationary.

    \sa Image
*/

int QDeclarativeRectanglePrivate::doUpdateSlotIdx = -1;

QDeclarativeRectangle::QDeclarativeRectangle(QDeclarativeItem *parent)
   : QDeclarativeItem(*(new QDeclarativeRectanglePrivate), parent)
{
}

void QDeclarativeRectangle::doUpdate()
{
   Q_D(QDeclarativeRectangle);
   d->rectImage = QPixmap();
   const int pw = d->pen && d->pen->isValid() ? d->pen->width() : 0;
   d->setPaintMargin((pw + 1) / 2);
   update();
}

/*!
    \qmlproperty int Rectangle::border.width
    \qmlproperty color Rectangle::border.color

    The width and color used to draw the border of the rectangle.

    A width of 1 creates a thin line. For no line, use a width of 0 or a transparent color.

    \note The width of the rectangle's border does not affect the geometry of the
    rectangle itself or its position relative to other items if anchors are used.

    If \c border.width is an odd number, the rectangle is painted at a half-pixel offset to retain
    border smoothness. Also, the border is rendered evenly on either side of the
    rectangle's boundaries, and the spare pixel is rendered to the right and below the
    rectangle (as documented for QRect rendering). This can cause unintended effects if
    \c border.width is 1 and the rectangle is \l{Item::clip}{clipped} by a parent item:

    \div {class="float-right"}
    \inlineimage rect-border-width.png
    \enddiv

    \snippet doc/src/snippets/declarative/rectangle/rect-border-width.qml 0

    \clearfloat
    Here, the innermost rectangle's border is clipped on the bottom and right edges by its
    parent. To avoid this, the border width can be set to two instead of one.
*/
QDeclarativePen *QDeclarativeRectangle::border()
{
   Q_D(QDeclarativeRectangle);
   return d->getPen();
}

/*!
    \qmlproperty Gradient Rectangle::gradient

    The gradient to use to fill the rectangle.

    This property allows for the construction of simple vertical gradients.
    Other gradients may by formed by adding rotation to the rectangle.

    \div {class="float-left"}
    \inlineimage declarative-rect_gradient.png
    \enddiv

    \snippet doc/src/snippets/declarative/rectangle/rectangle-gradient.qml rectangles
    \clearfloat

    If both a gradient and a color are specified, the gradient will be used.

    \sa Gradient, color
*/
QDeclarativeGradient *QDeclarativeRectangle::gradient() const
{
   Q_D(const QDeclarativeRectangle);
   return d->gradient;
}

void QDeclarativeRectangle::setGradient(QDeclarativeGradient *gradient)
{
   Q_D(QDeclarativeRectangle);
   if (d->gradient == gradient) {
      return;
   }
   static int updatedSignalIdx = -1;
   if (updatedSignalIdx < 0) {
      updatedSignalIdx = QDeclarativeGradient::staticMetaObject.indexOfSignal("updated()");
   }
   if (d->doUpdateSlotIdx < 0) {
      d->doUpdateSlotIdx = QDeclarativeRectangle::staticMetaObject.indexOfSlot("doUpdate()");
   }
   if (d->gradient) {
      QMetaObject::disconnect(d->gradient, updatedSignalIdx, this, d->doUpdateSlotIdx);
   }
   d->gradient = gradient;
   if (d->gradient) {
      QMetaObject::connect(d->gradient, updatedSignalIdx, this, d->doUpdateSlotIdx);
   }
   update();
}


/*!
    \qmlproperty real Rectangle::radius
    This property holds the corner radius used to draw a rounded rectangle.

    If radius is non-zero, the rectangle will be painted as a rounded rectangle, otherwise it will be
    painted as a normal rectangle. The same radius is used by all 4 corners; there is currently
    no way to specify different radii for different corners.
*/
qreal QDeclarativeRectangle::radius() const
{
   Q_D(const QDeclarativeRectangle);
   return d->radius;
}

void QDeclarativeRectangle::setRadius(qreal radius)
{
   Q_D(QDeclarativeRectangle);
   if (d->radius == radius) {
      return;
   }

   d->radius = radius;
   d->rectImage = QPixmap();
   update();
   emit radiusChanged();
}

/*!
    \qmlproperty color Rectangle::color
    This property holds the color used to fill the rectangle.

    The default color is white.

    \div {class="float-right"}
    \inlineimage rect-color.png
    \enddiv

    The following example shows rectangles with colors specified
    using hexadecimal and named color notation:

    \snippet doc/src/snippets/declarative/rectangle/rectangle-colors.qml rectangles

    \clearfloat
    If both a gradient and a color are specified, the gradient will be used.

    \sa gradient
*/
QColor QDeclarativeRectangle::color() const
{
   Q_D(const QDeclarativeRectangle);
   return d->color;
}

void QDeclarativeRectangle::setColor(const QColor &c)
{
   Q_D(QDeclarativeRectangle);
   if (d->color == c) {
      return;
   }

   d->color = c;
   d->rectImage = QPixmap();
   update();
   emit colorChanged();
}

void QDeclarativeRectangle::generateRoundedRect()
{
   Q_D(QDeclarativeRectangle);
   if (d->rectImage.isNull()) {
      const int pw = d->pen && d->pen->isValid() ? d->pen->width() : 0;
      const int radius = qCeil(d->radius);    //ensure odd numbered width/height so we get 1-pixel center

      QString key = QLatin1String("q_") % QString::number(pw) % d->color.name() % QString::number(d->color.alpha(),
                    16) % QLatin1Char('_') % QString::number(radius);
      if (d->pen && d->pen->isValid()) {
         key += d->pen->color().name() % QString::number(d->pen->color().alpha(), 16);
      }

      if (!QPixmapCache::find(key, &d->rectImage)) {
         d->rectImage = QPixmap(radius * 2 + 3 + pw * 2, radius * 2 + 3 + pw * 2);
         d->rectImage.fill(Qt::transparent);
         QPainter p(&(d->rectImage));
         p.setRenderHint(QPainter::Antialiasing);
         if (d->pen && d->pen->isValid()) {
            QPen pn(QColor(d->pen->color()), d->pen->width());
            p.setPen(pn);
         } else {
            p.setPen(Qt::NoPen);
         }
         p.setBrush(d->color);
         if (pw % 2) {
            p.drawRoundedRect(QRectF(qreal(pw) / 2 + 1, qreal(pw) / 2 + 1, d->rectImage.width() - (pw + 1),
                                     d->rectImage.height() - (pw + 1)), d->radius, d->radius);
         } else {
            p.drawRoundedRect(QRectF(qreal(pw) / 2, qreal(pw) / 2, d->rectImage.width() - pw, d->rectImage.height() - pw),
                              d->radius, d->radius);
         }

         // end painting before inserting pixmap
         // to pixmap cache to avoid a deep copy
         p.end();
         QPixmapCache::insert(key, d->rectImage);
      }
   }
}

void QDeclarativeRectangle::generateBorderedRect()
{
   Q_D(QDeclarativeRectangle);
   if (d->rectImage.isNull()) {
      const int pw = d->pen && d->pen->isValid() ? d->pen->width() : 0;

      QString key = QLatin1String("q_") % QString::number(pw) % d->color.name() % QString::number(d->color.alpha(), 16);
      if (d->pen && d->pen->isValid()) {
         key += d->pen->color().name() % QString::number(d->pen->color().alpha(), 16);
      }

      if (!QPixmapCache::find(key, &d->rectImage)) {
         // Adding 5 here makes qDrawBorderPixmap() paint correctly with smooth: true
         // See QTBUG-7999 and QTBUG-10765 for more details.
         d->rectImage = QPixmap(pw * 2 + 5, pw * 2 + 5);
         d->rectImage.fill(Qt::transparent);
         QPainter p(&(d->rectImage));
         p.setRenderHint(QPainter::Antialiasing);
         if (d->pen && d->pen->isValid()) {
            QPen pn(QColor(d->pen->color()), d->pen->width());
            pn.setJoinStyle(Qt::MiterJoin);
            p.setPen(pn);
         } else {
            p.setPen(Qt::NoPen);
         }
         p.setBrush(d->color);
         if (pw % 2) {
            p.drawRect(QRectF(qreal(pw) / 2 + 1, qreal(pw) / 2 + 1, d->rectImage.width() - (pw + 1),
                              d->rectImage.height() - (pw + 1)));
         } else {
            p.drawRect(QRectF(qreal(pw) / 2, qreal(pw) / 2, d->rectImage.width() - pw, d->rectImage.height() - pw));
         }

         // end painting before inserting pixmap
         // to pixmap cache to avoid a deep copy
         p.end();
         QPixmapCache::insert(key, d->rectImage);
      }
   }
}

void QDeclarativeRectangle::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
   Q_D(QDeclarativeRectangle);
   if (width() <= 0 || height() <= 0) {
      return;
   }
   if (d->radius > 0 || (d->pen && d->pen->isValid())
         || (d->gradient && d->gradient->gradient()) ) {
      drawRect(*p);
   } else {
      bool oldAA = p->testRenderHint(QPainter::Antialiasing);
      if (d->smooth) {
         p->setRenderHints(QPainter::Antialiasing, true);
      }
      p->fillRect(QRectF(0, 0, width(), height()), d->color);
      if (d->smooth) {
         p->setRenderHint(QPainter::Antialiasing, oldAA);
      }
   }
}

void QDeclarativeRectangle::drawRect(QPainter &p)
{
   Q_D(QDeclarativeRectangle);
   if ((d->gradient && d->gradient->gradient())
         || d->radius > width() / 2 || d->radius > height() / 2
         || width() < 3 || height() < 3) {
      // XXX This path is still slower than the image path
      // Image path won't work for gradients or invalid radius though
      bool oldAA = p.testRenderHint(QPainter::Antialiasing);
      if (d->smooth) {
         p.setRenderHint(QPainter::Antialiasing);
      }
      if (d->pen && d->pen->isValid()) {
         QPen pn(QColor(d->pen->color()), d->pen->width());
         pn.setJoinStyle(Qt::MiterJoin);
         p.setPen(pn);
      } else {
         p.setPen(Qt::NoPen);
      }
      if (d->gradient && d->gradient->gradient()) {
         p.setBrush(*d->gradient->gradient());
      } else {
         p.setBrush(d->color);
      }
      const int pw = d->pen && d->pen->isValid() ? d->pen->width() : 0;
      QRectF rect;
      if (pw % 2) {
         rect = QRectF(0.5, 0.5, width() - 1, height() - 1);
      } else {
         rect = QRectF(0, 0, width(), height());
      }
      qreal radius = d->radius;
      if (radius > width() / 2 || radius > height() / 2) {
         radius = qMin(width() / 2, height() / 2);
      }
      if (radius > 0.) {
         p.drawRoundedRect(rect, radius, radius);
      } else {
         p.drawRect(rect);
      }
      if (d->smooth) {
         p.setRenderHint(QPainter::Antialiasing, oldAA);
      }
   } else {
      bool oldAA = p.testRenderHint(QPainter::Antialiasing);
      bool oldSmooth = p.testRenderHint(QPainter::SmoothPixmapTransform);
      if (d->smooth) {
         p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, d->smooth);
      }

      const int pw = d->pen && d->pen->isValid() ? (d->pen->width() + 1) / 2 * 2 : 0;

      if (d->radius > 0) {
         generateRoundedRect();
      } else {
         generateBorderedRect();
      }

      int xOffset = (d->rectImage.width() - 1) / 2;
      int yOffset = (d->rectImage.height() - 1) / 2;
      Q_ASSERT(d->rectImage.width() == 2 * xOffset + 1);
      Q_ASSERT(d->rectImage.height() == 2 * yOffset + 1);

      // check whether we've eliminated the center completely
      if (2 * xOffset > width() + pw) {
         xOffset = (width() + pw) / 2;
      }
      if (2 * yOffset > height() + pw) {
         yOffset = (height() + pw) / 2;
      }

      QMargins margins(xOffset, yOffset, xOffset, yOffset);
      QTileRules rules(Qt::StretchTile, Qt::StretchTile);
      //NOTE: even though our item may have qreal-based width and height, qDrawBorderPixmap only supports QRects
      qDrawBorderPixmap(&p, QRect(-pw / 2, -pw / 2, width() + pw, height() + pw), margins, d->rectImage, d->rectImage.rect(),
                        margins, rules);

      if (d->smooth) {
         p.setRenderHint(QPainter::Antialiasing, oldAA);
         p.setRenderHint(QPainter::SmoothPixmapTransform, oldSmooth);
      }
   }
}

/*!
    \qmlproperty bool Rectangle::smooth

    Set this property if you want the item to be smoothly scaled or
    transformed.  Smooth filtering gives better visual quality, but is slower.  If
    the item is displayed at its natural size, this property has no visual or
    performance effect.

    \note Generally scaling artifacts are only visible if the item is stationary on
    the screen.  A common pattern when animating an item is to disable smooth
    filtering at the beginning of the animation and reenable it at the conclusion.

    \image rect-smooth.png
    On this image, smooth is turned off on the top half and on on the bottom half.
*/

QRectF QDeclarativeRectangle::boundingRect() const
{
   Q_D(const QDeclarativeRectangle);
   return QRectF(-d->paintmargin, -d->paintmargin, d->width() + d->paintmargin * 2, d->height() + d->paintmargin * 2);
}

QT_END_NAMESPACE
