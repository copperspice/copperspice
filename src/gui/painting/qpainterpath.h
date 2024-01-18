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

#ifndef QPAINTERPATH_H
#define QPAINTERPATH_H

#include <qmatrix.h>
#include <qglobal.h>
#include <qrect.h>
#include <qline.h>
#include <qvector.h>
#include <qscopedpointer.h>

class QFont;
class QPen;
class QPolygonF;
class QRegion;
class QVectorPath;

class QPainterPathData;
class QPainterPathPrivate;
class QPainterPathStrokerPrivate;

namespace cs_internal {
   struct QPainterPathPrivateDeleter {
      void operator()(QPainterPathPrivate *d) const;
   };
}

class Q_GUI_EXPORT QPainterPath
{
 public:
   enum ElementType {
      MoveToElement,
      LineToElement,
      CurveToElement,
      CurveToDataElement
   };

   class Element
   {
    public:
      qreal x;
      qreal y;
      ElementType type;

      bool isMoveTo() const {
         return type == MoveToElement;
      }

      bool isLineTo() const {
         return type == LineToElement;
      }

      bool isCurveTo() const {
         return type == CurveToElement;
      }

      operator QPointF () const {
         return QPointF(x, y);
      }

      bool operator==(const Element &other) const {
         return qFuzzyCompare(x, other.x) && qFuzzyCompare(y, other.y) && type == other.type;
      }

      bool operator!=(const Element &other) const {
         return !operator==(other);
      }
   };

   QPainterPath();
   explicit QPainterPath(const QPointF &startPoint);
   QPainterPath(const QPainterPath &other);
   QPainterPath &operator=(const QPainterPath &other);

   QPainterPath &operator=(QPainterPath &&other) {
      qSwap(d_ptr, other.d_ptr);
      return *this;
   }

   ~QPainterPath();

   void swap(QPainterPath &other) {
      d_ptr.swap(other.d_ptr);
   }

   void closeSubpath();

   void moveTo(const QPointF &point);
   inline void moveTo(qreal x, qreal y);

   void lineTo(const QPointF &endPoint);
   inline void lineTo(qreal x, qreal y);

   void arcMoveTo(const QRectF &rect, qreal angle);
   inline void arcMoveTo(qreal x, qreal y, qreal width, qreal height, qreal angle);

   void arcTo(const QRectF &rect, qreal startAngle, qreal arcLength);
   inline void arcTo(qreal x, qreal y, qreal width, qreal height, qreal startAngle, qreal arcLength);

   void cubicTo(const QPointF &point1, const QPointF &point2, const QPointF &point3);
   inline void cubicTo(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3);

   void quadTo(const QPointF &point1, const QPointF &point2);
   inline void quadTo(qreal x1, qreal y1, qreal x2, qreal y2);

   QPointF currentPosition() const;

   void addRect(const QRectF &rect);
   inline void addRect(qreal x, qreal y, qreal width, qreal height);
   void addEllipse(const QRectF &rect);
   inline void addEllipse(qreal x, qreal y, qreal width, qreal height);
   inline void addEllipse(const QPointF &center, qreal rx, qreal ry);
   void addPolygon(const QPolygonF &polygon);
   void addText(const QPointF &point, const QFont &font, const QString &text);
   inline void addText(qreal x, qreal y, const QFont &font, const QString &text);
   void addPath(const QPainterPath &path);
   void addRegion(const QRegion &region);

   void addRoundedRect(const QRectF &rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize);
   inline void addRoundedRect(qreal x, qreal y, qreal width, qreal height,  qreal xRadius,
         qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize);

   void addRoundRect(const QRectF &rect, int xRound, int yRound);
   inline void addRoundRect(qreal x, qreal y, qreal width, qreal height, int xRound, int yRound);
   inline void addRoundRect(const QRectF &rect, int roundness);
   inline void addRoundRect(qreal x, qreal y, qreal width, qreal height, int roundness);

   void connectPath(const QPainterPath &path);

   bool contains(const QPointF &point) const;
   bool contains(const QRectF &rect) const;
   bool intersects(const QRectF &rect) const;

   void translate(qreal dx, qreal dy);
   inline void translate(const QPointF &offset);

   QPainterPath translated(qreal dx, qreal dy) const;
   inline QPainterPath translated(const QPointF &offset) const;

   QRectF boundingRect() const;
   QRectF controlPointRect() const;

   Qt::FillRule fillRule() const;
   void setFillRule(Qt::FillRule fillRule);

   bool isEmpty() const;

   QPainterPath toReversed() const;
   QList<QPolygonF> toSubpathPolygons(const QMatrix &matrix = QMatrix()) const;
   QList<QPolygonF> toFillPolygons(const QMatrix &matrix = QMatrix()) const;
   QPolygonF toFillPolygon(const QMatrix &matrix = QMatrix()) const;
   QList<QPolygonF> toSubpathPolygons(const QTransform &matrix) const;
   QList<QPolygonF> toFillPolygons(const QTransform &matrix) const;
   QPolygonF toFillPolygon(const QTransform &matrix) const;

   int elementCount() const;
   QPainterPath::Element elementAt(int index) const;
   void setElementPositionAt(int index, qreal x, qreal y);

   qreal length() const;
   qreal percentAtLength(qreal len) const;
   QPointF pointAtPercent(qreal t) const;
   qreal angleAtPercent(qreal t) const;
   qreal slopeAtPercent(qreal t) const;

   bool intersects(const QPainterPath &other) const;
   bool contains(const QPainterPath &other) const;
   QPainterPath united(const QPainterPath &other) const;
   QPainterPath intersected(const QPainterPath &other) const;
   QPainterPath subtracted(const QPainterPath &other) const;
   QPainterPath subtractedInverted(const QPainterPath &other) const;

   QPainterPath simplified() const;

   bool operator==(const QPainterPath &other) const;
   bool operator!=(const QPainterPath &other) const;

   QPainterPath operator&(const QPainterPath &other) const;
   QPainterPath operator|(const QPainterPath &other) const;
   QPainterPath operator+(const QPainterPath &other) const;
   QPainterPath operator-(const QPainterPath &other) const;
   QPainterPath &operator&=(const QPainterPath &other);
   QPainterPath &operator|=(const QPainterPath &other);
   QPainterPath &operator+=(const QPainterPath &other);
   QPainterPath &operator-=(const QPainterPath &other);

 private:
   QScopedPointer<QPainterPathPrivate, cs_internal::QPainterPathPrivateDeleter> d_ptr;

   void ensureData() {
      if (! d_ptr) {
         ensureData_helper();
      }
   }

   void ensureData_helper();
   void detach();
   void detach_helper();
   void setDirty(bool);
   void computeBoundingRect() const;
   void computeControlPointRect() const;

   QPainterPathData *d_func() const {
      return reinterpret_cast<QPainterPathData *>(d_ptr.data());
   }

   friend class QPainterPathData;
   friend class QPainterPathStroker;
   friend class QPainterPathStrokerPrivate;
   friend class QMatrix;
   friend class QTransform;
   friend class QVectorPath;
   friend Q_GUI_EXPORT const QVectorPath &qtVectorPathForPath(const QPainterPath &);

   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPainterPath &path);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPainterPath &path);

};

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPainterPath &path);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPainterPath &path);

class Q_GUI_EXPORT QPainterPathStroker
{
 public:
   QPainterPathStroker();
   explicit QPainterPathStroker(const QPen &pen);

   QPainterPathStroker(const QPainterPathStroker &) = delete;
   QPainterPathStroker &operator=(const QPainterPathStroker &) = delete;

   ~QPainterPathStroker();

   void setWidth(qreal width);
   qreal width() const;

   void setCapStyle(Qt::PenCapStyle style);
   Qt::PenCapStyle capStyle() const;

   void setJoinStyle(Qt::PenJoinStyle style);
   Qt::PenJoinStyle joinStyle() const;

   void setMiterLimit(qreal limit);
   qreal miterLimit() const;

   void setCurveThreshold(qreal threshold);
   qreal curveThreshold() const;

   void setDashPattern(Qt::PenStyle style);
   void setDashPattern(const QVector<qreal> &dashPattern);
   QVector<qreal> dashPattern() const;

   void setDashOffset(qreal offset);
   qreal dashOffset() const;

   QPainterPath createStroke(const QPainterPath &path) const;

 private:
   Q_DECLARE_PRIVATE(QPainterPathStroker)

   QScopedPointer<QPainterPathStrokerPrivate> d_ptr;

   friend class QX11PaintEngine;
};

inline void QPainterPath::moveTo(qreal x, qreal y)
{
   moveTo(QPointF(x, y));
}

inline void QPainterPath::lineTo(qreal x, qreal y)
{
   lineTo(QPointF(x, y));
}

inline void QPainterPath::arcTo(qreal x, qreal y, qreal width, qreal height, qreal startAngle, qreal arcLength)
{
   arcTo(QRectF(x, y, width, height), startAngle, arcLength);
}

inline void QPainterPath::arcMoveTo(qreal x, qreal y, qreal width, qreal height, qreal angle)
{
   arcMoveTo(QRectF(x, y, width, height), angle);
}

inline void QPainterPath::cubicTo(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3)
{
   cubicTo(QPointF(x1, y1), QPointF(x2, y2), QPointF(x3, y3));
}

inline void QPainterPath::quadTo(qreal x1, qreal y1, qreal x2, qreal y2)
{
   quadTo(QPointF(x1, y1), QPointF(x2, y2));
}

inline void QPainterPath::addEllipse(qreal x, qreal y, qreal width, qreal height)
{
   addEllipse(QRectF(x, y, width, height));
}

inline void QPainterPath::addEllipse(const QPointF &center, qreal rx, qreal ry)
{
   addEllipse(QRectF(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
}

inline void QPainterPath::addRect(qreal x, qreal y, qreal width, qreal height)
{
   addRect(QRectF(x, y, width, height));
}

inline void QPainterPath::addRoundedRect(qreal x, qreal y, qreal width, qreal height,
   qreal xRadius, qreal yRadius, Qt::SizeMode mode)
{
   addRoundedRect(QRectF(x, y, width, height), xRadius, yRadius, mode);
}

inline void QPainterPath::addRoundRect(qreal x, qreal y, qreal width, qreal height, int xRound, int yRound)
{
   addRoundRect(QRectF(x, y, width, height), xRound, yRound);
}

inline void QPainterPath::addRoundRect(const QRectF &rect, int roundness)
{
   int xRound = roundness;
   int yRound = roundness;

   if (rect.width() > rect.height()) {
      xRound = int(roundness * rect.height() / rect.width());
   } else {
      yRound = int(roundness * rect.width() / rect.height());
   }

   addRoundRect(rect, xRound, yRound);
}

inline void QPainterPath::addRoundRect(qreal x, qreal y, qreal width, qreal height, int roundness)
{
   addRoundRect(QRectF(x, y, width, height), roundness);
}

inline void QPainterPath::addText(qreal x, qreal y, const QFont &font, const QString &text)
{
   addText(QPointF(x, y), font, text);
}

inline void QPainterPath::translate(const QPointF &offset)
{
   translate(offset.x(), offset.y());
}

inline QPainterPath QPainterPath::translated(const QPointF &offset) const
{
   return translated(offset.x(), offset.y());
}

Q_GUI_EXPORT QDebug operator<<(QDebug, const QPainterPath &);

#endif
