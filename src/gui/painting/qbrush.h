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

#ifndef QBRUSH_H
#define QBRUSH_H

#include <QtCore/qpair.h>
#include <QtCore/qpoint.h>
#include <QtCore/qvector.h>
#include <QtCore/qscopedpointer.h>
#include <QtGui/qcolor.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qtransform.h>
#include <QtGui/qimage.h>
#include <QtGui/qpixmap.h>

QT_BEGIN_NAMESPACE

struct QBrushData;
class QPixmap;
class QGradient;
class QVariant;
struct QBrushDataPointerDeleter;

class Q_GUI_EXPORT QBrush
{
 public:
   QBrush();
   QBrush(Qt::BrushStyle bs);
   QBrush(const QColor &color, Qt::BrushStyle bs = Qt::SolidPattern);
   QBrush(Qt::GlobalColor color, Qt::BrushStyle bs = Qt::SolidPattern);

   QBrush(const QColor &color, const QPixmap &pixmap);
   QBrush(Qt::GlobalColor color, const QPixmap &pixmap);
   QBrush(const QPixmap &pixmap);
   QBrush(const QImage &image);

   QBrush(const QBrush &brush);

   QBrush(const QGradient &gradient);

   ~QBrush();
   QBrush &operator=(const QBrush &brush);

   inline QBrush &operator=(QBrush && other) {
      qSwap(d, other.d);
      return *this;
   }

   inline void swap(QBrush &other) {
      qSwap(d, other.d);
   }

   operator QVariant() const;

   inline Qt::BrushStyle style() const;
   void setStyle(Qt::BrushStyle);

   inline const QMatrix &matrix() const;
   void setMatrix(const QMatrix &mat);

   inline QTransform transform() const;
   void setTransform(const QTransform &);

   QPixmap texture() const;
   void setTexture(const QPixmap &pixmap);

   QImage textureImage() const;
   void setTextureImage(const QImage &image);

   inline const QColor &color() const;
   void setColor(const QColor &color);
   inline void setColor(Qt::GlobalColor color);

   const QGradient *gradient() const;

   bool isOpaque() const;

   bool operator==(const QBrush &b) const;
   inline bool operator!=(const QBrush &b) const {
      return !(operator==(b));
   }

  inline bool isDetached() const;
   typedef QScopedPointer<QBrushData, QBrushDataPointerDeleter> DataPtr;
   inline DataPtr &data_ptr() {
      return d;
   }

 private:

#if defined(Q_WS_X11)
   friend class QX11PaintEngine;
#endif

   friend class QRasterPaintEngine;
   friend class QRasterPaintEnginePrivate;
   friend struct QSpanData;
   friend class QPainter;
   friend bool Q_GUI_EXPORT qHasPixmapTexture(const QBrush &brush);
   void detach(Qt::BrushStyle newStyle);
   void init(const QColor &color, Qt::BrushStyle bs);
   QScopedPointer<QBrushData, QBrushDataPointerDeleter> d;
   void cleanUp(QBrushData *x); 
};

inline void QBrush::setColor(Qt::GlobalColor acolor)
{
   setColor(QColor(acolor));
}

Q_DECLARE_TYPEINFO(QBrush, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QBrush)

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QBrush &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QBrush &);
#endif

Q_GUI_EXPORT QDebug operator<<(QDebug, const QBrush &);

struct QBrushData {
   QAtomicInt ref;
   Qt::BrushStyle style;
   QColor color;
   QTransform transform;
};

inline Qt::BrushStyle QBrush::style() const
{
   return d->style;
}
inline const QColor &QBrush::color() const
{
   return d->color;
}
inline const QMatrix &QBrush::matrix() const
{
   return d->transform.toAffine();
}
inline QTransform QBrush::transform() const
{
   return d->transform;
}
inline bool QBrush::isDetached() const
{
   return d->ref.load() == 1;
}

class QGradientPrivate;

typedef QPair<qreal, QColor> QGradientStop;
typedef QVector<QGradientStop> QGradientStops;

class Q_GUI_EXPORT QGradient
{
   GUI_CS_GADGET(QGradient)

   GUI_CS_ENUM(Type)
   GUI_CS_ENUM(Spread)
   GUI_CS_ENUM(CoordinateMode)

 public:
   enum Type {
      LinearGradient,
      RadialGradient,
      ConicalGradient,
      NoGradient
   };

   enum Spread {
      PadSpread,
      ReflectSpread,
      RepeatSpread
   };

   enum CoordinateMode {
      LogicalMode,
      StretchToDeviceMode,
      ObjectBoundingMode
   };

   enum InterpolationMode {
      ColorInterpolation,
      ComponentInterpolation
   };

   QGradient();

   Type type() const {
      return m_type;
   }

   inline void setSpread(Spread spread);
   Spread spread() const {
      return m_spread;
   }

   void setColorAt(qreal pos, const QColor &color);

   void setStops(const QGradientStops &stops);
   QGradientStops stops() const;

   CoordinateMode coordinateMode() const;
   void setCoordinateMode(CoordinateMode mode);

   InterpolationMode interpolationMode() const;
   void setInterpolationMode(InterpolationMode mode);

   bool operator==(const QGradient &gradient) const;
   inline bool operator!=(const QGradient &other) const {
      return !operator==(other);
   }

   bool operator==(const QGradient &gradient); // ### Qt5/remove

 private:
   friend class QLinearGradient;
   friend class QRadialGradient;
   friend class QConicalGradient;
   friend class QBrush;

   Type m_type;
   Spread m_spread;
   QGradientStops m_stops;
   union {
      struct {
         qreal x1, y1, x2, y2;
      } linear;
      struct {
         qreal cx, cy, fx, fy, cradius;
      } radial;
      struct {
         qreal cx, cy, angle;
      } conical;
   } m_data;
   void *dummy;
};

inline void QGradient::setSpread(Spread aspread)
{
   m_spread = aspread;
}

class Q_GUI_EXPORT QLinearGradient : public QGradient
{
 public:
   QLinearGradient();
   QLinearGradient(const QPointF &start, const QPointF &finalStop);
   QLinearGradient(qreal xStart, qreal yStart, qreal xFinalStop, qreal yFinalStop);

   QPointF start() const;
   void setStart(const QPointF &start);
   inline void setStart(qreal x, qreal y) {
      setStart(QPointF(x, y));
   }

   QPointF finalStop() const;
   void setFinalStop(const QPointF &stop);
   inline void setFinalStop(qreal x, qreal y) {
      setFinalStop(QPointF(x, y));
   }
};


class Q_GUI_EXPORT QRadialGradient : public QGradient
{
 public:
   QRadialGradient();
   QRadialGradient(const QPointF &center, qreal radius, const QPointF &focalPoint);
   QRadialGradient(qreal cx, qreal cy, qreal radius, qreal fx, qreal fy);

   QRadialGradient(const QPointF &center, qreal radius);
   QRadialGradient(qreal cx, qreal cy, qreal radius);

   QRadialGradient(const QPointF &center, qreal centerRadius, const QPointF &focalPoint, qreal focalRadius);
   QRadialGradient(qreal cx, qreal cy, qreal centerRadius, qreal fx, qreal fy, qreal focalRadius);

   QPointF center() const;
   void setCenter(const QPointF &center);
   inline void setCenter(qreal x, qreal y) {
      setCenter(QPointF(x, y));
   }

   QPointF focalPoint() const;
   void setFocalPoint(const QPointF &focalPoint);
   inline void setFocalPoint(qreal x, qreal y) {
      setFocalPoint(QPointF(x, y));
   }

   qreal radius() const;
   void setRadius(qreal radius);

   qreal centerRadius() const;
   void setCenterRadius(qreal radius);

   qreal focalRadius() const;
   void setFocalRadius(qreal radius);
};


class Q_GUI_EXPORT QConicalGradient : public QGradient
{
 public:
   QConicalGradient();
   QConicalGradient(const QPointF &center, qreal startAngle);
   QConicalGradient(qreal cx, qreal cy, qreal startAngle);

   QPointF center() const;
   void setCenter(const QPointF &center);
   inline void setCenter(qreal x, qreal y) {
      setCenter(QPointF(x, y));
   }

   qreal angle() const;
   void setAngle(qreal angle);
};

QT_END_NAMESPACE

#endif // QBRUSH_H
