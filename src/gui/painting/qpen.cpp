/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qpen.h>
#include <qpen_p.h>
#include <qdatastream.h>
#include <qvariant.h>
#include <qbrush.h>
#include <qdebug.h>

typedef QPenPrivate QPenData;

// internal
inline QPenPrivate::QPenPrivate(const QBrush &_brush, qreal _width, Qt::PenStyle penStyle,
   Qt::PenCapStyle _capStyle, Qt::PenJoinStyle _joinStyle, bool _defaultWidth)
   : ref(1), dashOffset(0), miterLimit(2), cosmetic(false), defaultWidth(_defaultWidth)
{
   width = _width;
   brush = _brush;
   style = penStyle;
   capStyle = _capStyle;
   joinStyle = _joinStyle;
}

static const Qt::PenCapStyle qpen_default_cap = Qt::SquareCap;
static const Qt::PenJoinStyle qpen_default_join = Qt::BevelJoin;

class QPenDataHolder
{
 public:
   QPenData *pen;
   QPenDataHolder(const QBrush &brush, qreal width, Qt::PenStyle penStyle,
      Qt::PenCapStyle penCapStyle, Qt::PenJoinStyle _joinStyle)
      : pen(new QPenData(brush, width, penStyle, penCapStyle, _joinStyle))
   { }

   ~QPenDataHolder() {
      if (!pen->ref.deref()) {
         delete pen;
      }

      pen = nullptr;
   }
};

static QPenDataHolder *defaultPenInstance()
{
   static QPenDataHolder retval(Qt::black, 1, Qt::SolidLine, qpen_default_cap, qpen_default_join);
   return &retval;
}

static QPenDataHolder *nullPenInstance()
{
   static QPenDataHolder retval(Qt::black, 1, Qt::NoPen, qpen_default_cap, qpen_default_join);
   return &retval;
}

QPen::QPen()
{
   d = defaultPenInstance()->pen;
   d->ref.ref();
}

QPen::QPen(Qt::PenStyle style)
{
   if (style == Qt::NoPen) {
      d = nullPenInstance()->pen;
      d->ref.ref();
   } else {
      d = new QPenData(Qt::black, 1, style, qpen_default_cap, qpen_default_join);
   }
}


/*!
    Constructs a solid line pen with 0 width and the given \a color.

    \sa setBrush(), setColor()
*/

QPen::QPen(const QColor &color)
{
   d = new QPenData(color, 1, Qt::SolidLine, qpen_default_cap, qpen_default_join);
}


/*!
    \fn QPen::QPen(const QBrush &brush, qreal width, Qt::PenStyle style, Qt::PenCapStyle cap, Qt::PenJoinStyle join)

    Constructs a pen with the specified \a brush, \a width, pen \a style,
    \a cap style and \a join style.

    \sa setBrush(), setWidth(), setStyle(),  setCapStyle(), setJoinStyle()
*/

QPen::QPen(const QBrush &brush, qreal width, Qt::PenStyle s, Qt::PenCapStyle c, Qt::PenJoinStyle j)
{
   d = new QPenData(brush, width, s, c, j, false);
}

/*!
    \fn QPen::QPen(const QPen &pen)

    Constructs a pen that is a copy of the given \a pen.
*/

QPen::QPen(const QPen &p)
{
   d = p.d;
   if (d) {
      d->ref.ref();
   }
}


/*!
    Destroys the pen.
*/

QPen::~QPen()
{
   if (d && ! d->ref.deref()) {
      delete d;
   }
}

/*!
    \fn void QPen::detach()
    Detaches from shared pen data to make sure that this pen is the
    only one referring the data.

    If multiple pens share common data, this pen dereferences the data
    and gets a copy of the data. Nothing is done if there is just a
    single reference.
*/

void QPen::detach()
{
   if (d->ref.load() == 1) {
      return;
   }

   QPenData *x = new QPenData(*static_cast<QPenData *>(d));
   if (! d->ref.deref()) {
      delete d;
   }

   x->ref.store(1);
   d = x;
}


/*!
    \fn QPen &QPen::operator=(const QPen &pen)

    Assigns the given \a pen to this pen and returns a reference to
    this pen.
*/

QPen &QPen::operator=(const QPen &p)
{
   QPen(p).swap(*this);
   return *this;
}

QPen::operator QVariant() const
{
   return QVariant(QVariant::Pen, this);
}


Qt::PenStyle QPen::style() const
{
   return d->style;
}


void QPen::setStyle(Qt::PenStyle s)
{
   if (d->style == s) {
      return;
   }

   detach();
   d->style = s;
   QPenData *dd = static_cast<QPenData *>(d);
   dd->dashPattern.clear();
   dd->dashOffset = 0;
}


QVector<qreal> QPen::dashPattern() const
{
   QPenData *dd = static_cast<QPenData *>(d);
   if (d->style == Qt::SolidLine || d->style == Qt::NoPen) {
      return QVector<qreal>();
   } else if (dd->dashPattern.isEmpty()) {
      const qreal space = 2;
      const qreal dot = 1;
      const qreal dash = 4;

      switch (d->style) {
         case Qt::DashLine:
            dd->dashPattern.reserve(2);
            dd->dashPattern << dash << space;
            break;
         case Qt::DotLine:
            dd->dashPattern.reserve(2);
            dd->dashPattern << dot << space;
            break;
         case Qt::DashDotLine:
            dd->dashPattern.reserve(4);
            dd->dashPattern << dash << space << dot << space;
            break;
         case Qt::DashDotDotLine:
            dd->dashPattern.reserve(6);
            dd->dashPattern << dash << space << dot << space << dot << space;
            break;
         default:
            break;
      }
   }
   return dd->dashPattern;
}


void QPen::setDashPattern(const QVector<qreal> &pattern)
{
   if (pattern.isEmpty()) {
      return;
   }
   detach();

   QPenData *dd = static_cast<QPenData *>(d);
   dd->dashPattern = pattern;
   d->style = Qt::CustomDashLine;

   if ((dd->dashPattern.size() % 2) == 1) {
      qWarning("QPen::setDashPattern: Pattern not of even length");
      dd->dashPattern << 1;
   }
}


qreal QPen::dashOffset() const
{
   QPenData *dd = static_cast<QPenData *>(d);
   return dd->dashOffset;
}

void QPen::setDashOffset(qreal offset)
{
   if (qFuzzyCompare(offset, static_cast<QPenData *>(d)->dashOffset)) {
      return;
   }
   detach();
   QPenData *dd = static_cast<QPenData *>(d);
   dd->dashOffset = offset;
   if (d->style != Qt::CustomDashLine) {
      dd->dashPattern = dashPattern();
      d->style = Qt::CustomDashLine;
   }
}

qreal QPen::miterLimit() const
{
   const QPenData *dd = static_cast<QPenData *>(d);
   return dd->miterLimit;
}


void QPen::setMiterLimit(qreal limit)
{
   detach();
   QPenData *dd = static_cast<QPenData *>(d);
   dd->miterLimit = limit;
}



int QPen::width() const
{
   return qRound(d->width);
}


qreal QPen::widthF() const
{
   return d->width;
}


void QPen::setWidth(int width)
{
   if (width < 0) {
      qWarning("QPen::setWidth: Setting a pen width with a negative value is not defined");
   }
   if ((qreal)width == d->width) {
      return;
   }
   detach();
   d->width = width;
}



void QPen::setWidthF(qreal width)
{
   if (width < 0.f) {
      qWarning("QPen::setWidthF: Setting a pen width with a negative value is not defined");
   }
   if (qAbs(d->width - width) < 0.00000001f) {
      return;
   }

   detach();
   d->width = width;
   d->defaultWidth = false;
}


/*!
    Returns the pen's cap style.

    \sa setCapStyle(), {QPen#Cap Style}{Cap Style}
*/
Qt::PenCapStyle QPen::capStyle() const
{
   return d->capStyle;
}

/*!
    \fn void QPen::setCapStyle(Qt::PenCapStyle style)

    Sets the pen's cap style to the given \a style. The default value
    is Qt::SquareCap.

    \sa capStyle(), {QPen#Cap Style}{Cap Style}
*/

void QPen::setCapStyle(Qt::PenCapStyle c)
{
   if (d->capStyle == c) {
      return;
   }
   detach();
   d->capStyle = c;
}

/*!
    Returns the pen's join style.

    \sa setJoinStyle(),  {QPen#Join Style}{Join Style}
*/
Qt::PenJoinStyle QPen::joinStyle() const
{
   return d->joinStyle;
}

/*!
    \fn void QPen::setJoinStyle(Qt::PenJoinStyle style)

    Sets the pen's join style to the given \a style. The default value
    is Qt::BevelJoin.

    \sa joinStyle(), {QPen#Join Style}{Join Style}
*/

void QPen::setJoinStyle(Qt::PenJoinStyle j)
{
   if (d->joinStyle == j) {
      return;
   }
   detach();
   d->joinStyle = j;
}

/*!
    \fn const QColor &QPen::color() const

    Returns the color of this pen's brush.

    \sa brush(), setColor()
*/
QColor QPen::color() const
{
   return d->brush.color();
}

/*!
    \fn void QPen::setColor(const QColor &color)

    Sets the color of this pen's brush to the given \a color.

    \sa setBrush(), color()
*/

void QPen::setColor(const QColor &c)
{
   detach();
   d->brush = QBrush(c);
}


/*!
    Returns the brush used to fill strokes generated with this pen.
*/
QBrush QPen::brush() const
{
   return d->brush;
}

/*!
    Sets the brush used to fill strokes generated with this pen to the given
    \a brush.

    \sa brush(), setColor()
*/
void QPen::setBrush(const QBrush &brush)
{
   detach();
   d->brush = brush;
}


/*!
    Returns true if the pen has a solid fill, otherwise false.

    \sa style(), dashPattern()
*/
bool QPen::isSolid() const
{
   return d->brush.style() == Qt::SolidPattern;
}


/*!
    Returns true if the pen is cosmetic; otherwise returns false.

    Cosmetic pens are used to draw strokes that have a constant width
    regardless of any transformations applied to the QPainter they are
    used with. Drawing a shape with a cosmetic pen ensures that its
    outline will have the same thickness at different scale factors.

    A zero width pen is cosmetic by default; pens with a non-zero width
    are non-cosmetic.

    \sa setCosmetic(), widthF()
*/

bool QPen::isCosmetic() const
{
   QPenData *dd = static_cast<QPenData *>(d);
   return (dd->cosmetic == true) || d->width == 0;
}


/*!
    Sets this pen to cosmetic or non-cosmetic, depending on the value of
    \a cosmetic.

    \sa isCosmetic()
*/

void QPen::setCosmetic(bool cosmetic)
{
   detach();
   QPenData *dd = static_cast<QPenData *>(d);
   dd->cosmetic = cosmetic;
}



/*!
    \fn bool QPen::operator!=(const QPen &pen) const

    Returns true if the pen is different from the given \a pen;
    otherwise false. Two pens are different if they have different
    styles, widths or colors.

    \sa operator==()
*/

/*!
    \fn bool QPen::operator==(const QPen &pen) const

    Returns true if the pen is equal to the given \a pen; otherwise
    false. Two pens are equal if they have equal styles, widths and
    colors.

    \sa operator!=()
*/

bool QPen::operator==(const QPen &p) const
{
   QPenData *dd = static_cast<QPenData *>(d);
   QPenData *pdd = static_cast<QPenData *>(p.d);
   return (p.d == d)
      || (p.d->style == d->style
         && p.d->capStyle == d->capStyle
         && p.d->joinStyle == d->joinStyle
         && p.d->width == d->width
         && pdd->miterLimit == dd->miterLimit
         && (d->style != Qt::CustomDashLine
            || (qFuzzyCompare(pdd->dashOffset, dd->dashOffset) &&
               pdd->dashPattern == dd->dashPattern))
         && p.d->brush == d->brush
         && pdd->cosmetic == dd->cosmetic
         && pdd->defaultWidth == dd->defaultWidth);
}


/*!
    \fn bool QPen::isDetached()

    \internal
*/

bool QPen::isDetached()
{
   return d->ref.load() == 1;
}

QDataStream &operator<<(QDataStream &s, const QPen &p)
{
   QPenData *dd = static_cast<QPenData *>(p.d);


   s << (quint16)(p.style() | p.capStyle() | p.joinStyle());
   s << (bool)(dd->cosmetic);


   s << double(p.widthF());
   s << p.brush();
   s << double(p.miterLimit());

   if (sizeof(qreal) == sizeof(double)) {
      s << p.dashPattern();

   } else {
      // ensure that we write doubles here instead of streaming the pattern
      // directly; otherwise, platforms that redefine qreal might generate
      // data that cannot be read on other platforms.
      QVector<qreal> pattern = p.dashPattern();
      s << quint32(pattern.size());
      for (int i = 0; i < pattern.size(); ++i) {
         s << double(pattern.at(i));
      }
   }


   s << double(p.dashOffset());
   s << bool(dd->defaultWidth);


   return s;
}

QDataStream &operator>>(QDataStream &s, QPen &p)
{
   quint16 style;
   double width  = 0;
   QColor color;
   QBrush brush;
   double miterLimit = 2;

   QVector<qreal> dashPattern;
   double dashOffset = 0;
   bool cosmetic     = false;
   bool defaultWidth = false;

   s >> style;
   s >> cosmetic;

   s >> width;
   s >> brush;
   s >> miterLimit;

   if (sizeof(qreal) == sizeof(double)) {
      s >> dashPattern;

   } else {
      quint32 numDashes;
      s >> numDashes;
      double dash;
      dashPattern.reserve(numDashes);

      for (quint32 i = 0; i < numDashes; ++i) {
         s >> dash;
         dashPattern << dash;
      }
   }

   s >> dashOffset;
   s >> defaultWidth;

   p.detach();

   QPenData *dd = static_cast<QPenData *>(p.d);
   dd->width = width;
   dd->brush = brush;
   dd->style = Qt::PenStyle(style & Qt::MPenStyle);
   dd->capStyle = Qt::PenCapStyle(style & Qt::MPenCapStyle);
   dd->joinStyle = Qt::PenJoinStyle(style & Qt::MPenJoinStyle);
   dd->dashPattern = dashPattern;
   dd->miterLimit = miterLimit;
   dd->dashOffset = dashOffset;
   dd->cosmetic = cosmetic;
   dd->defaultWidth = defaultWidth;

   return s;
}

QDebug operator<<(QDebug dbg, const QPen &p)
{
   const char *PEN_STYLES[] = {
      "NoPen",
      "SolidLine",
      "DashLine",
      "DotLine",
      "DashDotLine",
      "DashDotDotLine",
      "CustomDashLine"
   };

   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QPen(" << p.width() << ',' << p.brush()
      << ',' << PEN_STYLES[p.style()] << ',' << int(p.capStyle())
      << ',' << int(p.joinStyle()) << ',' << p.dashPattern()
      << ',' << p.dashOffset()
      << ',' << p.miterLimit() << ')';

   return dbg;
}

#undef QT_COMPILING_QPEN
