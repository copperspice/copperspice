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

#ifndef QBRUSH_H
#define QBRUSH_H

#include <qpair.h>
#include <qpoint.h>
#include <qvector.h>
#include <qscopedpointer.h>
#include <qcolor.h>
#include <qmatrix.h>
#include <qtransform.h>
#include <qimage.h>
#include <qpixmap.h>

class QPixmap;
class QGradient;
class QVariant;

class QGradientPrivate;

struct QBrushData;

namespace cs_internal {
   struct QBrushDataPointerDeleter {
      static void deleteData(QBrushData *d);
      void operator()(QBrushData *d) const;
   };
}

class Q_GUI_EXPORT QBrush
{
 public:
   QBrush();
   QBrush(Qt::BrushStyle style);
   QBrush(const QColor &color, Qt::BrushStyle style = Qt::SolidPattern);
   QBrush(Qt::GlobalColor color, Qt::BrushStyle style = Qt::SolidPattern);

   QBrush(const QColor &color, const QPixmap &pixmap);
   QBrush(Qt::GlobalColor color, const QPixmap &pixmap);
   QBrush(const QPixmap &pixmap);
   QBrush(const QImage &image);

   QBrush(const QBrush &other);

   QBrush(const QGradient &gradient);

   ~QBrush();

   QBrush &operator=(const QBrush &other);

   inline QBrush &operator=(QBrush &&other) {
      qSwap(d, other.d);
      return *this;
   }

   inline void swap(QBrush &other) {
      qSwap(d, other.d);
   }

   operator QVariant() const;

   inline Qt::BrushStyle style() const;
   void setStyle(Qt::BrushStyle style);

   inline const QMatrix &matrix() const;
   void setMatrix(const QMatrix &matrix);

   inline QTransform transform() const;
   void setTransform(const QTransform &transform);

   QPixmap texture() const;
   void setTexture(const QPixmap &pixmap);

   QImage textureImage() const;
   void setTextureImage(const QImage &image);

   inline const QColor &color() const;
   void setColor(const QColor &color);
   inline void setColor(Qt::GlobalColor color);

   const QGradient *gradient() const;

   bool isOpaque() const;

   bool operator==(const QBrush &brush) const;
   inline bool operator!=(const QBrush &brush) const {
      return !(operator==(brush));
   }

   inline bool isDetached() const;

   using DataPtr = QScopedPointer<QBrushData, cs_internal::QBrushDataPointerDeleter>;

   inline DataPtr &data_ptr() {
      return d;
   }

 private:
   friend class QRasterPaintEngine;
   friend class QRasterPaintEnginePrivate;
   friend class QPainter;
   friend struct QSpanData;

   friend bool Q_GUI_EXPORT qHasPixmapTexture(const QBrush &brush);

   void detach(Qt::BrushStyle newStyle);
   void init(const QColor &color, Qt::BrushStyle style);
   QScopedPointer<QBrushData, cs_internal::QBrushDataPointerDeleter> d;
   void cleanUp(QBrushData *data);
};

inline void QBrush::setColor(Qt::GlobalColor color)
{
   setColor(QColor(color));
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QBrush &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QBrush &);

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

   inline void setSpread(Spread spreadType);
   Spread spread() const {
      return m_spread;
   }

   void setColorAt(qreal position, const QColor &color);

   void setStops(const QVector<QPair<qreal, QColor>> &stops);
   QVector<QPair<qreal, QColor>> stops() const;

   CoordinateMode coordinateMode() const;
   void setCoordinateMode(CoordinateMode mode);

   InterpolationMode interpolationMode() const;
   void setInterpolationMode(InterpolationMode mode);

   bool operator==(const QGradient &other) const;

   bool operator!=(const QGradient &other) const {
      return ! operator==(other);
   }

 private:
   struct LinearData {
      qreal x1;
      qreal y1;
      qreal x2;
      qreal y2;
   };

   struct RadialData {
      qreal cx;
      qreal cy;
      qreal fx;
      qreal fy;
      qreal cradius;
      qreal fradius;
   };

   struct ConicalData {
      qreal cx;
      qreal cy;
      qreal angle;
   };

   union {
      LinearData  linear;
      RadialData  radial;
      ConicalData conical;

   } m_data;

   Type m_type;
   Spread m_spread;
   QVector<QPair<qreal, QColor>> m_stops;

   CoordinateMode m_CoordinateMode;
   InterpolationMode m_InterpolationMode;

   friend class QBrush;
   friend class QLinearGradient;
   friend class QRadialGradient;
   friend class QConicalGradient;
};

inline void QGradient::setSpread(Spread spreadType)
{
   m_spread = spreadType;
}

class Q_GUI_EXPORT QLinearGradient : public QGradient
{
 public:
   QLinearGradient();
   QLinearGradient(const QPointF &start, const QPointF &finalStop);
   QLinearGradient(qreal x1, qreal y1, qreal x2, qreal y2);

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

#endif // QBRUSH_H
